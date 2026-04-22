#include "Response.hpp"
#include "fcntl.h"
#include <unistd.h>
#include <cstring>

Response::Response() {
  reset();
}

Response::Response(const Response& in) {
  *this = in;
}

Response& Response::operator=(const Response& in) {
  if (this != &in) {
    _header_sent = in._header_sent;
    _head_offset = in._head_offset;
    _header_size = in._header_size;
    
    std::memcpy(_header, in._header, MAX_HEADER_SIZE);
    
    _filepath = in._filepath;
    _status_code = in._status_code;
    _content_type = in._content_type;

    _content_len = in._content_len;
    _body_offset = in._body_offset;
  }
  return (*this);
}

Response::~Response() {}

void Response::reset() {
  _header_sent = false;
  _head_offset = 0;
  _header_size = 0;
  
  std::memset(_header, 0, MAX_HEADER_SIZE);

  _filepath = StrSlice(); // is calculated by root + error/file/...
  _status_code = 0;
  _content_type = OTHER;

  _content_len = -1;
  _body_offset = 0;
  
}


Option<int> Response::handleRequest(const Request& req) {
  if (req.getMethod() == GET) {
    //heanlde method #todo
  }
  else {
   _status_code = 405;
  }
  return Option<int>(true);
}

size_t Response::chunker(char *tmp_buffer, size_t max_len) {
  if (tmp_buffer == NULL || max_len == 0) {
    return 0;
  }
  if (!_header_sent)
  {
    size_t remaining = _header_size - _head_offset;
    size_t to_copy = std::min(remaining, max_len);
    if (to_copy > 0) {
      std::memcpy(tmp_buffer, _header, to_copy);
      _head_offset += to_copy;
    }
    if (_head_offset >= _header_size)
      _header_sent = true;
    return to_copy;
  }
  return 0;
}

void Response::setBasicMessage() {
  //_message = std::string("HTTP/1.1 200 OK\r\nContent-Length: 869\r\nConnection: close\r\n\r\n\"hey\""); // remove
}

bool Response::isFullySent() const {
  return _header_sent && (_content_len == -1 || _body_offset >= _content_len);
}

Option<int> ft_open(const std::string &filename) {
  int fd = open(filename.c_str(), O_RDWR);

  if (fd == -1) {
    Option<int> rt(false);
    return (rt);
  }
  Option<int> rt(fd);
  return rt;
}
