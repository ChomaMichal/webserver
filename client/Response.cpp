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
    _sent_offset = in._sent_offset;
    _total_size = in._total_size;
    _status_code = in._status_code;

    _root = in._root;
    _filepath = in._filepath;

    _http_version = in._http_version;
    _code = in._code;
    _code_reason = in._code_reason;
    _status_line = in._status_line;

    _server = in._server;
    _content_type = in._content_type;
    _header = in._header;

    _body = in._body;
    _message = in._message;
  }
  return (*this);
}

Response::~Response() {}

void Response::reset() {
  _sent_offset = 0;
  _total_size = 0;
  _status_code = 0;

  _root.clear();
  _root = std::string("./root"); // will be given by alvarp
  _filepath.clear(); // is calculated by root + error/file/...

  _http_version = "HTTP/1.1"; // cannot change so maybe I'll keep it like this
  _code.clear();
  _code_reason.clear();
  _status_line.clear(); // becomes http_version + code + code reason

  _server = "webserver"; // can't change either
  _content_type.clear();
  _header.clear(); // all headers at once

  _body.clear(); // might be there

  _message.clear(); // the full combo of status_line + header + body(maybe)
}


size_t Response::chunker(char *tmp_buffer, size_t max_len) {
  if (tmp_buffer == NULL || max_len == 0) {
    return 0;
  }

  size_t remaining = _message.size() - _sent_offset;
  size_t to_copy = std::min(remaining, max_len);
  if (to_copy > 0) {
    std::memcpy(tmp_buffer, _message.data() + _sent_offset, to_copy);
    _sent_offset += to_copy;
  }
  return to_copy;
}

void Response::setBasicMessage() {
  _message = std::string("HTTP/1.1 200 OK\r\nContent-Length: 869\r\nConnection: close\r\n\r\n\"hey\"");
}

bool Response::isFullySent() const {
  return _sent_offset >= _message.size();
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
