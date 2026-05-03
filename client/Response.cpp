#include "Response.hpp"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <dirent.h>

Response::Response() : _body_fd(-1) {
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
    _location = in._location;
    _content_len = in._content_len;
    _body_fd = in._body_fd;
    _body_offset = in._body_offset;
    _has_mem_body = in._has_mem_body;
    std::memcpy(_mem_body, in._mem_body, MAX_HEADER_SIZE);
  }
  return (*this);
}

Response::~Response() {}

void Response::reset() {
  if (_body_fd != -1) {
    close(_body_fd);
  }
  _has_mem_body = false;
  _header_sent = false;
  _head_offset = 0;
  _header_size = 0;
  
  std::memset(_mem_body, 0, MAX_HEADER_SIZE);
  std::memset(_header, 0, MAX_HEADER_SIZE);

  std::memset(_filepath, 0, MAX_FILE_PATH);
  _status_code = 200;
  _content_type = OTHER;
  _location = 0;
  _content_len = 0;
  _body_fd = -1;
  _body_offset = 0;
  
}

void Response::setFilePath(const Request& req, const Config_Server& serv) {
  const char * root = serv.getRoot().c_str();
  StrSlice uri = req.getRequestURI();
  char full_path[MAX_FILE_PATH];
  size_t out_len = 0;
  size_t root_len = std::strlen(root);

  std::memcpy(full_path, root, root_len);
  out_len = root_len;

  if (out_len > 0 && full_path[out_len - 1] != '/') {
    full_path[out_len] = '/';
    out_len++;
  }

  size_t i = 0;
  if (uri.getLen() > 0 && uri[0] == '/') {
    i = 1;
  }


  if (i >= uri.getLen() && req.getMethod() == GET) { // only "/" or "" in URI
    const char *index_file = "index.html";
    size_t n = std::strlen(index_file);
    if (out_len + n >= MAX_FILE_PATH) {
      std::memcpy(_filepath, "", 1);
      return ;
    }
    std::memcpy(full_path + out_len, index_file, n);
    out_len += n;
  }
  else {
    for (; i < uri.getLen(); ++i) {
      if (out_len + 1 >= MAX_FILE_PATH) {
        std::memcpy(_filepath, "", 1);
        return ;
      }
      // std::cout << uri[i] << std::endl;
      full_path[out_len] = uri[i];
      out_len++;
    }
  }

  full_path[out_len] = 0;
  std::memcpy(_filepath, full_path, out_len + 1);
}

void Response::setContentType() {
  const char *ext = std::strrchr(_filepath, '.');

  if (ext == NULL || *(ext + 1) == 0) {
    return;
  }

  switch (*(ext + 1)) {
    case 'b':
      if (std::strcmp(ext, ".bin") == 0) {
        _content_type = BIN;
      }
      break;
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

Result<bool> Response::handleRequest(const Request& req, const Config_Server& serv) {
  // std::cout << "Response :: 127 _" << _filepath << std::endl;
  // TODO: setFilePath
  setFilePath(req, serv);
  if (req.getMethod() == GET) {
    return handleGet(req, serv);
  }
  else if (req.getMethod() == POST) {
    return handlePost(req, serv);
  }
  else if (req.getMethod() == DELETE) {
    return handleDelete(req, serv);
  }
  _status_code = 405;
  return handleError(serv);
}

bool Response::file_stat_read(struct stat& _file_stat, const Request &req) {
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

  if (S_ISDIR(_file_stat.st_mode)) {
    char index_path[MAX_FILE_PATH];
    std::strncpy(index_path, _filepath, MAX_FILE_PATH - 1);
    index_path[MAX_FILE_PATH - 1] = 0;

    size_t len = std::strlen(index_path);
    if (len > 0 && index_path[len - 1] != '/') {
      if (len < MAX_FILE_PATH - 1) {
        index_path[len] = '/';
        index_path[len + 1] = 0;
      }
    }
    
    char const *index_file = "index.html";
    if (std::strlen(index_path) + std::strlen(index_file) < MAX_FILE_PATH) {
      std::strcat(index_path, index_file);
      struct stat idx_stat;
      if (stat(index_path, &idx_stat) == 0 && S_ISREG(idx_stat.st_mode) && access(index_path, R_OK) == 0) {
        std::strcpy(_filepath, index_path);
        _file_stat = idx_stat;
      } else {
        return generateDirectoryIndex(_filepath, req);
      }
    } else {
      return generateDirectoryIndex(_filepath, req);
    }
  }

  if (S_ISLNK(_file_stat.st_mode) || !S_ISREG(_file_stat.st_mode)) {
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
  dest[dest_len + n] = 0;
  return true;
}

bool Response::generateDirectoryIndex(const char *dir_path, const Request &req) {
  DIR* dir = opendir(dir_path);
  if (!dir) {
    _status_code = 403;
    return false;
  }

  char uri_path[MAX_FILE_PATH];
  StrSlice req_uri = req.getRequestURI();
  size_t copy_len = req_uri.getLen();
  std::memcpy(uri_path, &req_uri.at(0), copy_len);
  uri_path[copy_len] = 0;
  
  if (copy_len > 0 && uri_path[copy_len - 1] != '/') {
    uri_path[copy_len] = '/';
    uri_path[copy_len + 1] = 0;
  }

  _mem_body[0] = 0;
  size_t max_len = sizeof(_mem_body);

  memcat(_mem_body, "<html><head><title>Index of ", std::strlen("<html><head><title>Index of "), max_len);
  memcat(_mem_body, uri_path, std::strlen(uri_path), max_len);
  memcat(_mem_body, "</title></head><body><h1>Index of ", std::strlen("</title></head><body><h1>Index of "), max_len);
  memcat(_mem_body, uri_path, std::strlen(uri_path), max_len);
  memcat(_mem_body, "</h1><hr><ul>\n", std::strlen("</h1><hr><ul>\n"), max_len);

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (std::strcmp(entry->d_name, ".") == 0) {
      continue;
    }
    char path_buf[MAX_FILE_PATH] = {};
    memcat(path_buf, dir_path, std::strlen(dir_path), sizeof(path_buf));
    memcat(path_buf, "/", 1, sizeof(path_buf));
    memcat(path_buf, entry->d_name, std::strlen(entry->d_name), sizeof(path_buf));
    
    struct stat s_st;
    bool is_dir = true;
    if (stat(path_buf, &s_st) == 0 && !S_ISDIR(s_st.st_mode)) {
      is_dir = false;
    }

    char target[MAX_FILE_PATH];
    target[0] = 0;
    memcat(target, uri_path, std::strlen(uri_path), sizeof(target));
    memcat(target, entry->d_name, std::strlen(entry->d_name), sizeof(target));

    char link[2048];
    link[0] = 0;
    size_t link_max = sizeof(link);
    
    memcat(link, "<li><a href=\"", 13, link_max);
    memcat(link, target, std::strlen(target), link_max);
    if (is_dir) {
      memcat(link, "/", 1, link_max);
    }
    memcat(link, "\">", 2, link_max);
    memcat(link, entry->d_name, std::strlen(entry->d_name), link_max);
    if (is_dir) {
      memcat(link, "/", 1, link_max);
    }
    memcat(link, "</a></li>\n", 10, link_max);

    memcat(_mem_body, link, std::strlen(link), max_len);
  }
  closedir(dir);

  if (!memcat(_mem_body, "</ul><hr></body></html>\n", std::strlen("</ul><hr></body></html>\n"), max_len)) {
    _status_code = 413;
    return false;
  }
  _content_len = std::strlen(_mem_body);
  _content_type = HTML;
  _has_mem_body = true;
  _status_code = 200;
  return true;
}

const char * find_code_reason(int code) {
  switch (code) {
    case 200: return "OK";
    case 201: return "Created";
    case 204: return "No Content";
    case 301: return "Moved Permanently";
    case 302: return "Found";
    case 307: return "TODO";
    case 400: return "Bad Request";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 409: return "Conflict";
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
    case BIN: return "application/octet-stream";
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

bool Response::setHeader(const Config_Server& serv) {
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

Result<bool> Response::handleGet(const Request& req, const Config_Server& serv) {
  struct stat _file_stat;
  if (!file_stat_read(_file_stat, req))
    return (handleError(serv));
  if (!setHeader(serv))
    return Result<bool>("Cannot set head shit has hit the fan");
  (void)req;
  bool ok = true;
  return Result<bool>(ok);
}

static bool check_post_parent_dir(const char *filepath, int &status_code) {
  if (filepath == NULL || *filepath == 0) {
    status_code = 400;
    return false;
  }

  char parent[MAX_FILE_PATH] = {};
  const char *last_slash = std::strrchr(filepath, '/');

  if (last_slash == NULL) {
    std::memmove(parent, ".", 1);
  }
  else if (last_slash == filepath) {
    std::memmove(parent, "/", 1);
  }
  else {
    int to_copy = last_slash - filepath;
    std::strncpy(parent, filepath, to_copy);
    parent[to_copy] = 0;
  }

  struct stat st;
  if (stat(parent, &st) == -1) {
    if (errno == ENOENT) {
      status_code = 409;
    }
    else if (errno == EACCES) {
      status_code = 403;
    }
    else {
      status_code = 500;
    }
    return false;
  }

  if (!S_ISDIR(st.st_mode)) {
    status_code = 409;
    return false;
  }

  if (access(parent, W_OK | X_OK) == -1) {
    status_code = 403;
    return false;
  }

  return true;
}

Result<bool> Response::handlePost(const Request& req, const Config_Server& serv) {
  if (!check_post_parent_dir(_filepath, _status_code)) {
    return handleError(serv);
  }

  int access_stat = access(_filepath, F_OK);

  int fd = open(_filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd == -1) {
    _status_code = 500;
    return handleError(serv);
  }
  StrSlice body = req.getBody();

  int _written = write(fd, &body.at(0), body.getLen());
  if (_written != body.getLen()) {
    close(fd);
    _status_code = 500;
    return handleError(serv);
  }
  if (access_stat == -1)
    _status_code = 201;
  else
    _status_code = 204;
  setHeader(serv);
  (void)req;
  bool ok = true;
  return Result<bool>(ok);
}

Result<bool> Response::handleDelete(const Request& req, const Config_Server& serv) {
  (void)req;
  bool ok = true;
  return Result<bool>(ok);
}


Result<bool> Response::handleError(const Config_Server& serv) {
  if (_body_fd != -1) {
    close(_body_fd);
    _body_fd = -1;
  }
  // std::memset(_filepath, 0, MAX_FILE_PATH);
  // std::strcpy(_filepath, _root_error);

  _content_type = PLAIN;
  _content_len = -1;
  if (!setHeader(serv))
    return Result<bool>("Cannot set error header just send backup error");
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

  if (_body_fd == -1 && !_has_mem_body) {
    return 0;
  }

  size_t remaining_body = (size_t)_content_len - _body_offset;
  size_t to_read = std::min(max_len, remaining_body);
  
  if (_has_mem_body) {
    if (to_read > 0) {
      std::memcpy(tmp_buffer, _mem_body + _body_offset, to_read);
      _body_offset += to_read;
    }
    return to_read;
  }

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
