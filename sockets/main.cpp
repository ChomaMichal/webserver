#include "Listener.hpp"
#include "Networking.hpp"
#include "Stream.hpp"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void sendOnPort(void) {
  {
    const char *HOST = "127.0.0.1";
    const int PORT = 8080;
    const char *MESSAGE = "Hello, hard-coded world!\n";

    size_t msglen = std::strlen(MESSAGE);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
      std::cerr << "socket() failed: " << strerror(errno) << "\n";
      exit(1);
    }

    struct sockaddr_in srv;
    std::memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_port = htons(static_cast<uint16_t>(PORT));
    if (inet_pton(AF_INET, HOST, &srv.sin_addr) != 1) {
      std::cerr << "inet_pton() failed for " << HOST << "\n";
      close(sock);
      exit(1);
    }

    if (connect(sock, reinterpret_cast<struct sockaddr *>(&srv), sizeof(srv)) ==
        -1) {
      std::cerr << "connect() failed: " << strerror(errno) << "\n";
      close(sock);
      exit(1);
    }

    ssize_t total_sent = 0;
    while (static_cast<size_t>(total_sent) < msglen) {
      ssize_t n = send(sock, MESSAGE + total_sent, msglen - total_sent, 0);
      if (n == -1) {
        std::cerr << "send() failed: " << strerror(errno) << "\n";
        close(sock);
        exit(1);
      }
      total_sent += n;
    }

    std::cout << "Sent " << total_sent << " bytes to " << HOST << ":" << PORT
              << "\n";

    close(sock);
    exit(0);
  }
}

#include "Listener.hpp"
#include "Networking.hpp"
#include "Stream.hpp"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  auto lis = Listener::connect(9090);
  if (lis.is_error()) {
    std::cerr << "Listener error: " << lis.get_error() << std::endl;
    return 1;
  }
  
  int lis_fd = (*lis).getFd();
  std::cout << "Listener on port 9090, fd=" << lis_fd << std::endl;
  
  // Fork: parent is server, child is client
  pid_t pid = fork();
  
  if (pid == 0) {
    // CHILD: Client
    sleep(1); // Give server time to start listening
    
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
      std::cerr << "Client: socket failed" << std::endl;
      exit(1);
    }
    
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9090);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    if (connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
      std::cerr << "Client: connect failed: " << strerror(errno) << std::endl;
      exit(1);
    }
    
    std::cout << "Client: connected (fd=" << client_fd << "), sending data" << std::endl;
    const char* msg = "Hello from client!";
    send(client_fd, msg, strlen(msg), 0);
    
    sleep(2);
    close(client_fd);
    exit(0);
  } else if (pid > 0) {
    // PARENT: Server
    std::cout << "Server: waiting for connection..." << std::endl;
    
    // Keep checking for incoming connection
    for (int i = 0; i < 5; i++) {
      sleep(1);
      
      // Call poll to check for incoming connection
      Networking::update_fd_status();
      short revents = (*lis).getFdStatus();
      std::cout << "Poll attempt " << (i+1) << ": revents=" << revents << std::endl;
      
      if (revents & POLLIN) {
        std::cout << "Server: connection available!" << std::endl;
        
        // Accept the connection
        auto stream_opt = (*lis).accept();
        if (stream_opt.is_error()) {
          std::cerr << "Server: accept error: " << stream_opt.get_error() << std::endl;
        } else {
          Option<Stream> opt = *stream_opt;
          if (!opt.is_none()) {
            Stream stream = *opt;
            int stream_fd = stream.getFd();
            std::cout << "Server: accepted connection, stream fd=" << stream_fd << std::endl;
            
            // Poll again to get revents for the stream socket
            Networking::update_fd_status();
            short stream_revents = (*lis).getPollarr()[stream_fd].revents;
            std::cout << "Server: stream revents=" << stream_revents << " (should be POLLIN to read)" << std::endl;
            
            // Try to read data from stream
            std::cout << "Server: attempting to read..." << std::endl;
            auto read_result = stream.read();
            if (!read_result.is_error() && *read_result) {
              std::cout << "Server: read succeeded, data: ";
              stream.printBuffer();
              std::cout << std::endl;
            } else {
              std::cout << "Server: read returned false/error" << std::endl;
            }
            
            std::cout << "Server: stream going out of scope (will close fd=" << stream_fd << ")" << std::endl;
            // stream destructor will be called here - fd_refcount[stream_fd]-- and if 0, close(stream_fd)
          } else {
            std::cout << "Server: option is none" << std::endl;
          }
        }
        break;
      }
    }
    
    // Wait for child to finish
    int status;
    waitpid(pid, &status, 0);
    
    std::cout << "Server: test complete, listener fd=" << lis_fd << " still open" << std::endl;
    std::cout << "Test: Creating another listener to verify fd reuse" << std::endl;
    
    auto lis2 = Listener::connect(9091);
    if (!lis2.is_error()) {
      int lis2_fd = (*lis2).getFd();
      std::cout << "Listener 2 on port 9091, fd=" << lis2_fd << " (should be same as freed stream fd)" << std::endl;
    }
  } else {
    std::cerr << "fork() failed" << std::endl;
    return 1;
  }
  
  return 0;
}
