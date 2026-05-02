#include "Config_Route.hpp"
#include "Config_Server.hpp"
#include "utils.hpp"
#include <fstream>
#include <stdexcept>
Config_Route::Config_Route() {}
Config_Route::~Config_Route() {}
bool Config_Route::are_fields_ready() {
  // std::cout << "IP " << this->InterfacePort.first << ":"
  //           << this->InterfacePort.second << std::endl;
  // std::cout << "Upload: " << this->UploadLocation << ":" <<
  // this->UploadAllowed
  //           << std::endl;
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
  this->py_cgi_route = "";
  this->php_cgi_route = "";
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
    } else if (line == "cgi_extension") {
      getline_stripspace(infile, line, " \n");
      if (line == ".py") {
        getline_stripspace(infile, line);
        this->py_cgi_route = line;
      } else if (line == ".php") {
        getline_stripspace(infile, line);
        this->php_cgi_route = line;
      } else {
        throw std::runtime_error("Invalid Config File: invalid cgi directive");
      }
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
const std::pair<int, int> &Config_Route::getInterfacePort() const {
  return this->InterfacePort;
}
const std::string &Config_Route::getServerName() const {
  return this->ServerName;
}
const bool &Config_Route::getIsDefault() const { return this->IsDefault; }
const std::string &Config_Route::getRoot() const { return this->root; }
const std::string &Config_Route::getNotFound() const { return this->NotFound; }
const std::string &Config_Route::getUnauthorized() const {
  return this->Unauthorized;
}
const std::string &Config_Route::getConflict() const { return this->Conflict; }
const std::string &Config_Route::getMethodNotAllowed() const {
  return this->MethodNotAllowed;
}
const bool &Config_Route::getAutoIndex() const { return this->AutoIndex; }
const std::string &Config_Route::getDefaultIndex() const {
  return this->DefaultIndex;
}
const std::pair<int, std::string> &Config_Route::getRedirection() const {
  return this->Redirection;
}
const bool &Config_Route::getUploadAllowed() const {
  return this->UploadAllowed;
}
const std::string &Config_Route::getUploadLocation() const {
  return this->UploadLocation;
}
const ssize_t &Config_Route::getMaxPayloadSize() const {
  return this->MaxPayloadSize;
}
const std::vector<Config_Route> &Config_Route::getRoutes() const {
  return this->routes;
}
const std::string &Config_Route::getLocation() const { return this->Location; }
const std::string &Config_Route::getpy_cgi_route() const {
  return this->py_cgi_route;
}
const std::string &Config_Route::getphp_cgi_route() const {
  return this->php_cgi_route;
}
