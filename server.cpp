#include <iostream>
#include <cassert>
#include <memory>
#include "csapp.h"
#include "exceptions.h"
#include "guard.h"
#include "server.h"
#include <cstring>

Server::Server()
  : server_fd(-1)
{
  pthread_mutex_init(&tables_mutex, nullptr);
}

Server::~Server()
{
  if (server_fd != -1) {
    close(server_fd);
  }
  pthread_mutex_destroy(&tables_mutex);
  for (auto &pair : tables) {
    delete pair.second;
  }
}

void Server::listen( const std::string &port )
{
  server_fd = Open_listenfd(port.c_str());
  if (server_fd < 0) {
    throw std::runtime_error("Could not open listen socket");
  }
}

void Server::server_loop()
{
  struct sockaddr_storage client_addr;
  socklen_t client_len = sizeof(client_addr);
  while (true) {
    int client_fd = accept(server_fd, (SA *)&client_addr, &client_len);
    if (client_fd < 0) {
      log_error("Could not accept connection");
      continue;
    }

    ClientConnection *client = new ClientConnection(this, client_fd);
    pthread_t thr_id;
    if (pthread_create(&thr_id, nullptr, client_worker, client) != 0) {
      log_error("Could not create client thread");
      delete client;
      close(client_fd);
    }
  }
}


void *Server::client_worker( void *arg )
{
  pthread_detach(pthread_self());

  std::unique_ptr<ClientConnection> client(static_cast<ClientConnection *>(arg));
  client->chat_with_client();
  return nullptr;
}

void Server::log_error( const std::string &what )
{
  std::cerr << "Error: " << what << "\n";
}

void Server::create_table(const std::string &name)
{
  pthread_mutex_lock(&tables_mutex);
  if (tables.find(name) == tables.end()) {
    tables[name] = new Table(name);
  }
  pthread_mutex_unlock(&tables_mutex);
}

Table *Server::find_table(const std::string &name)
{
  pthread_mutex_lock(&tables_mutex);
  Table *table = nullptr;
  auto it = tables.find(name);
  if (it != tables.end()) {
      table = it->second;
  }
  pthread_mutex_unlock(&tables_mutex);
  return table;
}
