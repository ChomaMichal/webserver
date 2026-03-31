#pragma once
#include "../utils/option/Option.hpp"
#include "../utils/str_slice/StrSlice.hpp"
#include <cstddef>
#include <iostream>

typedef enum method { NOT_SET, ERROR, GET, POST, DELETE } e_method;
typedef enum connection { NONE, CLOSE, KEEP_ALIVE } e_connection;

class Request {
public:
  Request();
  ~Request();
  Request(const Request &obj);
  Request &operator=(const Request &obj);

  static Option<Request> parse(char *buffer);

  e_method getMethod(void) const;
  StrSlice getRequestURI(void) const;
  StrSlice getHTTPVersion(void) const;
  ssize_t getLen(void) const;
  StrSlice getHost(void) const;
  e_connection getConnection(void) const;
  StrSlice getContentType(void) const;
  StrSlice getBody(void) const;

  void setMethod(e_method method);
  void setRequestURI(StrSlice requestURI);
  void setHTTPVersion(StrSlice httpVersion);
  void setLen(size_t len);
  void setHost(StrSlice host);
  void setConnection(e_connection connection);
  void setContentType(StrSlice contentType);
  void setBody(StrSlice body);

private:
  e_method _method;
  StrSlice _requestURI;
  StrSlice _httpVersion;
  ssize_t _len;
  StrSlice _host;
  e_connection _connection;
  StrSlice _contentType;
  StrSlice _body;
};

std::ostream &operator<<(std::ostream &os, const Request &req);
