#include "Config_Server.hpp"
#include "Config.hpp"

static void separate_interface_port_pairs(std::string line,
                                          std::pair<std::string, int> &pair) {

  size_t colon = line.find_first_of(':');
  if (colon == std::string::npos) {
    pair.first = line;
    return;
  }
  pair.first = line.substr(0, colon - 1);
  std::string sport = line.substr(colon + 1, std::string::npos);
  char *endptr;
  int iport = std::strtol(sport.c_str(), &endptr, 10);
  if (*endptr != '\0')
    throw("Invalid Config File: Invalid Port");
  pair.second = iport;
}
Config_Server::Config_Server(std::ifstream &infile) {
  try {
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
    while (!infile.eof()) {
      std::string line;
      getline_stripspace(infile, line);
      if (line != "{")
        throw(std::runtime_error("Invalid Config File: Scope missing"));
      else {
        getline_stripspace(infile, line, ' ');
        if (line != "listen")
          throw(std::runtime_error("Invalid Config File: invalid listener"));
        getline_stripspace(infile, line);
        separate_interface_port_pairs(line, this->InterfacePort);

        // find :. no :? Save in first, set second to 80.
      }
      getline_stripspace(infile, line);
      if (line == "}")
        return; // also check every member of the class;
      else
        throw(std::runtime_error("Invalid Config File: Scope missing"));
    }
  } catch (std::exception &e) {
    throw;
  }
}
