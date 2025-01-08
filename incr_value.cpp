#include <cstdlib>
#include <iostream>
#include "csapp.h"
#include "message.h"
#include "message_serialization.h"

int main(int argc, char **argv) {
  if ( argc != 6 && (argc != 7 || std::string(argv[1]) != "-t") ) {
    std::cerr << "Usage: ./incr_value [-t] <hostname> <port> <username> <table> <key>\n";
    std::cerr << "Options:\n";
    std::cerr << "  -t      execute the increment as a transaction\n";
    return 1;
  }

  int count = 1;

  bool use_transaction = false;
  if ( argc == 7 ) {
    use_transaction = true;
    count = 2;
  }

  std::string hostname = argv[count++];
  std::string port = argv[count++];
  std::string username = argv[count++];
  std::string table = argv[count++];
  std::string key = argv[count++];

  int fd = open_clientfd(hostname.c_str(), port.c_str());
  if (fd < 0){
    std::cerr << "Error: Couldn't connect to server" << std::endl;
    exit(1);
  }

  Message login(MessageType::LOGIN, {username});
  Message begin(MessageType::BEGIN);
  Message get(MessageType::GET, {table, key});
  Message push(MessageType::PUSH, {"1"});
  Message add(MessageType::ADD);
  Message set(MessageType::SET, {table, key});
  Message commit(MessageType::COMMIT);
  Message bye(MessageType::BYE);

  std::string encoded_login, encoded_begin, encoded_get, encoded_push, encoded_add, encoded_set, encoded_commit, encoded_bye; 
  MessageSerialization::encode(login, encoded_login);
  MessageSerialization::encode(begin, encoded_begin);
  MessageSerialization::encode(get, encoded_get);
  MessageSerialization::encode(push, encoded_push);
  MessageSerialization::encode(add, encoded_add);
  MessageSerialization::encode(set, encoded_set);
  MessageSerialization::encode(commit, encoded_commit);
  MessageSerialization::encode(bye, encoded_bye);

  rio_t rio;
  char buf[1024];
  Message response;

  rio_writen(fd, encoded_login.c_str(), encoded_login.length());
  rio_readinitb(&rio, fd);
  ssize_t n = rio_readlineb(&rio, buf, sizeof(buf));
  if(n <= 0){
    std::cerr << "Error: No response from server. " << std::endl;
    exit(1);
  }
  MessageSerialization::decode(buf, response);
  if(response.get_message_type() != MessageType::OK){
    std::cerr << "Error: " << response.get_quoted_text() << std::endl;
    exit(1);
  }

  if(use_transaction) {
    rio_writen(fd, encoded_begin.c_str(), encoded_begin.length());
    rio_readinitb(&rio, fd);
    n = rio_readlineb(&rio, buf, sizeof(buf));
    if(n <= 0){
      std::cerr << "Error: No response from server. " << std::endl;
      exit(1);
    }
    MessageSerialization::decode(buf, response);
    if(response.get_message_type() != MessageType::OK){
      std::cerr << "Error: " << response.get_quoted_text() << std::endl;
      exit(1);
    }
  }

  rio_writen(fd, encoded_get.c_str(), encoded_get.length());
  rio_readinitb(&rio, fd);
  n = rio_readlineb(&rio, buf, sizeof(buf));
  if(n <= 0){
    std::cerr << "Error: No response from server. " << std::endl;
    exit(1);
  }
  MessageSerialization::decode(buf, response);
  if(response.get_message_type() != MessageType::OK){
    std::cerr << "Error: " << response.get_quoted_text() << std::endl;
    exit(1);
  }

  rio_writen(fd, encoded_push.c_str(), encoded_push.length());
  rio_readinitb(&rio, fd);
  n = rio_readlineb(&rio, buf, sizeof(buf));
  if(n <= 0){
    std::cerr << "Error: No response from server. " << std::endl;
    exit(1);
  }
  MessageSerialization::decode(buf, response);
  if(response.get_message_type() != MessageType::OK){
    std::cerr << "Error: " << response.get_quoted_text() << std::endl;
    exit(1);
  }

  rio_writen(fd, encoded_add.c_str(), encoded_add.length());
  rio_readinitb(&rio, fd);
  n = rio_readlineb(&rio, buf, sizeof(buf));
  if(n <= 0){
    std::cerr << "Error: No response from server. " << std::endl;
    exit(1);
  }
  MessageSerialization::decode(buf, response);
  if(response.get_message_type() != MessageType::OK){
    std::cerr << "Error: " << response.get_quoted_text() << std::endl;
    exit(1);
  }

  rio_writen(fd, encoded_set.c_str(), encoded_set.length());
  rio_readinitb(&rio, fd);
  n = rio_readlineb(&rio, buf, sizeof(buf));
  if(n <= 0){
    std::cerr << "Error: No response from server. " << std::endl;
    exit(1);
  }
  MessageSerialization::decode(buf, response);
  if(response.get_message_type() != MessageType::OK){
    std::cerr << "Error: " << response.get_quoted_text() << std::endl;
    exit(1);
  }

  if(use_transaction){
    rio_writen(fd, encoded_commit.c_str(), encoded_commit.length());
    rio_readinitb(&rio, fd);
    n = rio_readlineb(&rio, buf, sizeof(buf));
    if(n <= 0){
      std::cerr << "Error: No response from server. " << std::endl;
      exit(1);
    }
    MessageSerialization::decode(buf, response);
    if(response.get_message_type() != MessageType::OK){
      std::cerr << "Error: " << response.get_quoted_text() << std::endl;
      exit(1);
    }
  }

  rio_writen(fd, encoded_bye.c_str(), encoded_bye.length());
  rio_readinitb(&rio, fd);
  n = rio_readlineb(&rio, buf, sizeof(buf));
  if(n <= 0){
    std::cerr << "Error: No response from server. " << std::endl;
    exit(1);
  }
  MessageSerialization::decode(buf, response);
  if(response.get_message_type() != MessageType::OK){
    std::cerr << "Error: " << response.get_quoted_text() << std::endl;
    exit(1);
  }
  
  close(fd);

}
