#pragma once
#include "../sockets/Sockets.hpp"
#include "Request.hpp"

class Client {
public:
  Client();
  ~Client();
  Client(const Client &obj);
  Client &operator=(const Client &obj);

private:
  Stream _stream;
  Request _request;
};
