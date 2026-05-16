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

Result<bool> Client::setResponse(const Config_Server& in) {
  // std::cout << "Client :: 44 URI = " << _request.getRequestURI() << std::endl;
  auto request_res = _response.handleRequest(_request, in); // handle request #todo
  if (request_res.is_error()) {
    return Result<bool>(request_res.get_error());
  }

  _response_ready = true;
  bool ret = true;
  return (Result<bool>(ret));
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

  memset(_send_buffer, 0, MAX_SEND_BUFFER);
  ssize_t to_send = _response.chunker(_send_buffer, MAX_SEND_BUFFER);

  static int i = 0;
  std::cout << "client :: 67 :: to_send = " << to_send << std::endl;
  std::cout << "client :: 68 :: send_buffer = " << _send_buffer << std::endl;
  i++;
  if (i == 10)
    return Result<bool>("Over");
  if (to_send == -1)  
    return Result<bool>("CGI failed");
  if (to_send == 0) {
    bool done = true;
    return Result<bool>(done);
  }

  // write the chunk into the shared networking send buffer and set length
  _send_buffer_len = to_send;
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

bool Client::isResponseFullySent(void) {
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
