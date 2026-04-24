#include "Response.hpp"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>

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
    
    std::memcpy(_filepath, in._filepath, MAX_FILE_PATH);
    _status_code = in._status_code;
    _content_type = in._content_type;

    _content_len = in._content_len;
    _body_fd = in._body_fd;
    _body_offset = in._body_offset;
  }
  return (*this);
}

Response::~Response() {}

void Response::reset() {
  if (_body_fd != -1) {
    close(_body_fd);
  }
  _header_sent = false;
  _head_offset = 0;
  _header_size = 0;
  
  std::memset(_header, 0, MAX_HEADER_SIZE);

  std::memset(_filepath, 0, MAX_FILE_PATH);
  _status_code = 200;
  _content_type = OTHER;

  _content_len = -1;
  _body_fd = -1;
  _body_offset = 0;
  
}

void Response::setFilePath(const char *path) {
  if (path == NULL || !*path) {
    _filepath[0] = '\0';
    return;
  }
  std::strncpy(_filepath, path, MAX_FILE_PATH - 1);
  _filepath[MAX_FILE_PATH - 1] = '\0';
}

void Response::setContentType() {
  const char *ext = std::strrchr(_filepath, '.');

  if (ext == NULL || *(ext + 1) == '\0') {
    return;
  }

  switch (*(ext + 1)) {
    case 'h':
      if (std::strcmp(ext, ".html") == 0 || std::strcmp(ext, ".htm") == 0) {
        _content_type = HTML;
      }
      break;
    case 'c':
      if (std::strcmp(ext, ".css") == 0) {
        _content_type = CSS;
      }
      break;
    case 'j':
      if (std::strcmp(ext, ".js") == 0) {
        _content_type = JAVASCRIPT;
      }
      else if (std::strcmp(ext, ".json") == 0) {
        _content_type = JSON;
      }
      else if (std::strcmp(ext, ".jpeg") == 0 || std::strcmp(ext, ".jpg") == 0) {
        _content_type = JPEG;
      }
      break;
    case 'p':
      if (std::strcmp(ext, ".png") == 0) {
        _content_type = PNG;
      }
      break;
    case 'i':
      if (std::strcmp(ext, ".ico") == 0) {
        _content_type = XICON;
      }
      break;
    case 't':
      if (std::strcmp(ext, ".txt") == 0) {
        _content_type = PLAIN;
      }
      break;
    default:
      _content_type = OTHER;
      break;
  }
}

Result<bool> Response::handleRequest(const Request& req) {
  if (req.getMethod() == GET) {
    return handleGet(req);
  }
  // else if (req.getMethod() == POST) {
  //   return handlePost(req);
  // }
  // else if (req.getMethod() == DELETE) {
  //   return handleDelete(req);
  // }
  _status_code = 405;
  return handleError();
}

bool Response::file_stat(struct stat& _file_stat) {
  errno = 0;
  if (stat(_filepath, &_file_stat) == -1) {
    if (errno == ENOENT) {
      return (_status_code = 404, false);
    }
    if (errno == EACCES) {
      return (_status_code = 403, false);
    }
    return (_status_code = 500, false);
  }
  errno = 0;
  if (S_ISDIR(_file_stat.st_mode) || S_ISLNK(_file_stat.st_mode) || !S_ISREG(_file_stat.st_mode)) {
    return (_status_code = 403, false);
  }
  if (access(_filepath, R_OK) == -1) {
    return (_status_code = 403, false);
  }
  setContentType();
  if (_content_type == OTHER) {
    return (_status_code = 415, false);
  }
  if (_file_stat.st_size > MAX_REQUEST_BODY_BYTES) {
    return (_status_code = 413, false);
  }
  _body_fd = open(_filepath, O_RDONLY);
  if (_body_fd == -1) {
    return (_status_code = 500, false);
  }

  _content_len = _file_stat.st_size;
  return (true);
}

bool memcat(char *dest, const char *src, size_t n, size_t dest_max_len) {
  if (dest == NULL || src == NULL || dest_max_len == 0) {
    return false;
  }

  size_t dest_len = std::strlen(dest);
  if (dest_len >= dest_max_len) {
    return false;
  }

  size_t space_left = dest_max_len - dest_len - 1;
  if (n > space_left) {
    return false;
  }

  std::memcpy(dest + dest_len, src, n);
  dest[dest_len + n] = '\0';
  return true;
}

const char * find_code_reason(int code) {
  switch (code) {
    case 200: return "OK";
    case 201: return "Created";
    case 204: return "No Content";
    case 301: return "Moved Permanently";
    case 302: return "Found";
    case 400: return "Bad Request";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 413: return "Payload Too Large";
    case 414: return "URI Too Long";
    case 415: return "Unsupported Media Type";
    case 500: return "Internal Server Error";
    case 501: return "Not Implemented";
    case 502: return "Bad Gateway";
    case 503: return "Service Unavailable";
    case 504: return "Gateway Timeout";
    default:  return "Unknown Status";
  }
  return "Unknown Status";
}

const char * find_content_type(e_content_type type) {
  switch (type) {
    case HTML: return "text/html";
    case CSS: return "text/css";
    case JAVASCRIPT: return "application/javascript";
    case JSON: return "application/json";
    case PLAIN: return "text/plain";
    case PNG: return "image/png";
    case JPEG: return "image/jpeg";
    case XICON: return "image/x-icon";
    case OTHER: return "application/octet-stream";
    default: return "what/other";
  }
}

bool numcat(char *dest, ssize_t i, size_t max_dest_len) {
  if (i == -1)
    return memcat(dest, "-1", 2, max_dest_len);
  else if (i < 0)
    return false;
  else if (i > 9) {
    if (!numcat(dest, i / 10, max_dest_len))
      return false;
    if (!numcat(dest, i % 10, max_dest_len))
      return false;
  }
  else {
    const char c[2] = {(const char)(i % 10 + '0'), 0};
    if (!memcat(dest, c, 1, max_dest_len))
      return false;
  }
  return true;
}

bool Response::setHeader() {
  std::memset(_header, 0, MAX_HEADER_SIZE);
  _head_offset = 0;
  _header_sent = false;
  std::memcpy(_header, _http_version, std::strlen(_http_version) + 1);

  const char code[5] = {(const char)(_status_code / 100 + '0'),
              (const char)(_status_code / 10 % 10 + '0'),
              (const char)(_status_code % 10 + '0'), ' ', 0};

  if (!memcat(_header, code, 4, MAX_HEADER_SIZE))
    return false;

  const char *reason = find_code_reason(_status_code);
  if (!memcat(_header, reason, std::strlen(reason), MAX_HEADER_SIZE))
    return false;

  if (!memcat(_header, "\r\n", 2, MAX_HEADER_SIZE))
    return false;

  if (!memcat(_header, _server, std::strlen(_server), MAX_HEADER_SIZE))
    return false;
  if (!memcat(_header, "\r\n", 2, MAX_HEADER_SIZE))
    return false;

  if (!memcat(_header, _header_content_type, std::strlen(_header_content_type), MAX_HEADER_SIZE))
    return false;
  const char *content_type = find_content_type(_content_type);
  if (!memcat(_header, content_type, std::strlen(content_type), MAX_HEADER_SIZE))
    return false;

  if (!memcat(_header, "\r\n", 2, MAX_HEADER_SIZE))
    return false;

  if (!memcat(_header, _header_content_length, std::strlen(_header_content_length), MAX_HEADER_SIZE))
    return false;
  if (!numcat(_header, _content_len, MAX_HEADER_SIZE))
    return false;
  if (!memcat(_header, "\r\n", 2, MAX_HEADER_SIZE))
    return false;
  
  if (!memcat(_header, _header_connection_close, std::strlen(_header_connection_close), MAX_HEADER_SIZE))
    return false;

  if (!memcat(_header, "\r\n\r\n", 4, MAX_HEADER_SIZE))
    return false;

  _header_size = std::strlen(_header);

  return true;
}

Result<bool> Response::handleGet(const Request& req) {
  struct stat _file_stat;
  if (!file_stat(_file_stat))
    return (handleError());
  if (!setHeader())
    return Result<bool>("Cannot set head shit has hit the fan");
  (void)req;
  bool ok = true;
  return Result<bool>(ok);
}

Result<bool> Response::handlePost(const Request& req) {
  (void)req;
  bool ok = true;
  return Result<bool>(ok);
}

Result<bool> Response::handleDelete(const Request& req) {
  (void)req;
  bool ok = true;
  return Result<bool>(ok);
}

Result<bool> Response::handleError() {
  if (_body_fd != -1) {
    close(_body_fd);
    _body_fd = -1;
  }
  _content_type = PLAIN;
  _content_len = 0;
  if (!setHeader())
    return Result<bool>("Cannot set error header");
  bool ok = true;
  return Result<bool>(ok);
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

  if (_body_fd == -1 || _body_offset >= (size_t)_content_len) {
    if (_body_fd != -1) {
      close(_body_fd);
      _body_fd = -1;
    }
    return 0;
  }

  size_t remaining_body = (size_t)_content_len - _body_offset;
  size_t to_read = std::min(max_len, remaining_body);
  ssize_t read_len = read(_body_fd, tmp_buffer, to_read);
  if (read_len <= 0) {
    close(_body_fd);
    _body_fd = -1;
    return 0;
  }
  _body_offset += (size_t)read_len;

  if (_body_offset >= (size_t)_content_len) {
    close(_body_fd);
    _body_fd = -1;
  }

  return (size_t)read_len;
  return 0;
}

void Response::setBasicMessage() {
  //_message = std::string("HTTP/1.1 200 OK\r\nContent-Length: 869\r\nConnection: close\r\n\r\n\"hey\""); // remove
}

bool Response::getHeaderSent() const {
  return _header_sent;
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
