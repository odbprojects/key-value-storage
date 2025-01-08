#ifndef CLIENT_CONNECTION_H
#define CLIENT_CONNECTION_H

#include <cstdint>
#include <set>
#include "message.h"
#include "csapp.h"
#include <stack>

class Server; // forward declaration
class Table; // forward declaration

class ClientConnection {
private:
  Server *m_server;
  int m_client_fd;
  rio_t m_fdbuf;
  std::stack<std::string> operand_stack;
  bool autocommit_mode;
  std::vector<Table*> locked_tables;
  bool logged_in;
  bool loop;

  // copy constructor and assignment operator are prohibited
  ClientConnection( const ClientConnection & );
  ClientConnection &operator=( const ClientConnection & );

public:
  ClientConnection( Server *server, int client_fd );
  ~ClientConnection();

  void chat_with_client();

  // TODO: additional member functions
  void respond_ok();
  void respond_error(const std::string &error_msg); 
  void respond_failed(const std::string &error_msg); 
  void handle_logged_in();
  int string_to_int(std::string &str);
};

#endif // CLIENT_CONNECTION_H
