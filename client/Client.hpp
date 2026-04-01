#pragma once
#include "../sockets/Sockets.hpp"
#include "../sockets/Stream.hpp"
#include "../utils/result/Result.hpp"
#include "Request.hpp"

class Client {
public:
  ~Client();
  Client(const Client &obj);
  Client &operator=(const Client &obj);
  Client(Stream &obj);

  // returns false if there is no request true is request was susccesfuly
  // recieved and parsed error is set in result
  Result<bool> recieveRequest(void);
  void close(void);
  Request &getRequest(void);

private:
  Client();
  Stream _stream;
  Request _request;
};
