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
    
    ft_memcpy(_header, in._header, MAX_HEADER_SIZE);
    
    ft_memcpy(_filepath, in._filepath, MAX_FILE_PATH);
    _status_code = in._status_code;
    _content_type = in._content_type;
    _has_location = in._has_location;
    _content_len = in._content_len;
    _body_fd = in._body_fd;
    _body_offset = in._body_offset;
    _has_mem_body = in._has_mem_body;
    _uri_index = in._uri_index;
    _location = in._location;
    _root = in._root;
    ft_memcpy(_mem_body, in._mem_body, MAX_HEADER_SIZE);
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
  
  ft_memset(_mem_body, 0, MAX_HEADER_SIZE);
  ft_memset(_header, 0, MAX_HEADER_SIZE);

  ft_memset(_filepath, 0, MAX_FILE_PATH);
  _status_code = 200;
  _content_type = OTHER;
  _has_location = 0;
  _has_content_type = 0;
  _has_content_length = 0;
  _content_len = 0;
  _body_fd = -1;
  _body_offset = 0;
  _root = NULL;
  _error = NULL;
  _location = NULL;
  _uri_index = 0;
}

void Response::setFilePath(const Request& req) {
  const char * root = _root;
  StrSlice uri = req.getRequestURI();
  char full_path[MAX_FILE_PATH];
  size_t out_len = 0;
  size_t root_len = ft_strlen(root);

  ft_memcpy(full_path, root, root_len);
  out_len = root_len;

  if (out_len > 0 && full_path[out_len - 1] != '/') {
    full_path[out_len] = '/';
    out_len++;
  }

  size_t i = _uri_index;
  // std::cout << "response :: 81 :: _uri_index = " << _uri_index << std::endl;

  for (; i < uri.getLen(); ++i) {
    if (out_len + 1 >= MAX_FILE_PATH) {
      ft_memcpy(_filepath, "", 1);
      return ;
    }
    if (uri[i] == '/' && out_len > 0 && full_path[out_len - 1] == '/') {
      continue;
    }
    // std::cout << uri[i] << std::endl;
    full_path[out_len] = uri[i];
    out_len++;
  }

  full_path[out_len] = 0;
  ft_memcpy(_filepath, full_path, out_len + 1);
}

void Response::setContentType() {
  const char *ext = ft_strrchr(_filepath, '.');

  if (ext == NULL || *(ext + 1) == 0) {
    return;
  }

  switch (*(ext + 1)) {
    case 'b':
      if (ft_strcmp(ext, ".bin") == 0) {
        _content_type = BIN;
      }
      break;
    case 'h':
      if (ft_strcmp(ext, ".html") == 0 || ft_strcmp(ext, ".htm") == 0) {
        _content_type = HTML;
      }
      break;
    case 'c':
      if (ft_strcmp(ext, ".css") == 0) {
        _content_type = CSS;
      }
      break;
    case 'j':
      if (ft_strcmp(ext, ".js") == 0) {
        _content_type = JAVASCRIPT;
      }
      else if (ft_strcmp(ext, ".json") == 0) {
        _content_type = JSON;
      }
      else if (ft_strcmp(ext, ".jpeg") == 0 || ft_strcmp(ext, ".jpg") == 0) {
        _content_type = JPEG;
      }
      break;
    case 'p':
      if (ft_strcmp(ext, ".png") == 0) {
        _content_type = PNG;
      }
      break;
    case 'i':
      if (ft_strcmp(ext, ".ico") == 0) {
        _content_type = XICON;
      }
      break;
    case 't':
      if (ft_strcmp(ext, ".txt") == 0) {
        _content_type = PLAIN;
      }
      break;
    default:
      _content_type = OTHER;
      break;
  }
}

const char * Response::matchRouteToRoot(const Request& req, const std::vector<Config_Route>& routes) {
  _uri_index = 0;
  for (auto idx = routes.begin(); idx != routes.end(); idx++) {
    const std::string& loc = idx->getLocation();
    const StrSlice& uri = req.getRequestURI();

    if (uri.getLen() >= loc.size()) {
      bool is_prefix = true;
      for (size_t i = 0; i < loc.size(); ++i) {
        if (uri[i] != loc[i]) {
          is_prefix = false;
          break;
        }
      }
      if (is_prefix) {
        if (loc.size() == 0 || loc[loc.size() - 1] == '/' || uri.getLen() == loc.size() || uri[loc.size()] == '/') {
          if (idx->getRootChanged()) {
            _uri_index = loc.size();
            if (_uri_index < uri.getLen() && uri[_uri_index] == '/') {
              if (loc.empty() || loc.back() != '/') {
                _uri_index++;
              }
            }
          }
          _matched_route = &(*idx);
          return idx->getRoot().c_str();
        }
      }
    }
  }

  return (NULL);
}

Result<bool> Response::handleRequest(const Request& req, const Config_Server& serv) {
  if (serv.getRedirectionSet()) {
    _status_code = serv.getRedirection().first;
    _location = serv.getRedirection().second.c_str();
    return handleRedirect();
  }
  const std::vector<Config_Route>& routes = serv.getRoutes();
  _root = matchRouteToRoot(req, routes);
  if (!_root || _root[0] == '\0') {
    _root = serv.getRoot().c_str();
    _uri_index = 0;
  }
  else if (_root && _matched_route) {
    _status_code = serv.getRedirection().first;
    _location = serv.getRedirection().second.c_str();
    return handleRedirect();
  }
  // std::cout << "response :: 186 :: root = " << _root << std::endl;
  setFilePath(req);
  // std::cout << "response :: 188 :: filepath = " << _filepath << std::endl;
  
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

Result<bool> Response::handleRedirect() {
  // TODO
  bool ok = true;
  return Result<bool>(ok);
}

bool Response::fileStatRead(struct stat& _file_stat, const Request &req, const Config_Server& serv) {
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
    ft_strncpy(index_path, _filepath, MAX_FILE_PATH - 1);
    index_path[MAX_FILE_PATH - 1] = 0;

    size_t len = ft_strlen(index_path);
    if (len > 0 && index_path[len - 1] != '/') {
      if (len < MAX_FILE_PATH - 1) {
        index_path[len] = '/';
        index_path[len + 1] = 0;
      }
    }
    
    // std::cout << "response :: 224 :: index_path = " << index_path << std::endl;
    char const *index_file = "index.html";
    if (ft_strlen(index_path) + ft_strlen(index_file) < MAX_FILE_PATH) {
      ft_strcat(index_path, index_file);
      struct stat idx_stat;
      if (stat(index_path, &idx_stat) == 0 && S_ISREG(idx_stat.st_mode) && access(index_path, R_OK) == 0) {
        ft_strcpy(_filepath, index_path);
        _file_stat = idx_stat;
        // std::cout << "response :: 232 :: index_path = " << index_path << std::endl;
      } else if (serv.getAutoIndex()) {
        return generateDirectoryIndex(_filepath, req);
      } else {
        return (_status_code = 403, false);
      }
    } else {
      return (_status_code = 500, false);
    }
  }
  // std::cout << "response :: 241 :: filepath = " << _filepath << std::endl;
  if (S_ISLNK(_file_stat.st_mode) || !S_ISREG(_file_stat.st_mode)) {
    return (_status_code = 403, false);
  }
  // std::cout << "response :: 245 :: filepath = " << _filepath << std::endl;
  if (access(_filepath, R_OK) == -1) {
    return (_status_code = 403, false);
  }
  setContentType();
  // std::cout << "response :: 250 :: filepath = " << _filepath << std::endl;
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

  size_t dest_len = ft_strlen(dest);
  if (dest_len >= dest_max_len) {
    return false;
  }

  size_t space_left = dest_max_len - dest_len - 1;
  if (n > space_left) {
    return false;
  }

  ft_memcpy(dest + dest_len, src, n);
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
  ft_memcpy(uri_path, &req_uri.at(0), copy_len);
  uri_path[copy_len] = 0;
  
  if (copy_len > 0 && uri_path[copy_len - 1] != '/') {
    uri_path[copy_len] = '/';
    uri_path[copy_len + 1] = 0;
  }

  _mem_body[0] = 0;
  size_t max_len = sizeof(_mem_body);

  memcat(_mem_body, "<html><head><title>Index of ", ft_strlen("<html><head><title>Index of "), max_len);
  memcat(_mem_body, uri_path, ft_strlen(uri_path), max_len);
  memcat(_mem_body, "</title></head><body><h1>Index of ", ft_strlen("</title></head><body><h1>Index of "), max_len);
  memcat(_mem_body, uri_path, ft_strlen(uri_path), max_len);
  memcat(_mem_body, "</h1><hr><ul>\n", ft_strlen("</h1><hr><ul>\n"), max_len);

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (ft_strcmp(entry->d_name, ".") == 0) {
      continue;
    }
    char path_buf[MAX_FILE_PATH] = {};
    memcat(path_buf, dir_path, ft_strlen(dir_path), sizeof(path_buf));
    memcat(path_buf, "/", 1, sizeof(path_buf));
    memcat(path_buf, entry->d_name, ft_strlen(entry->d_name), sizeof(path_buf));
    
    struct stat s_st;
    bool is_dir = true;
    if (stat(path_buf, &s_st) == 0 && !S_ISDIR(s_st.st_mode)) {
      is_dir = false;
    }

    char target[MAX_FILE_PATH];
    target[0] = 0;
    memcat(target, uri_path, ft_strlen(uri_path), sizeof(target));
    memcat(target, entry->d_name, ft_strlen(entry->d_name), sizeof(target));

    char link[2048];
    link[0] = 0;
    size_t link_max = sizeof(link);
    
    memcat(link, "<li><a href=\"", 13, link_max);
    memcat(link, target, ft_strlen(target), link_max);
    if (is_dir) {
      memcat(link, "/", 1, link_max);
    }
    memcat(link, "\">", 2, link_max);
    memcat(link, entry->d_name, ft_strlen(entry->d_name), link_max);
    if (is_dir) {
      memcat(link, "/", 1, link_max);
    }
    memcat(link, "</a></li>\n", 10, link_max);

    memcat(_mem_body, link, ft_strlen(link), max_len);
  }
  closedir(dir);

  if (!memcat(_mem_body, "</ul><hr></body></html>\n", ft_strlen("</ul><hr></body></html>\n"), max_len)) {
    _status_code = 413;
    return false;
  }
  _content_len = ft_strlen(_mem_body);
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
  ft_memset(_header, 0, MAX_HEADER_SIZE);
  _head_offset = 0;
  _header_sent = false;
  ft_memcpy(_header, _http_version, ft_strlen(_http_version) + 1);

  const char code[5] = {(const char)(_status_code / 100 + '0'),
              (const char)(_status_code / 10 % 10 + '0'),
              (const char)(_status_code % 10 + '0'), ' ', 0};

  if (!memcat(_header, code, 4, MAX_HEADER_SIZE))
    return false;

  const char *reason = find_code_reason(_status_code);
  if (!memcat(_header, reason, ft_strlen(reason), MAX_HEADER_SIZE))
    return false;

  if (!memcat(_header, "\r\n", 2, MAX_HEADER_SIZE))
    return false;

  if (!memcat(_header, _server, ft_strlen(_server), MAX_HEADER_SIZE))
    return false;
  if (!memcat(_header, "\r\n", 2, MAX_HEADER_SIZE))
    return false;

  if (!memcat(_header, _header_content_type, ft_strlen(_header_content_type), MAX_HEADER_SIZE))
    return false;
  const char *content_type = find_content_type(_content_type);
  if (!memcat(_header, content_type, ft_strlen(content_type), MAX_HEADER_SIZE))
    return false;

  if (!memcat(_header, "\r\n", 2, MAX_HEADER_SIZE))
    return false;

  if (!memcat(_header, _header_content_length, ft_strlen(_header_content_length), MAX_HEADER_SIZE))
    return false;
  if (!numcat(_header, _content_len, MAX_HEADER_SIZE))
    return false;
  if (!memcat(_header, "\r\n", 2, MAX_HEADER_SIZE))
    return false;
  
  if (!memcat(_header, _header_connection_close, ft_strlen(_header_connection_close), MAX_HEADER_SIZE))
    return false;

  if (!memcat(_header, "\r\n\r\n", 4, MAX_HEADER_SIZE))
    return false;

  _header_size = ft_strlen(_header);

  return true;
}

Result<bool> Response::handleGet(const Request& req, const Config_Server& serv) {
  struct stat _file_stat;
  if (!fileStatRead(_file_stat, req, serv))
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
  const char *last_slash = ft_strrchr(filepath, '/');

  if (last_slash == NULL) {
    ft_memmove(parent, ".", 1);
  }
  else if (last_slash == filepath) {
    ft_memmove(parent, "/", 1);
  }
  else {
    int to_copy = last_slash - filepath;
    ft_strncpy(parent, filepath, to_copy);
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
  if (!serv.getUploadAllowed() || ft_strncmp(_filepath, serv.getUploadLocation().c_str(), serv.getUploadLocation().size()) != 0) {
    _status_code = 403;
    return handleError(serv);
  }

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

  size_t _written = write(fd, &body.at(0), body.getLen());
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
  if (!serv.getUploadAllowed() || ft_strncmp(_filepath, serv.getUploadLocation().c_str(), serv.getUploadLocation().size()) != 0) {
    _status_code = 403;
    return handleError(serv);
  }

  if (access(_filepath, F_OK) == -1) {
    _status_code = 404;
    return handleError(serv);
  }
  
  if (access(_filepath, W_OK) == -1) {
    _status_code = 403;
    return handleError(serv);
  }

  if (std::remove(_filepath) != 0) {
    _status_code = 500;
    return handleError(serv);
  }

  _status_code = 204;
  setHeader(serv);
  (void)req;
  bool ok = true;
  return Result<bool>(ok);
}


Result<bool> Response::handleError(const Config_Server& serv) { //TODO
  if (_body_fd != -1) {
    close(_body_fd);
    _body_fd = -1;
  }
  // ft_memset(_filepath, 0, MAX_FILE_PATH);
  // ft_strcpy(_filepath, _root_error);

  _content_type = PLAIN;
  _content_len = 0;
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
      ft_memcpy(tmp_buffer, _header, to_copy);
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
      ft_memcpy(tmp_buffer, _mem_body + _body_offset, to_read);
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
  return _header_sent && (_content_len == 0 || _body_offset >= _content_len);
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
