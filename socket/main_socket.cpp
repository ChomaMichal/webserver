#include "../utils/NetworkIO.hpp"
#include "Socket.hpp"
#include <chrono>
#include <cstring>
#include <iostream>
#include <list>
#include <pthread.h>
#include <stdexcept>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <vector>

void server(void) {
  std::cout << "[SERVER] Starting server...\n";
  auto maybe = Socket::bind();
  if (!maybe.check()) {
    std::cerr << "[SERVER] Failed to bind socket\n";
    return;
  }

  Socket soc = maybe.safe_unwrap();
  Requests clients;
  std::cout << "[SERVER] Waiting for connections...\n";
  while (true) {
    while (true) {
      // std::cerr << "Looping\n";
      auto err = soc.accept();
      if (!err.check()) {
        std::cerr << "[SERVER] Error accepting connection\n";
        continue;
      }
      auto maybe_fd = err.safe_unwrap();
      if (maybe_fd.check()) {
        int client_fd = maybe_fd.safe_unwrap();
        std::cout << "[SERVER] Client accepted with fd: " << client_fd << "\n";
        clients.add(client_fd);
      } else {
        break;
      }
    }

    // Read from client using NetworkIO
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
}

void client(void) {
  // Give server time to start
  std::this_thread::sleep_for(std::chrono::seconds(1));

  std::cout << "[CLIENT] Starting client...\n";
  auto maybe = Socket::connect();
  if (!maybe.check()) {
    std::cerr << "[CLIENT] Failed to connect socket\n";
    return;
  }
  Socket soc = maybe.safe_unwrap();
  std::cout << "[CLIENT] Connected to server\n";

  // Send data using NetworkIO
  NetworkIO io(soc.getFd());
  io << "Hello from client!" << sendData;
  std::cout << "[CLIENT] Sent message to server\n";

  // Give server time to read
  std::this_thread::sleep_for(std::chrono::seconds(1));
  std::cout << "[CLIENT] Closing connection\n";
}

int main() {
  try {
    for (int i = 0; i < 10; i++) {
      int pid = fork();
      if (pid == 0) {
        client();
        exit(0);
      }
    }
    server();

  } catch (std::invalid_argument &e) {
    std::cerr << "Exception thrown\n" << std::endl;
    std::cerr << e.what() << std::endl;
  }
  return 0;
}
