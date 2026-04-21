#include "Client.hpp"

Client::Client() : _response_sent_bytes(0), _response_ready(false) {}
Client::~Client() {}

Client::Client(Stream &obj) : _stream(obj), _response_sent_bytes(0), _response_ready(false) {}

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

  _request = (*maybe);
  _response.reset();
  bool rt = true;
  return (Result<bool>(rt));
}

Result<bool> Client::setResponse(void) {
  _response.setBasicMessage(); // handle request #todo

  _response_sent_bytes = 0;
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


  char temp_buffer[MAX_SEND_BUFFER];
  size_t to_send = _response.chunker(temp_buffer, MAX_SEND_BUFFER);
  if (to_send == 0) {
    bool done = true;
    return Result<bool>(done);
  }

  std::cout.write(temp_buffer, to_send);
  std::cout << "\n"; // verbose
  setSendBuffer(temp_buffer, to_send);
  auto err = _stream.write();
  if (err.is_error()) {
    return (Result<bool>(err.get_error()));
  }

  if ((*err) == false) {
    return (Result<bool>(*err));
  }

  _response_sent_bytes += to_send;

  bool rt = true;
  return (Result<bool>(rt));
}

bool Client::isResponseFullySent(void) const {
  if (_response_ready == false) {
    return true;
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
