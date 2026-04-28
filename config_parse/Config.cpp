#include "Config.hpp"
#include <stdexcept>

Config::Config(std::ifstream &infile) {
  try {

    while (!infile.eof()) {
      std::string line;
      getline_stripspace(infile, line);
      if (line != "server:")
        throw(std::runtime_error(
            "Invalid Config File: invalid server directive"));
      this->_servers.push_back(Config_Server(infile));
    }
  } catch (std::exception &e) {
    throw;
  }
}
void getline_stripspace(std::ifstream &infile, std::string &line) {
  getline(infile, line);
  line.erase(0, line.find_first_not_of(" \t"));
  line.erase(line.find_last_not_of(" \t") + 1);
}
void getline_stripspace(std::ifstream &infile, std::string &line, char delim) {
  getline(infile, line, delim);
  line.erase(0, line.find_first_not_of(" \t"));
  line.erase(line.find_last_not_of(" \t") + 1);
}
void get_redirection_pair(std::string line, std::pair<int, std::string> pair) {
  size_t space = line.find_first_of(' ');
  if (space == std::string::npos) {
    char *endptr;
    pair.first = std::strtol(line.c_str(), &endptr, 10);
    if (*endptr != '\0')
      throw("Invalid Config File: Invalid Redirection");
  } else {
    std::string sredir = line.substr(0, space);
    char *endptr;
    pair.first = std::strtol(line.c_str(), &endptr, 10);
    if (*endptr != '\0')
      throw("Invalid Config File: Invalid Redirection");
    pair.second = line.substr(space + 1, std::string::npos);
  }
}
