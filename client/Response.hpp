#pragma once
#include "../utils/cstd/cstd.hpp"
#include "../utils/result/Result.hpp"
#include "../utils/str_slice/StrSlice.hpp"
#include "../config_parse/Config.hpp"
#include "Request.hpp"
#include <cstddef>
#include <sys/stat.h>
#include <iostream>

#ifndef MAX_REQUEST_BODY_BYTES
# define MAX_REQUEST_BODY_BYTES 1048576 // 1MB
#endif

#ifndef MAX_HEADER_SIZE
# define MAX_HEADER_SIZE 8192
#endif

#ifndef ROOT_MAX
# define ROOT_MAX 256
#endif

#ifndef MAX_FILE_PATH
# define MAX_FILE_PATH (256 + ROOT_MAX)
#endif

enum e_content_type {
	HTML,
	CSS,
	JAVASCRIPT,
	JSON,
	PLAIN,
	PNG,
	JPEG,
	XICON,
	BIN,
	OTHER,	
};

class Response {
public:
 //SET OR RESET PRIVATE VALUES
 void reset();
//  void setFilePath();
//  void setCode(int code);

 //GET STUF
//  std::string getStatusReason();

 //HANDLE STUFF
 Result<bool> handleRequest(const Request& req, const Config_Server& serv); //#todo
 Result<bool> handleGet(const Request& req, const Config_Server& serv);
 Result<bool> handlePost(const Request& req, const Config_Server& serv);
 Result<bool> handleDelete(const Request& req, const Config_Server& serv);
 Result<bool> handleError(const Config_Server& serv);

 void setStatusCode(int status_code);
 bool getHeaderSent() const;
 void setFilePath(const Request& req);

 size_t chunker(char *tmp_buffer, size_t max_len);

 bool isFullySent() const;
 // OCF
 Response(); 
 Response(const Response& in);
 Response& operator=(const Response& in);
 ~Response();

private:
 bool _header_sent;

 size_t _head_offset;
 size_t _header_size;
 char _header[MAX_HEADER_SIZE];
 
 
 char _filepath[MAX_FILE_PATH];
 
 const char *_http_version = "HTTP/1.1 ";
 int _status_code;
 
 const char *_server = "Server: webserver"; // will change
 const char *_header_connection_close = "Connection: close"; // might change
 const char *_header_content_length = "Content-Length: ";
 const char *_header_content_type = "Content-Type: ";
 const char *_header_allow_get_post_delete = "Allow: GET, POST, DELETE";
 const char *_header_location = "Location: ";


 const Config_Route * _matched_route;
 bool _location;
 e_content_type _content_type;
 size_t _content_len;

 int _body_fd;
 size_t _body_offset;
 char _mem_body[MAX_HEADER_SIZE];
 bool _has_mem_body;
 const char * _root;
 const char * _error;
 size_t _uri_index;

 bool setHeader(const Config_Server& serv);
 void setContentType();
 bool fileStatRead(struct stat &, const Request &req, const Config_Server& serv);
 bool generateDirectoryIndex(const char *dir_path, const Request &req);
 const char * matchRouteToRoot(const Request& req, const std::vector<Config_Route>& routes);
};

// for michal <your class> popen(const char * _file_path, enum file_extention, const char *env_var);