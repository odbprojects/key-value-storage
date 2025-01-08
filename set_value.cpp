#include <iostream>
#include "csapp.h"
#include "message.h"
#include "message_serialization.h"

int main(int argc, char **argv)
{
  if (argc != 7) {
    std::cerr << "Usage: ./set_value <hostname> <port> <username> <table> <key> <value>\n";
    return 1;
  }

  std::string hostname = argv[1];
  std::string port = argv[2];
  std::string username = argv[3];
  std::string table = argv[4];
  std::string key = argv[5];
  std::string value = argv[6];

  int fd = open_clientfd(hostname.c_str(), port.c_str());
  if (fd < 0){
    std::cerr << "Couldn't connect to server" << std::endl;
    exit(1);
  }

  Message login(MessageType::LOGIN, {username});
  Message push(MessageType::PUSH, {value});
  Message set(MessageType::SET, {table, key});
  Message bye(MessageType::BYE);
  std::string encoded_login, encoded_push, encoded_set, encoded_bye;
  MessageSerialization::encode(login, encoded_login);
  MessageSerialization::encode(push, encoded_push);
  MessageSerialization::encode(set, encoded_set);
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
