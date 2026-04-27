#pragma once
#include "Config_Server.hpp"
class Config_Route : public Config_Server {
public:
  Config_Route();
  Config_Route(std::ifstream &infile, Config_Server &server);
  virtual ~Config_Route();
  std::string &getRoot();
  std::string &getLocation();

private:
  std::string Root;
  std::string Location;
};
