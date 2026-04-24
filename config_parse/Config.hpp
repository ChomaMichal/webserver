#pragma once
#include "Config_Server.hpp"
#include <fstream>
#ifndef MAX_PATH
#define MAX_PATH 512;
#endif
class Config {
public:
  Config();
  Config(std::ifstream &infile);
  ~Config();
  std::vector<Config_Server> &getServers();
  // Methods to get stuff
private:
  std::vector<Config_Server> _servers;
};
void getline_stripspace(std::ifstream &infile, std::string &line);
void getline_stripspace(std::ifstream &infile, std::string &line, char delim);
