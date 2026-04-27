#include "Config_Route.hpp"
#include <fstream>
Config_Route::Config_Route(std::ifstream &infile, Config_Server &server) {
  try {
    this->InterfacePort = server.getInterfacePort();
    this->ServerName = server.getServerName();
    this->root = server.getRoot();
    this->NotFound = server.getNotFound();
    this->Unauthorized = server.getUnauthorized();
    this->Conflict = server.getConflict();
    this->MethodNotAllowed = server.getMethodNotAllowed();
    this->AutoIndex = server.getAutoIndex();
    this->DefaultIndex = server.getDefaultIndex();
    this->Redirection = server.getRedirection();
    this->UploadAllowed = server.getUploadAllowed();
    this->MaxPayloadSize = server.getMaxPayloadSize();
  } catch (std::exception &e) {
    throw;
  }
}
