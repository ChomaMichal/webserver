#include "Config_Route.hpp"
#include "Config.hpp"
#include <fstream>
#include <stdexcept>
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
  this->MaxPayloadSize = server.getMaxPayloadSize();
  std::string line;
  getline_stripspace(infile, line);
  this->Location = line;
  getline_stripspace(infile, line);
  if (line != "{") {
    throw(std::runtime_error("Invalid Config File: Scope Missing"));
  }
  while (!infile.eof()) {
    getline_stripspace(infile, line, ' ');
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
    } else if (line == "}") {
      if (are_fields_ready())
        return;
      else
        throw std::runtime_error("Invalid Config File: Incomplete server data");
    }
  }
  throw(std::runtime_error("Invalid Config File: Scope missing"));
}
std::string &Config_Route::getLocation() { return this->Location; }
