#pragma once
#include "../utils/option/Option.hpp"
#include "../utils/str_slice/StrSlice.hpp"
#include "Request.hpp"
#include <cstddef>
#include <iostream>

#ifndef MAX_HEADER_SIZE
#define MAX_HEADER_SIZE 8192
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
 Option<int> handleRequest(const Request& req); //#todo
 Option<bool> handleGet(const Request& req);
 //  Option<bool> handlePost(const Request& req);
 //  Option<bool> handleDelete(const Request& req);
 void setBasicMessage();

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
 
 const char * _root = "./root"; //alv
 StrSlice _filepath;
 
 const char *_http_version = "HTTP/1.1";
 int _status_code;
 
 const char *_server = "Server: webserver";
 const char *_header_connection_close = "Connection: close";
 const char *_header_content_length = "Content-Length: ";
 const char *_header_content_type = "Content-Type: ";
 const char *_header_allow_get_post_delete = "Allow: GET, POST, DELETE";

 e_content_type _content_type;
 ssize_t _content_len;

 size_t _body_offset;

//  std::string _body;

//  std::string _message
};