#include "Config_Server.hpp"
#include "Config.hpp"
Config_Server::Config_Server(std::ifstream &infile) {
  try {
    while (!infile.eof()) {
      std::string line;
      getline_stripspace(infile, line);
      if (line != "{")
        throw(std::runtime_error("Invalid Config File: Scope missing"));
      getline_stripspace(infile, line, ' ');
      if (line != "listen:")
        throw(std::runtime_error("Invalid Config File: invalid listener"));
      getline_stripspace(infile, line);
      // find :. no :? Save in first, set second to 80.
    }
  } catch (std::exception &e) {
    throw;
  }
}
