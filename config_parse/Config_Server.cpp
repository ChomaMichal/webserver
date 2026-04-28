#include "Config_Server.hpp"
#include "Config.hpp"
#include "Config_Route.hpp"
#include <stdexcept>

bool Config_Server::are_fields_ready() {
  if (this->InterfacePort.first == "")
    return false;
  if (this->UploadAllowed == true && this->UploadLocation == "")
    return false;
  // check if there's spaces inside the strings
  return true;
}
static void separate_interface_port_pairs(std::string line,
                                          std::pair<std::string, int> &pair) {

  size_t colon = line.find_first_of(':');
  if (colon == std::string::npos) {
    pair.first = line;
    return;
  }
  pair.first = line.substr(0, colon);
  std::string sport = line.substr(colon + 1, std::string::npos);
  char *endptr;
  int iport = std::strtol(sport.c_str(), &endptr, 10);
  if (*endptr != '\0')
    throw("Invalid Config File: Invalid Port");
  pair.second = iport;
}

static void get_redirection_pair(std::string line,
                                 std::pair<int, std::string> pair) {
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
void Config_Server::init_members() {

  this->InterfacePort.second = 80;
  this->InterfacePort.first = "";
  this->ServerName = "";
  this->IsDefault = false;
  this->root = "root";
  this->NotFound = "notfound.html"; // change these to correct paths
  this->Unauthorized = "unauthorized.html";
  this->Conflict = "conflict.html";
  this->MethodNotAllowed = "notallowed.html";
  this->AutoIndex = true;
  this->DefaultIndex = "index.html";
  this->Redirection.first = -1;
  this->Redirection.second = "";
  this->UploadAllowed = false;
  this->UploadLocation = "";
  this->MaxPayloadSize = -1;
}

Config_Server::Config_Server(std::ifstream &infile) {
  try {
    init_members();
    std::string line;
    getline_stripspace(infile, line);
    if (line != "{")
      throw(std::runtime_error("Invalid Config File: Scope missing"));
    while (!infile.eof()) {
      getline_stripspace(infile, line, ' ');
      if (line == "listen") {
        getline_stripspace(infile, line);
        separate_interface_port_pairs(line, this->InterfacePort);
      } else if (line == "server_name") {
        getline_stripspace(infile, line);
        this->ServerName = line;
      } else if (line == "default") {
        getline_stripspace(infile, line);
        this->IsDefault = true;
      } else if (line == "root") {
        getline_stripspace(infile, line);
        this->root = line;
      } else if (line == "404") {
        getline_stripspace(infile, line);
        this->NotFound = line;
      } else if (line == "401") {
        getline_stripspace(infile, line);
        this->Unauthorized = line;
      } else if (line == "409") {
        getline_stripspace(infile, line);
        this->Conflict = line;
      } else if (line == "405") {
        getline_stripspace(infile, line);
        this->MethodNotAllowed = line;
      } else if (line == "autoindex") {
        getline_stripspace(infile, line);
        if (line == "on") {
          this->AutoIndex = true;
        }
        if (line == "off") {
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
        }
        if (line == "off") {
          this->UploadAllowed = false;
        } else {
          throw std::runtime_error(
              "Invalid Config File: invalid autoindex status");
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
        get_redirection_pair(line, this->Redirection);
      } else if (line == "location") {
        if (are_fields_ready())
          this->routes.push_back(Config_Route(infile, *this));
        else
          throw std::runtime_error(
              "Invalid Config File: Routes should always be at the end");
      } else if (line == "}") {
        if (are_fields_ready())
          return;
        else
          throw std::runtime_error(
              "Invalid Config File: Incomplete server data");
      }
    }
    throw(std::runtime_error("Invalid Config File: Scope missing"));
  } catch (std::exception &e) {
    throw;
  }
}

const std::pair<std::string, int> &Config_Server::getInterfacePort() {
  return this->InterfacePort;
}
const std::string &Config_Server::getServerName() { return this->ServerName; }
const bool &Config_Server::getIsDefault() { return this->IsDefault; }
const std::string &Config_Server::getNotFound() { return this->NotFound; }
const std::string &Config_Server::getUnauthorized() {
  return this->Unauthorized;
}
const std::string &Config_Server::getConflict() { return this->Conflict; }
const std::string &Config_Server::getMethodNotAllowed() {
  return this->MethodNotAllowed;
}
const bool &Config_Server::getAutoIndex() { return this->AutoIndex; }
const std::string &Config_Server::getDefaultIndex() {
  return this->DefaultIndex;
}
const std::pair<int, std::string> &Config_Server::getRedirection() {
  return this->Redirection;
}
const bool &Config_Server::getUploadAllowed() { return this->UploadAllowed; }
const std::string &Config_Server::getUploadLocation() {
  return this->UploadLocation;
}
const ssize_t &Config_Server::getMaxPayloadSize() {
  return this->MaxPayloadSize;
}
const std::string &Config_Server::getRoot() { return this->root; }
