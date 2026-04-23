#pragma once
#include "Config_Servers.hpp"
#include <fstream>
class Config {
public:
  Config();
  Config(std::ifstream &infile);
  ~Config();

private:
  std::vector<Config_Servers> _servers;
  // Methods to get stuff
};
