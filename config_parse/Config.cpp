#include "Config.hpp"
#include "Config_Server.hpp"
#include <cstdio>
#include <stdexcept>
unsigned int ip_to_int(std::string &ip) {
  unsigned int a, b, c, d;
  ssize_t matches = sscanf(ip.c_str(), "%u.%u.%u.%u", &a, &b, &c, &d);
  if (matches != 4 || a > 255 || b > 255 || c > 255 || d > 255)
    throw std::runtime_error("Invalid Config File: malformed IP");
  return a << 24 | b << 16 | c << 8 | d;
}
unsigned int ip_to_int(const char *ip) {
  unsigned int a, b, c, d;
  ssize_t matches = sscanf(ip, "%u.%u.%u.%u", &a, &b, &c, &d);
  if (matches != 4 || a > 255 || b > 255 || c > 255 || d > 255)
    throw std::runtime_error("Invalid Config File: malformed IP");
  return a << 24 | b << 16 | c << 8 | d;
}
unsigned int ip_to_int(char *ip) {
  unsigned int a, b, c, d;
  ssize_t matches = sscanf(ip, "%u.%u.%u.%u", &a, &b, &c, &d);
  if (matches != 4 || a > 255 || b > 255 || c > 255 || d > 255)
    throw std::runtime_error("Invalid Config File: malformed IP");
  return a << 24 | b << 16 | c << 8 | d;
}
Config::Config() {}
Config::~Config() {}
void Config::Populate(std::ifstream &infile) {
  try {
    while (!infile.eof()) {
      std::string line;
      getline_stripspace(infile, line);
      if (line != "server:" && !infile.eof()) {
        throw(std::runtime_error(
            "Invalid Config File: invalid server directive"));
      }
      if (!infile.eof())
        this->_servers.push_back(Config_Server(infile));
      std::cout << "hi!" << std::endl;
    }
  } catch (std::exception &e) {
    throw e;
  }
}
Config::Config(std::ifstream &infile) {
  try {
    while (!infile.eof()) {
      std::string line;
      getline_stripspace(infile, line);
      if (line != "server:" && !infile.eof()) {
        throw(std::runtime_error(
            "Invalid Config File: invalid server directive"));
      }
      if (!infile.eof())
        this->_servers.push_back(Config_Server(infile));
      std::cout << "hi!" << std::endl;
    }
  } catch (std::exception &e) {
    throw e;
  }
}
static bool isStrSliceEqualtoString(const std::string &string,
                                    const StrSlice &slice) {
  size_t i = 0;
  while (string[i] && i < slice.getLen()) {
    if (string[i] != slice[i])
      return false;
  }
  return true;
}
const Config_Server &Config::match_server(int &port, int &ip,
                                          const StrSlice &host) const {
  std::vector<Config_Server>::const_iterator start = this->_servers.begin();
  std::vector<Config_Server>::const_iterator selectedserver =
      this->_servers.begin();
  bool default_set = false;
  bool allow_general_match = true;
  bool first_set = false;
  while (start != this->_servers.end()) {
    if (start->getInterfacePort().first == ip &&
        start->getInterfacePort().second == port) {
      allow_general_match = false;
      if (isStrSliceEqualtoString(start->getServerName(), host)) {
        return (*start);
      } else {
        if (start->getIsDefault() && default_set == false) {
          selectedserver = start;
          default_set = true;
        } else if (default_set == false && first_set == false) {
          selectedserver = start;
          first_set = true;
        }
      }
    } else if (start->getInterfacePort().first == 0 &&
               start->getInterfacePort().second == port &&
               allow_general_match == true) {
      selectedserver = start;
      allow_general_match = false;
    }
    start++;
  }
  return *selectedserver;
}
const std::vector<Config_Server> &Config::getServers() const {
  return this->_servers;
}
