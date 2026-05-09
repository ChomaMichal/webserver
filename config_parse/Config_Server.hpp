#pragma once
#include "Config_Route.hpp"
#include <fstream>
#include <map>
#include <string>
#include <utility>
#include <vector>
class Config_Server {
public:
  Config_Server();
  Config_Server(std::ifstream &infile);
  virtual ~Config_Server();
  const std::pair<int, int> &getInterfacePort() const;
  const std::string &getServerName() const;
  const bool &getIsDefault() const;
  const std::string &getRoot() const;
  const bool &getAutoIndex() const;
  const std::string &getDefaultIndex() const;
  const std::pair<int, std::string> &getRedirection() const;
  const bool &getUploadAllowed() const;
  const std::string &getUploadLocation() const;
  const ssize_t &getMaxPayloadSize() const;
  const std::vector<Config_Route> &getRoutes() const;
  const std::map<int, std::string> &getErrors() const;
  const bool &getRedirectionSet() const;
  const bool &getRoutesExist() const;

protected:
  void init_members();
  void validate_server_members();
  bool are_fields_ready();
  std::pair<int, int> InterfacePort; // interface+port pair (example: 127.0.0.1,
                                     // 80). Port set to 80 by default
  std::string ServerName; // name of the server for matching. Can be NULL.
  std::vector<Config_Route> routes; // vector containing all routes.
  bool IsDefault;                   // is a default server config
  std::string root; // default root to be used if the routes don't have a
                    // root directive. CAN'T BE NULL
  /* Paths to error pages by default. Can be set, or will be populated by
   * default. Will never be NULL */
  std::string NotFound;
  std::string Unauthorized;
  std::string Conflict;
  std::string MethodNotAllowed;
  std::map<int, std::string> errors;
  /*                                                                                                  */
  bool AutoIndex; // allows autoindex. If on, if default index does not exist
                  // when queriying a directory, it will show the directory
                  // listing.
  std::string
      DefaultIndex; // path to index file to serve when requesting a directory.
                    // will be index.html by default, can't be NULL
  std::pair<int, std::string>
      Redirection; // int = code (error, redirection, etc), string = contents.
                   // String is never null, it contains quotes around what
                   // should be considered as string.
  bool UploadAllowed;
  std::string UploadLocation; // if upload_allowed == false, this is null
  ssize_t MaxPayloadSize;     // if -1, no payload max
  bool RedirectionSet;
  bool RoutesExist;
};
