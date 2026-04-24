#include "Config_Server.hpp"
Config_Server::Config_Server(std::ifstream &infile) {
  try {
    while (!infile.eof()) {
      std::string line;
      getline(infile, line);
      if (line != "{")
        throw(std::runtime_error("Invalid Config File: Scope missing"));
      getline(infile, line, ' ');
      if (line != "listen:")
        throw(std::runtime_error("Invalid Config File: invalid listener"));
      getline(infile, line);
    }
  } catch (std::exception &e) {
    throw;
  }
}
