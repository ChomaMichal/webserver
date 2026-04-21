#pragma once
#include "../utils/option/Option.hpp"
#include "../utils/str_slice/StrSlice.hpp"
#include "Request.hpp"
#include <cstddef>
#include <iostream>

class Response {
public:
 //SET OR RESET PRIVATE VALUES
 void reset();
//  void setFilePath();
//  void setCode(int code);

 //GET STUF
//  std::string getStatusReason();

 //HANDLE STUFF
//  Option<bool> handleRequest(const Request& req); #todo
//  Option<bool> handleGet(const Request& req);
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
 size_t _sent_offset;
 size_t _total_size;
 int _status_code;

 std::string _root;
 std::string _filepath;

 std::string _http_version;
 std::string _code;
 std::string _code_reason;
 std::string _status_line;
 
 std::string _server;
 std::string _content_type;
 std::string _header;

 std::string _body;

 std::string _message;
};