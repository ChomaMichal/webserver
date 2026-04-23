#pragma once
#include "Config_Servers.hpp"
class Config_Routes : public Config_Servers {
public:
  std::string root;
  std::string location;
};
