#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "csapp.h"
#include "message.h"
#include "message_serialization.h"

int main(int argc, char **argv)
{
  if ( argc != 6 ) {
    std::cerr << "Usage: ./get_value <hostname> <port> <username> <table> <key>\n";
    return 1;
  }

  std::string hostname = argv[1];
  std::string port = argv[2];
  std::string username = argv[3];
  std::string table = argv[4];
  std::string key = argv[5];

  int fd = open_clientfd(hostname.c_str(), port.c_str());
  if (fd < 0){
    std::cerr << "Error: Couldn't connect to server" << std::endl;
    exit(1);
  }

  Message login(MessageType::LOGIN, {username});
  Message get(MessageType::GET, {table, key});
  Message top(MessageType::TOP);
  Message bye(MessageType::BYE);
  std::string encoded_login, encoded_get, encoded_top, encoded_bye;
  MessageSerialization::encode(login, encoded_login);
  MessageSerialization::encode(get, encoded_get);
  MessageSerialization::encode(top, encoded_top);
  MessageSerialization::encode(bye, encoded_bye);

  
  rio_t rio;
  char buf[1024];
  Message response;

  rio_writen(fd, encoded_login.c_str(), encoded_login.length());
  rio_readinitb(&rio, fd);
  ssize_t n = rio_readlineb(&rio, buf, sizeof(buf));
  if (n <= 0){
    std::cerr << "Error: No response from server. " << std::endl;
    exit(1);
  }
  MessageSerialization::decode(buf, response);
  if(response.get_message_type() != MessageType::OK){
    std::cerr << "Error: " << response.get_quoted_text() << std::endl;
    exit(1);
  }

  rio_writen(fd, encoded_get.c_str(), encoded_get.length());
  rio_readinitb(&rio, fd);
  n = rio_readlineb(&rio, buf, sizeof(buf));
  if (n <= 0){
    std::cerr << "Error: No response from server. " << std::endl;
    exit(1);
  }
  MessageSerialization::decode(buf, response);
  if(response.get_message_type() != MessageType::OK){
    std::cerr << "Error: " << response.get_quoted_text() << std::endl;
    exit(1);
  } 

  rio_writen(fd, encoded_top.c_str(), encoded_top.length());
  n = rio_readlineb(&rio, buf, sizeof(buf));
  if (n <= 0){
    std::cerr << "Error: No response from server. " << std::endl;
    exit(1);
  }
  MessageSerialization::decode(buf, response);
  if(response.get_message_type() != MessageType::DATA){
    std::cerr << "Error: " << response.get_quoted_text() << std::endl;
    exit(1);
  }
  std::cout << response.get_value() << std::endl;

  rio_writen(fd, encoded_bye.c_str(), encoded_bye.length());
  n = rio_readlineb(&rio, buf, sizeof(buf));
  if (n <= 0){
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
