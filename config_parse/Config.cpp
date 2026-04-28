#include "Config.hpp"
#include <iostream>
#include <stdexcept>
Config::Config() {}
Config::~Config() {}
Config::Config(std::ifstream &infile) {
  try {
    while (!infile.eof()) {
      std::string line;
      getline_stripspace(infile, line);
      if (line != "server:" && !infile.eof())
        throw(std::runtime_error(
            "Invalid Config File: invalid server directive"));
      this->_servers.push_back(Config_Server(infile));
    }
  } catch (std::exception &e) {
    throw;
  }
}
std::vector<Config_Server> &Config::getServers() { return this->_servers; }
