#include <exception>
#include <iostream>
#include <cassert>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <vector>
#include "csapp.h"
#include "message.h"
#include "message_serialization.h"
#include "server.h"
#include "exceptions.h"
#include "client_connection.h"
#include "table.h"

ClientConnection::ClientConnection( Server *server, int client_fd )
  : m_server( server )
  , m_client_fd( client_fd )
  , autocommit_mode(true)
  , logged_in(false)
  , loop(true)
{
  rio_readinitb( &m_fdbuf, m_client_fd );
}

ClientConnection::~ClientConnection()
{
  close(m_client_fd);
}

void ClientConnection::chat_with_client()
{
  while (loop) {
    char buf[1024];
    Message client_message;
    ssize_t n = rio_readlineb(&m_fdbuf, buf, 1024);
    if (n <= 0) {
      break; // Handle no more info from client
    }

    try{
      MessageSerialization::decode(buf, client_message);
    } catch (InvalidMessage& e) {
      respond_error("Invalid message type");
      break; 
    }
    try {
      switch(client_message.get_message_type()){
        case MessageType::NONE:
          respond_error("Invalid message type");
          break;
        case MessageType::LOGIN: {
          logged_in = true;
          respond_ok();
          break;
        }
        case MessageType::CREATE: {
          handle_logged_in();
          m_server->create_table(client_message.get_table());
          respond_ok();
          break;
        }
        case MessageType::PUSH: {
          handle_logged_in();
          operand_stack.push(client_message.get_value());
          respond_ok();
          break;
        }
        case MessageType::POP: {
          handle_logged_in();
          if (operand_stack.empty()){
            throw OperationException("Operand Stack was empty. ");
          }
          operand_stack.pop();
          respond_ok();
          break;
        }
        case MessageType::TOP: {
          handle_logged_in();
          if (operand_stack.empty()){
            throw OperationException("Operand Stack was empty. ");
          }
          std::string top_value = operand_stack.top();
          Message top(MessageType::DATA, {top_value});
          std::string response;
          MessageSerialization::encode(top, response);
          rio_writen(m_client_fd, response.c_str(), response.length());
          break;
        }
        case MessageType::SET: {
          handle_logged_in();
          Table *table = m_server->find_table(client_message.get_table());
          if (table == nullptr) {
            throw OperationException("Table does not exist. ");
          }
          if (autocommit_mode) {
            table->lock();
            std::string value = operand_stack.top();
            operand_stack.pop();
            table->set(client_message.get_key(), value);
            table->commit_changes();
            table->unlock();
          } else {
            if (table->trylock()) {
              std::string value = operand_stack.top();
              operand_stack.pop();
              locked_tables.push_back(table);
              table->set(client_message.get_key(), value);
            } else { // table alr locked
              throw FailedTransaction("Couldn't aquire lock for requested table");
            }
          }
          respond_ok();
          break;
        }
        case MessageType::GET: {
          handle_logged_in();
          Table *table = m_server->find_table(client_message.get_table());
          if (table == nullptr) {
            throw OperationException("Table does not exist. ");
          }
          operand_stack.push(table->get(client_message.get_key()));
          respond_ok();
          break;
        }
        case MessageType::ADD: {
          handle_logged_in();
          if (operand_stack.size() < 2) {
            throw OperationException("Less than 2 values on Operand Stack. ");
          }
          std::string val1 = operand_stack.top();
          int num1 = string_to_int(val1);
          operand_stack.pop();
          std::string val2 = operand_stack.top();
          int num2 = string_to_int(val2);
          operand_stack.pop();
          operand_stack.push(std::to_string(num2 + num1));
          respond_ok();
          break;
        }
        case MessageType::SUB: {
          handle_logged_in();
          if (operand_stack.size() < 2) {
            throw OperationException("Less than 2 values on Operand Stack. ");
          }
          std::string val1 = operand_stack.top();
          int num1 = string_to_int(val1);
          operand_stack.pop();
          std::string val2 = operand_stack.top();
          int num2 = string_to_int(val2);
          operand_stack.pop();
          operand_stack.push(std::to_string(num2 - num1));
          respond_ok();
          break;
        }
        case MessageType::MUL: {
          handle_logged_in();
          if (operand_stack.size() < 2) {
            throw OperationException("Less than 2 values on Operand Stack. ");
          }
          std::string val1 = operand_stack.top();
          int num1 = string_to_int(val1);
          operand_stack.pop();
          std::string val2 = operand_stack.top();
          int num2 = string_to_int(val2);
          operand_stack.pop();
          operand_stack.push(std::to_string(num2 * num1));
          respond_ok();
          break;
        }
        case MessageType::DIV: {
          handle_logged_in();
          if (operand_stack.size() < 2) {
            throw OperationException("Less than 2 values on Operand Stack. ");
          }
          std::string val1 = operand_stack.top();
          int num1 = string_to_int(val1);
          operand_stack.pop();
          std::string val2 = operand_stack.top();
          int num2 = string_to_int(val2);
          operand_stack.pop();
          operand_stack.push(std::to_string(num2 / num1));
          respond_ok();
          break;
        }
        case MessageType::BEGIN: {
          handle_logged_in();
          if (!autocommit_mode) {
            for (std::vector<Table*>::const_iterator it = locked_tables.cbegin(); it != locked_tables.cend(); it++) {
              (*it)->rollback_changes();
            }
            locked_tables.clear();
            autocommit_mode = true;
            respond_failed("Cannot nest transactions. ");
            break;
          }
          autocommit_mode = false;
          respond_ok();
          break;
        }
        case MessageType::COMMIT: {
          handle_logged_in();
          if (autocommit_mode) {
            respond_failed("Cannot commit in autocommit mode. ");
            break;
          }
          for (std::vector<Table*>::const_iterator it = locked_tables.cbegin(); it != locked_tables.cend(); it++) {
            (*it)->commit_changes();
          }
          locked_tables.clear();
          autocommit_mode = true; 
          respond_ok();
          break;
        }
        case MessageType::BYE: {
          handle_logged_in();
          respond_ok();
          loop = false;
          close(m_client_fd);
        }
        default: {
          throw InvalidMessage("Invalid Message Type");
          break;
        }
      }
    } catch (OperationException& e) {
      respond_failed(e.what());
    } catch (FailedTransaction& e) {
      respond_failed(e.what());
    } catch (std::exception& e) {
      respond_error(e.what());
      loop = false;
    }
  }
}

// Other Member Functions

void ClientConnection::respond_ok() {
  Message ok(MessageType::OK);
  std::string response;
  MessageSerialization::encode(ok, response);
  rio_writen(m_client_fd, response.c_str(), response.length());
}

void ClientConnection::respond_error(const std::string &error_msg)
{
  Message error(MessageType::ERROR, {error_msg});
  std::string response;
  MessageSerialization::encode(error, response);
  rio_writen(m_client_fd, response.c_str(), response.length());
  close(m_client_fd);
  loop = false;
}

void ClientConnection::respond_failed(const std::string &error_msg)
{
  Message failed(MessageType::FAILED, {error_msg});
  std::string response;
  MessageSerialization::encode(failed, response);
  rio_writen(m_client_fd, response.c_str(), response.length());
}

void ClientConnection::handle_logged_in() {
  if (!logged_in) {
    throw OperationException("Must be logged in. ");
  }
}

int ClientConnection::string_to_int(std::string &str) {
  try {
    size_t pos;
    int result = std::stoi(str, &pos);

    if (pos != str.length()) {
      throw OperationException("Operand is not an integer. ");
    } else {
      return result;
    }
  } catch (std::invalid_argument &e) {
    throw OperationException("Operand is not an integer. ");
  }
}