#include "Config.hpp"
#include "Config_Route.hpp"
#include "Config_Server.hpp"
#include <fstream>
#include <iostream>

void print_server_values(Config_Server &server, int server_index) {
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
  std::cout << "Not Found Page: " << server.getNotFound() << std::endl;
  std::cout << "Unauthorized Page: " << server.getUnauthorized() << std::endl;
  std::cout << "Conflict Page: " << server.getConflict() << std::endl;
  std::cout << "Method Not Allowed Page: " << server.getMethodNotAllowed()
            << std::endl;

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

void print_route_values(Config_Route &route, int route_index) {
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

  std::cout << "Not Found Page: " << route.getNotFound() << std::endl;
  std::cout << "Unauthorized Page: " << route.getUnauthorized() << std::endl;
  std::cout << "Conflict Page: " << route.getConflict() << std::endl;
  std::cout << "Method Not Allowed Page: " << route.getMethodNotAllowed()
            << std::endl;

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

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
    return 1;
  }

  try {
    std::ifstream config_file(argv[1]);
    if (!config_file.is_open()) {
      std::cerr << "Error: Could not open config file: " << argv[1]
                << std::endl;
      return 1;
    }

    Config config(config_file);
    config_file.close();

    std::vector<Config_Server> &servers = config.getServers();

    std::cout << "\n╔════════════════════════════════════╗" << std::endl;
    std::cout << "║  CONFIG PARSE TEST - ALL VALUES   ║" << std::endl;
    std::cout << "╚════════════════════════════════════╝" << std::endl;
    std::cout << "\nTotal Servers: " << servers.size() << std::endl;

    for (size_t i = 0; i < servers.size(); ++i) {
      print_server_values(servers[i], i);

      std::vector<Config_Route> &routes = servers[i].getRoutes();
      std::cout << "\nRoutes in Server " << i << ": " << routes.size()
                << std::endl;

      for (size_t j = 0; j < routes.size(); ++j) {
        print_route_values(routes[j], j);
      }
    }

    std::cout << "\n\n════════════════════════════════════" << std::endl;
    std::cout << "Config parsing completed successfully!" << std::endl;
    std::cout << "════════════════════════════════════\n" << std::endl;

    return 0;
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
}
