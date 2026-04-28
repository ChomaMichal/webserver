#include "Config_Route.hpp"
#include "Config_Server.hpp"
#include "utils.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
Config_Route::Config_Route() {}
Config_Route::~Config_Route() {}
bool Config_Route::are_fields_ready() {
  // std::cout << "IP " << this->InterfacePort.first << ":"
  //           << this->InterfacePort.second << std::endl;
  // std::cout << "Upload: " << this->UploadLocation << ":" <<
  // this->UploadAllowed
  //           << std::endl;
  if (this->InterfacePort.first == "")
    return false;
  if (this->UploadAllowed == true && this->UploadLocation == "")
    return false;
  // check if there's spaces inside the strings
  return true;
}
Config_Route::Config_Route(std::ifstream &infile, Config_Server &server) {
  this->InterfacePort = server.getInterfacePort();
  this->ServerName = server.getServerName();
  this->root = server.getRoot();
  this->IsDefault = server.getIsDefault();
  this->NotFound = server.getNotFound();
  this->Unauthorized = server.getUnauthorized();
  this->Conflict = server.getConflict();
  this->MethodNotAllowed = server.getMethodNotAllowed();
  this->AutoIndex = server.getAutoIndex();
  this->DefaultIndex = server.getDefaultIndex();
  this->Redirection = server.getRedirection();
  this->UploadAllowed = server.getUploadAllowed();
  this->UploadLocation = server.getUploadLocation();
  this->MaxPayloadSize = server.getMaxPayloadSize();
  std::string line;
  getline_stripspace(infile, line);
  this->Location = line;
  getline_stripspace(infile, line);
  if (line != "{" && !infile.eof()) {
    throw(std::runtime_error("Invalid Config File: Scope Missing"));
  }
  while (!infile.eof()) {
    getline_stripspace(infile, line, " \n");
    if (line == "root") {
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
    } else if (line == "}") {
      if (are_fields_ready())
        return;
      else
        throw std::runtime_error("Invalid Config File: Incomplete server data");
    } else {
      throw std::runtime_error("Invalid Config File: unrecognized directive: " +
                               line);
    }
  }
}
const std::pair<std::string, int> &Config_Route::getInterfacePort() {
  return this->InterfacePort;
}
const std::string &Config_Route::getServerName() { return this->ServerName; }
const bool &Config_Route::getIsDefault() { return this->IsDefault; }
const std::string &Config_Route::getRoot() { return this->root; }
const std::string &Config_Route::getNotFound() { return this->NotFound; }
const std::string &Config_Route::getUnauthorized() {
  return this->Unauthorized;
}
const std::string &Config_Route::getConflict() { return this->Conflict; }
const std::string &Config_Route::getMethodNotAllowed() {
  return this->MethodNotAllowed;
}
const bool &Config_Route::getAutoIndex() { return this->AutoIndex; }
const std::string &Config_Route::getDefaultIndex() {
  return this->DefaultIndex;
}
const std::pair<int, std::string> &Config_Route::getRedirection() {
  return this->Redirection;
}
const bool &Config_Route::getUploadAllowed() { return this->UploadAllowed; }
const std::string &Config_Route::getUploadLocation() {
  return this->UploadLocation;
}
const ssize_t &Config_Route::getMaxPayloadSize() {
  return this->MaxPayloadSize;
}
std::vector<Config_Route> &Config_Route::getRoutes() { return this->routes; }
std::string &Config_Route::getLocation() { return this->Location; }
