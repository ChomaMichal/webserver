#pragma once
#include "../sockets/Sockets.hpp"
#include "../sockets/Stream.hpp"
#include "../utils/result/Result.hpp"
#include "Request.hpp"
#include "Response.hpp"

class Client : Networking {
public:
  ~Client();
  Client(Stream &obj);

  // returns false if there is no request true is request was susccesfuly
  // recieved and parsed error is set in result
  Result<bool> recieveRequest(void);
  Result<bool> sendResponse(void);
  Result<bool> setResponse(void); // mehras
  bool isResponseReady(void) const;
  bool isResponseFullySent(void) const;
  void close(void);
  Request &getRequest(void);
  Stream &getStream(void);
  // will only send at most  MAX_SEND_BUFFER bites
  void setSendBuffer(char *buff, size_t len);

private:
  Client();
  Client(const Client &obj);
  Client &operator=(const Client &obj);
  Stream _stream;
  Request _request;
  Response _response;
  bool _response_ready;
  size_t _response_sent_bytes;
};
