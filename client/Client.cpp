#include "Client.hpp"
#include <cstring>

Client::Client() :  _response_ready(false) {}
Client::Client(const Client& in ) {
  *this = in;
}

// Client& Client::operator=(const Client& in) {

// }
Client::~Client() {}

Client::Client(Stream &obj) : _stream(obj),  _response_ready(false) {}

Request &Client::getRequest(void) { return (_request); }

void Client::close() { _stream.close(); }

Result<bool> Client::recieveRequest(void) {
  auto err = _stream.read();
  if (err.is_error()) {
    return (Result<bool>(err.get_error()));
  }

  if ((*err) == false) {
    return (Result<bool>(*err));
  }

  //_stream.printBuffer(); why
  auto maybe = Request::parse(_stream.getBuffer());
  if (maybe.is_none()) {
    return (Result<bool>("Error in parsing request"));
  }

  _response_ready = false;
  _request = (*maybe);
  _response.reset();
  bool rt = true;
  return (Result<bool>(rt));
}

Result<bool> Client::setResponse(void) {
  // std::cout << "Client :: 44 URI = " << _request.getRequestURI() << std::endl;
  setFilePath();
  auto request_res = _response.handleRequest(_request); // handle request #todo
  if (request_res.is_error()) {
    return Result<bool>(request_res.get_error());
  }

  _response_ready = true;
  bool ret = true;
  return (Result<bool>(ret));
}

void Client::setFilePath() {
  const char * root = _root;
  if (_request.getMethod() != GET)
    root = _root_tmp;
  StrSlice uri = _request.getRequestURI();
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


  if (i >= uri.getLen() && _request.getMethod() == GET) { // only "/" or "" in URI
    const char *index_file = "index.html";
    size_t n = std::strlen(index_file);
    if (out_len + n >= MAX_FILE_PATH) {
      _response.setFilePath("");
      return ;
    }
    std::memcpy(full_path + out_len, index_file, n);
    out_len += n;
  }
  else {
    for (; i < uri.getLen(); ++i) {
      if (out_len + 1 >= MAX_FILE_PATH) {
        _response.setFilePath("");
        return ;
      }
      // std::cout << uri[i] << std::endl;
      full_path[out_len] = uri[i];
      out_len++;
    }
  }

  full_path[out_len] = 0;
  // std::cout << "Client :: 120 full path = " << full_path << std::endl;
  _response.setFilePath(full_path);
}

Result<bool> Client::sendResponse(void) {
  short fd_status = _stream.getFdStatus();
  if ((fd_status & (POLLERR | POLLHUP)) != 0) {
    return Result<bool>("fd error");
  }
  if ((fd_status & POLLOUT) == 0) {
    bool not_ready = false;
    return Result<bool>(not_ready);
  }

  char temp_buffer[MAX_SEND_BUFFER] = {};
  size_t to_send = _response.chunker(temp_buffer, MAX_SEND_BUFFER);
  if (to_send == 0) {
    bool done = true;
    return Result<bool>(done);
  }

  setSendBuffer(temp_buffer, to_send);
  // std::cout << temp_buffer << std::endl;
  auto err = _stream.write();
  if (err.is_error()) {
    return (Result<bool>(err.get_error()));
  }
  if ((*err) == false) {
    return (Result<bool>(*err));
  }

  bool rt = true;
  return (Result<bool>(rt));
}

bool Client::isResponseFullySent(void) const {
  if (_response_ready == false) {
    return false;
  }
  return _response.isFullySent();
}

bool Client::isResponseReady(void) const {
  return _response_ready;
}

void Client::setSendBuffer(char *buff, size_t len) {
  std::memcpy(_send_buffer, buff, len);
  _send_buffer_len = len;
}

Stream &Client::getStream(void) { return (_stream); }
