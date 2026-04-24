#include "Config.hpp"
#include <stdexcept>
Config::Config(std::ifstream &infile) {
  try {

    while (!infile.eof()) {
      std::string line;
      getline(infile, line);
      if (line != "Server:")
        throw(std::runtime_error("Invalid Config File"));
      this->_servers.push_back(Config_Server(infile));
    }
  } catch (std::exception &e) {
    throw;
  }
}
