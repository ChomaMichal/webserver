#include "../sockets/Listener.hpp"
#include "../sockets/Networking.hpp"
#include "Client.hpp"
#include "Request.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <list>
#include <netinet/in.h>
#include <new>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include "../config_parse/Config.hpp"
#include "../config_parse/Config_Route.hpp"
#include "../config_parse/Config_Server.hpp"
#include <fstream>

void print_server_values(const Config_Server &server, int server_index) {
  std::cout << "\n========== SERVER " << server_index
            << " ==========" << std::endl;

  // Interface and Port
  const auto &interface_port = server.getInterfacePort();
  std::cout << "Interface: " << interface_port.first << std::endl;
  std::cout << "Port: " << interface_port.second << std::endl;

  // Server Name
  std::cout << "Server Name: " << server.getServerName() << std::endl;

  // Is Default
  std::cout << "Is Default: " << (server.getIsDefault() ? "true" : "false")
            << std::endl;

  // Root
  std::cout << "Root: " << server.getRoot() << std::endl;

  // Error Pages

  // AutoIndex and Default Index
  std::cout << "AutoIndex: " << (server.getAutoIndex() ? "true" : "false")
            << std::endl;
  std::cout << "Default Index: " << server.getDefaultIndex() << std::endl;

  // Redirection
  const auto &redirection = server.getRedirection();
  std::cout << "Redirection Code: " << redirection.first << std::endl;
  std::cout << "Redirection Content: " << redirection.second << std::endl;

  // Upload Settings
  std::cout << "Upload Allowed: "
            << (server.getUploadAllowed() ? "true" : "false") << std::endl;
  std::cout << "Upload Location: " << server.getUploadLocation() << std::endl;

  // Max Payload Size
  std::cout << "Max Payload Size: " << server.getMaxPayloadSize() << " bytes"
            << std::endl;
}

void print_route_values(const Config_Route &route, int route_index) {
  std::cout << "\n--- ROUTE " << route_index << " ---" << std::endl;

  // Location (route-specific)
  std::cout << "Location: " << route.getLocation() << std::endl;

  // All server properties apply to routes too
  const auto &interface_port = route.getInterfacePort();
  std::cout << "Interface: " << interface_port.first << std::endl;
  std::cout << "Port: " << interface_port.second << std::endl;

  std::cout << "Server Name: " << route.getServerName() << std::endl;
  std::cout << "Is Default: " << (route.getIsDefault() ? "true" : "false")
            << std::endl;
  std::cout << "Root: " << route.getRoot() << std::endl;

  std::cout << "AutoIndex: " << (route.getAutoIndex() ? "true" : "false")
            << std::endl;
  std::cout << "Default Index: " << route.getDefaultIndex() << std::endl;

  const auto &redirection = route.getRedirection();
  std::cout << "Redirection Code: " << redirection.first << std::endl;
  std::cout << "Redirection Content: " << redirection.second << std::endl;

  std::cout << "Upload Allowed: "
            << (route.getUploadAllowed() ? "true" : "false") << std::endl;
  std::cout << "Upload Location: " << route.getUploadLocation() << std::endl;

  std::cout << "Max Payload Size: " << route.getMaxPayloadSize() << " bytes"
            << std::endl;
}

bool g_exit = 0;

void handleSigInt(int sig) {
  (void)sig;
  g_exit = 1;
}

const int PORT = 2223;

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
    return 1;
  }
  Config config;
  try {
    std::ifstream config_file(argv[1]);
    if (!config_file.is_open()) {
      std::cerr << "Error: Could not open config file: " << argv[1]
                << std::endl;
      return 1;
    }

    config.Populate(config_file);
    config_file.close();

    const std::vector<Config_Server> &servers = config.getServers();

    std::cout << "\n╔════════════════════════════════════╗" << std::endl;
    std::cout << "║  CONFIG PARSE TEST - ALL VALUES   ║" << std::endl;
    std::cout << "╚════════════════════════════════════╝" << std::endl;
    std::cout << "\nTotal Servers: " << servers.size() << std::endl;

    for (size_t i = 0; i < servers.size(); ++i) {
      print_server_values(servers[i], i);

      const std::vector<Config_Route> &routes = servers[i].getRoutes();
      std::cout << "\nRoutes in Server " << i << ": " << routes.size()
                << std::endl;

      for (size_t j = 0; j < routes.size(); ++j) {
        print_route_values(routes[j], j);
      }
    }

    std::cout << "\n\n════════════════════════════════════" << std::endl;
    std::cout << "Config parsing completed successfully!" << std::endl;
    std::cout << "════════════════════════════════════\n" << std::endl;

  } catch (const std::exception &e) {
    std::cerr << "Error parsing config: " << e.what() << std::endl;
    return 1;
  } catch (const char *e) {
    std::cerr << "Error parsing config: " << e << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Unknown error occurred during parsing" << std::endl;
    return 1;
  }

  // signal(SIGINT, handleSigInt);
  Networking::init();
  auto lis = Listener::connect(PORT);
  if (lis.is_error()) {
    std::cerr << lis.get_error() << std::endl;
    return (1);
  }
  std::list<Client> arr;
  std::cout << "Send an HTTP request using: curl http://127.0.0.1:" << PORT
            << std::endl;
  while (1) {
    // std::cout << "update_fd_status == 0 :: 31" << std::endl;
    if (g_exit == 1) {
      break;
    }
    if (Networking::update_fd_status() == 0) {
      sleep(1);
      continue;
    }
    if ((*lis).getFdStatus() & POLLIN) {
      auto res = Stream::accept(*lis);
      // std::cout << "after accept loop :: 37" << std::endl;
      if (res.is_error()) {
        std::cerr << res.get_error() << std::endl;
        return (1);
      }
      auto stream = res.unwrap();
      if (stream.is_some()) {
        // std::cout << "stream is some :: 45 fd = " << stream.unwrap().getFd()
        // << std::endl; std::cout << "stream is some :: 45 pl_index = " <<
        // stream.unwrap().pl_index << std::endl;

        arr.push_back(*stream);
      }
      // std::cout << "Creating stream :: 45" << std::endl;
      if (Networking::update_fd_status() == 0) {
        sleep(1);
        continue;
      }
    }

    auto element = arr.begin();
    while (element != arr.end()) {
      // std::cout << "In element loop :: 49" << std::endl;

      if (!element->isResponseReady()) {
        // std::cout << "here\n";
        auto ret = element->recieveRequest();
        // std::cout << "After recieveRequest :: 54" << std::endl;

        if (ret.is_error()) {
          std::cerr << ret.get_error() << std::endl;
          element->close();
          element = arr.erase(element);
          continue;
        }

        if (*ret == false) {
          // std::cout << "if (*ret == false) : 59" << std::endl;
          element++;
          continue;
        }
        // std::cout << "main :: 217 :: request = " << element->getRequest() <<
        // std::endl;
        int addr = INADDR_ANY;
        int port = PORT;
        const Config_Server tmp =
            config.match_server(addr, port, element->getRequest().getHost());
        // std::cout << "main :: 219 :: tmp_getroot = " << tmp.getRoot() <<
        // std::endl;
        auto response_ret = element->setResponse(tmp);
        if (response_ret.is_error()) {
          std::cerr << response_ret.get_error() << std::endl;
          element->close();
          element = arr.erase(element);
          continue;
        }
      }

      if (element->isResponseReady()) {
        // std::cout << "in main before send response :: 80" << std::endl;
        auto send_ret = element->sendResponse();
        if (send_ret.is_error()) {
          std::cerr << send_ret.get_error() << std::endl;
          element->close();
          element = arr.erase(element);
          continue;
        }

        if (element->isResponseFullySent()) {
          element->close();
          element = arr.erase(element);
          continue;
        }
      }

      element++;
    }
    // std::cout << "breaks out of the loop :: 91" << std::endl;
  }
  return (0);
}
