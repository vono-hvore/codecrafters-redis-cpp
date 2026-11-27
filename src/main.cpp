#include "EventLoop.hpp"
#include <arpa/inet.h>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <optional>
#include <string>
#include <string_view>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

void die(const char *msg);
void log_msg(std::string_view msg);
int make_socket_fd();
void set_sockopt(int socket_fd);
sockaddr_in make_socketaddr();
void bind_socket(int socket_fd, sockaddr_in server_addr);
void listen_socket(int server_fd);
int accept_client(int socket_fd);
std::optional<std::string> recv_msg(int socket_fd);
bool send_msg(int socket_fd, const std::string &msg);
const char *PONG = "+PONG\r\n";

void handle_client(int client_fd) {
  log_msg("Client connected");
  while (auto msg = recv_msg(client_fd)) {
    log_msg(*msg);
    if (!send_msg(client_fd, PONG))
      break;
  }
}

int main() {
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  EventLoop event_loop;
  int server_fd = make_socket_fd();
  set_sockopt(server_fd);
  bind_socket(server_fd, make_socketaddr());
  listen_socket(server_fd);

  log_msg("Waiting for a client to connect...");
  log_msg("Logs from your program will appear here!");

  while (auto client_fd = accept_client(server_fd)) {
    ActionHandler *handler = new ActionHandler([&client_fd] {
      handle_client(client_fd);
      return Event::Finish;
    });
    event_loop.post(client_fd, handler);
    close(client_fd);
  }

  close(server_fd);
  return 0;
}

void die(const char *msg) {
  std::cerr << msg << '\n';
  std::exit(1);
}

void log_msg(std::string_view msg) { std::cout << msg << '\n'; }

int make_socket_fd() {
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0)
    die("Failed to create server socket");
  return server_fd;
}

// Since the tester restarts your program quite often, setting SO_REUSEADDR
// ensures that we don't run into 'Address already in use' errors
void set_sockopt(int socket_fd) {
  int reuse = 1;
  int result
      = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
  if (result < 0)
    die("setsockopt failed");
}

sockaddr_in make_socketaddr() {
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(6379);

  return server_addr;
}

void bind_socket(int socket_fd, sockaddr_in server_addr) {
  int result
      = bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (result != 0)
    die("Failed to bind to port 6379");
}

void listen_socket(int server_fd) {
  int connection_backlog = 5;
  int result = listen(server_fd, connection_backlog);
  if (result != 0)
    die("listen failed");
}

int accept_client(int socket_fd) {
  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);
  const int client_fd = accept(socket_fd, (struct sockaddr *)&client_addr,
                               (socklen_t *)&client_addr_len);
  if (client_fd < 0)
    die("failed accept client");

  return client_fd;
}

std::optional<std::string> recv_msg(int socket_fd) {
  std::string buf(1024, '\0');
  const ssize_t size = recv(socket_fd, buf.data(), buf.size(), 0);
  const bool result = size > 0;
  if (result) {
    buf.resize(size);
    return buf;
  } else {
    std::cerr << "recv failed or client disconnected\n";
    return std::nullopt;
  }
}

bool send_msg(int socket_fd, const std::string &msg) {
  const ssize_t size = send(socket_fd, msg.data(), msg.size(), 0);
  const bool result = size > 0;
  if (!result)
    std::cerr << "failed to send to client message\n";
  return result;
}
