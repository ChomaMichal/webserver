#include <string>
#include <vector>
#pragma once

typedef struct s_route {
  std::string root;
  std::string location;
  /* these should be set to the ones from t_config by default and overriden as
   * needed if defined */
  std::string Not_Found;
  std::string Unauthorized;
  std::string Conflict;
  std::string Method_Not_Allowed;
  bool autoindex;
  std::string default_index;               // if none, should be index.html
  std::pair<int, std::string> Redirection; // int = code, string = contents.
  bool upload_allowed;
  std::string upload_location;

} t_route;
typedef struct s_config { // there might be multiple of these, server blocks
  std::pair<std::string, int>
      Interface_Port; // interface+port pair (example: 127.0.0.1, 80). Port set
                      // to 80 by default
  std::string server_name; // name of the server for matching. Can be NULL.
  std::string root; // default root to be used if the routes don't have a root
                    // directive. CAN'T BE NULL
  /* Paths to error pages by default. Can be set, or will be populated by
   * default. Will never be NULL */
  std::string Not_Found;
  std::string Unauthorized;
  std::string Conflict;
  std::string Method_Not_Allowed;
  /*                                                                                                  */
  bool autoindex; // allows autoindex. If on, if default index does not exist
                  // when queriying a directory, it will show the directory
                  // listing.
  std::string
      default_index; // path to index file to serve when requesting a directory.
                     // will be index.html by default, can't be NULL
  std::pair<int, std::string>
      Redirection; // int = code (error, redirection, etc), string = contents.
                   // String is never null, it contains quotes around what
                   // should be considered as string.
  bool upload_allowed;
  std::string upload_location; // if upload_allowed == false, this is null.
  bool is_default;             // is a default server config
  std::vector<t_route> routes; // vector containing all routes.
} t_config;
