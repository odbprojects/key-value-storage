#include <cstdio>
#include <set>
#include <map>
#include <regex>
#include <cctype>
#include <cassert>
#include "message.h"

Message::Message()
  : m_message_type(MessageType::NONE)
{
}

Message::Message( MessageType message_type, std::initializer_list<std::string> args )
  : m_message_type( message_type )
  , m_args( args )
{
}

Message::Message( MessageType message_type, std::vector<std::string> args )
  : m_message_type( message_type )
  , m_args( args )
{
}

Message::Message( const Message &other )
  : m_message_type( other.m_message_type )
  , m_args( other.m_args )
{
}

Message::~Message()
{
}

Message &Message::operator=( const Message &rhs )
{
  if (this != &rhs) {
    m_message_type = rhs.m_message_type;
    m_args = rhs.m_args;
  }
  return *this;
}

MessageType Message::get_message_type() const
{
  return m_message_type;
}

void Message::set_message_type(MessageType message_type)
{
  m_message_type = message_type;
}

std::string Message::get_username() const
{
  if (m_args.size() > 0) { 
    return m_args[0].c_str();
  }
  return "";
}

std::string Message::get_table() const
{
    return m_args[0].c_str();
}

std::string Message::get_key() const
{
  std::string result = (m_args.size() > 1) ? m_args[1] : m_args[0];
  return result; 
}

std::string Message::get_value() const
{
  return m_args[0].c_str();
}

std::string Message::get_quoted_text() const
{
  std::string result;
  std::regex quote_regex("\"([^\"]*)\"");
  for (const auto &arg : m_args) {
    std::smatch match;
    if (std::regex_search(arg, match, quote_regex)) {
      result += match[1].str();
    }
  }
  return result;
}

void Message::push_arg( const std::string &arg )
{
  m_args.push_back( arg );
}

bool Message::is_valid() const
{
  static const std::set<MessageType> valid_types = {
    MessageType::LOGIN, MessageType::CREATE, MessageType::PUSH,
    MessageType::POP, MessageType::TOP, MessageType::SET,
    MessageType::GET, MessageType::ADD, MessageType::SUB,
    MessageType::MUL, MessageType::DIV, MessageType::BEGIN,
    MessageType::COMMIT, MessageType::BYE, MessageType::OK,
    MessageType::FAILED, MessageType::ERROR, MessageType::DATA
  };

  bool is_valid = false; 

  for(MessageType m : valid_types) {
    if (m == m_message_type) {
      is_valid = true;
    }
  }

  if (m_message_type == MessageType::LOGIN || m_message_type == MessageType::CREATE) {
    if (get_num_args() != 1) {
      
      return false;
    }
    return is_identifier();
  }

  if (m_message_type == MessageType::SET || m_message_type == MessageType::GET) {
    if (get_num_args() != 2) {
      std::printf("Debug message returning: %d\n", 2);
      return false;
    }
    return is_identifier();
  }

  if (m_message_type == MessageType::PUSH || m_message_type == MessageType::FAILED || m_message_type == MessageType::ERROR || m_message_type == MessageType::DATA) {
    if (get_num_args() != 1) {
    
      return false;
    }
  }
  return is_valid; 
}

bool Message::is_identifier() const {
  for (const auto& identifier : m_args) {
    if (identifier.empty() || !std::isalpha(identifier[0])) {
      return false;
    }
    for (size_t i = 1; i < identifier.size(); ++i) {
      if (!std::isalnum(identifier[i]) && identifier[i] != '_') {
        return false;
      }
    }
  }
  return true;
}

void Message::clear_args() {
  m_args.clear();
}


