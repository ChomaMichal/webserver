#pragma once
#include "Config_Server.hpp"
#include <cstdlib>
#include <fstream>
#ifndef MAX_PATH
#define MAX_PATH 512;
#endif
class Config {
public:
  Config();
  Config(std::ifstream &infile);
  ~Config();
  const std::vector<Config_Server> &getServers() const;
  // Methods to get stuff
private:
  std::vector<Config_Server> _servers;
};
void getline_stripspace(std::ifstream &infile, std::string &line);
void getline_stripspace(std::ifstream &infile, std::string &line,
                        std::string delim);
void get_redirection_pair(std::string line, std::pair<int, std::string> pair);
