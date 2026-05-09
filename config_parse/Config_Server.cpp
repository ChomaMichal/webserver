#include "Config_Server.hpp"
#include "Config.hpp"
#include "Config_Route.hpp"
#include "utils.hpp"
#include <stdexcept>
bool Config_Server::are_fields_ready() {
  if (this->UploadAllowed == true && this->UploadLocation == "")
    return false;
  // check if there's spaces inside the strings
  return true;
}
static void separate_interface_port_pairs(std::string line,
                                          std::pair<int, int> &pair) {

  size_t colon = line.find_first_of(':');
  if (colon == std::string::npos) {
    pair.first = ip_to_int(line);
    return;
  }
  std::string ip = line.substr(0, colon);
  pair.first = ip_to_int(ip);
  std::string sport = line.substr(colon + 1, std::string::npos);
  char *endptr;
  int iport = std::strtol(sport.c_str(), &endptr, 10);
  if (*endptr != '\0')
    throw("Invalid Config File: Invalid Port");
  pair.second = iport;
}
Config_Server::Config_Server() {}
Config_Server::~Config_Server() {}
void Config_Server::init_members() {

  this->InterfacePort.second = 80;
  this->InterfacePort.first =
      ip_to_int("0.0.0.0"); // change this to int and store that way
  this->ServerName = "";
  this->IsDefault = false;
  this->root = "root";
  this->errors[400] = "/errors/400.html";
  this->errors[403] = "/errors/403.html";
  this->errors[404] = "/errors/404.html";
  this->errors[405] = "/errors/405.html";
  this->errors[409] = "/errors/409.html";
  this->errors[413] = "/errors/413.html";
  this->errors[414] = "/errors/414.html";
  this->errors[415] = "/errors/415.html";
  this->errors[500] = "/errors/500.html";
  this->errors[501] = "/errors/501.html";
  this->errors[502] = "/errors/502.html";
  this->errors[503] = "/errors/503.html";
  this->errors[504] = "/errors/504.html";
  this->AutoIndex = true;
  this->DefaultIndex = "index.html";
  this->RedirectionSet = false;
  this->Redirection.first = -1;
  this->Redirection.second = "";
  this->UploadAllowed = false;
  this->RoutesExist = false;
  this->UploadLocation = "";
  this->MaxPayloadSize = 10485761;
}

Config_Server::Config_Server(std::ifstream &infile) {
  try {
    init_members();
    std::string line;
    getline_stripspace(infile, line);
    if (line != "{" && !infile.eof())
      throw(std::runtime_error("Invalid Config File: Scope missing"));
    while (!infile.eof()) {
      getline_stripspace(infile, line, " \n");
      if (line == "listen") {
        getline_stripspace(infile, line);
        separate_interface_port_pairs(line, this->InterfacePort);
      } else if (line == "server_name") {
        getline_stripspace(infile, line);
        this->ServerName = line;
      } else if (line == "default") {
        this->IsDefault = true;
      } else if (line == "root") {
        getline_stripspace(infile, line);
        this->root = line;
      } else if (line == "400") {
        getline_stripspace(infile, line);
        this->errors[400] = line;
      } else if (line == "403") {
        getline_stripspace(infile, line);
        this->errors[403] = line;
      } else if (line == "404") {
        getline_stripspace(infile, line);
        this->errors[404] = line;
      } else if (line == "405") {
        getline_stripspace(infile, line);
        this->errors[405] = line;
      } else if (line == "409") {
        getline_stripspace(infile, line);
        this->errors[409] = line;
      } else if (line == "413") {
        getline_stripspace(infile, line);
        this->errors[413] = line;
      } else if (line == "414") {
        getline_stripspace(infile, line);
        this->errors[414] = line;
      } else if (line == "415") {
        getline_stripspace(infile, line);
        this->errors[415] = line;
      } else if (line == "500") {
        getline_stripspace(infile, line);
        this->errors[500] = line;
      } else if (line == "501") {
        getline_stripspace(infile, line);
        this->errors[501] = line;
      } else if (line == "502") {
        getline_stripspace(infile, line);
        this->errors[502] = line;
      } else if (line == "503") {
        getline_stripspace(infile, line);
        this->errors[503] = line;
      } else if (line == "504") {
        getline_stripspace(infile, line);
        this->errors[504] = line;
      } else if (line == "autoindex") {
        getline_stripspace(infile, line);
        if (line == "on") {
          this->AutoIndex = true;
        } else if (line == "off") {
          this->AutoIndex = false;
        } else {
          throw std::runtime_error(
              "Invalid Config File: invalid autoindex status");
        }
      } else if (line == "index") {
        getline_stripspace(infile, line);
        this->DefaultIndex = line;
      } else if (line == "upload") {
        getline_stripspace(infile, line);
        if (line == "on") {
          this->UploadAllowed = true;
        } else if (line == "off") {
          this->UploadAllowed = false;
        } else {
          throw std::runtime_error(
              "Invalid Config File: invalid upload status");
        }
      } else if (line == "upload_location") {
        getline_stripspace(infile, line);
        this->UploadLocation = line;
      } else if (line == "max_payload_size") {
        getline_stripspace(infile, line);
        char *last;
        this->MaxPayloadSize = std::strtol(line.c_str(), &last, 10);
        if (*last != '\0')
          throw std::runtime_error("Invalid Config File: invalid payload size");
      } else if (line == "return") {
        getline_stripspace(infile, line);
        this->RedirectionSet = true;
        get_redirection_pair(line, this->Redirection);
      } else if (line == "location") {
        if (are_fields_ready())
          this->routes.push_back(Config_Route(infile, *this));
        else
          throw std::runtime_error(
              "Invalid Config File: Routes should always be at the end");
        RoutesExist = true;
      } else if (line == "}") {
        if (are_fields_ready())
          return;
        else
          throw std::runtime_error(
              "Invalid Config File: Incomplete server data");
      } else {
        throw std::runtime_error(
            "Invalid Config File: unrecognized directive: " + line);
      }
    }
  } catch (std::exception &e) {
    throw;
  }
}

const std::pair<int, int> &Config_Server::getInterfacePort() const {
  return this->InterfacePort;
}
const std::string &Config_Server::getServerName() const {
  return this->ServerName;
}
const bool &Config_Server::getIsDefault() const { return this->IsDefault; }
const bool &Config_Server::getAutoIndex() const { return this->AutoIndex; }
const std::string &Config_Server::getDefaultIndex() const {
  return this->DefaultIndex;
}
const std::pair<int, std::string> &Config_Server::getRedirection() const {
  return this->Redirection;
}
const bool &Config_Server::getUploadAllowed() const {
  return this->UploadAllowed;
}
const std::string &Config_Server::getUploadLocation() const {
  return this->UploadLocation;
}
const ssize_t &Config_Server::getMaxPayloadSize() const {
  return this->MaxPayloadSize;
}
const std::string &Config_Server::getRoot() const { return this->root; }
const std::vector<Config_Route> &Config_Server::getRoutes() const {
  return this->routes;
}

const bool &Config_Server::getRedirectionSet() const {
  return this->RedirectionSet;
}
const std::map<int, std::string> &Config_Server::getErrors() const {
  return this->errors;
}
const bool &Config_Server::getRoutesExist() const { return this->RoutesExist; }
