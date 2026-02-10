#include "Client.hpp"

Client::Client() {}
Client::~Client() {}
Client::Client(const Client &obj) { *this = obj; }
// doesn't do a deep copy because of lack of move semantics and safety
Client &Client::operator=(const Client &obj) {
  std::memcpy((void *)this, (void *)&obj, sizeof(Client));
  return (*this);
};

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

  auto maybe = Request::parse(_stream.getBuffer());
  if (maybe.is_none()) {
    return (Result<bool>("Error in parsing request"));
  }

  _request = (*maybe);
  bool rt = true;
  return (Result<bool>(rt));
}
