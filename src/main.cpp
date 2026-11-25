#include <arpa/inet.h>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv) {
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    std::cerr << "Failed to create server socket\n";
    return 1;
  }

  // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse))
      < 0) {
    std::cerr << "setsockopt failed\n";
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(6379);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))
      != 0) {
    std::cerr << "Failed to bind to port 6379\n";
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0) {
    std::cerr << "listen failed\n";
    return 1;
  }

  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);
  std::cout << "Waiting for a client to connect...\n";

  // You can use print statements as follows for debugging, they'll be visible
  // when running tests.
  std::cout << "Logs from your program will appear here!\n";

  const int client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                               (socklen_t *)&client_addr_len);
  if (client_fd < 0) {
    std::cerr << "failed accept client\n";
    return 1;
  }

  std::cout << "Client connected\n";
  while (true) {
    std::string client_msg(1024, '\0');
    const size_t recv_size
        = recv(client_fd, client_msg.data(), client_msg.size(), 0);
    if (recv_size <= 0) {
      std::cerr << "recv failed or client disconnected\n";
      break;
    }

    std::cout << std::string(client_msg, recv_size) << "\n";
    std::string msg = "+PONG\r\n";
    const ssize_t msg_size = send(client_fd, msg.data(), msg.size(), 0);
    if (msg_size <= 0) {
      std::cerr << "failed to send to client message\n";
      break;
    }
    std::cout << client_msg << '\n';
  }

  close(client_fd);
  close(server_fd);

  return 0;
}
