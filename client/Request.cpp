#include "Request.hpp"
#include <cctype>
#include <cstring>
#include <string.h>
#include <strings.h>
#include <iostream>
Request::Request() : _method(NOT_SET), _len(-1), _connection(NONE) {}
Request::~Request() {}

Request::Request(const Request &obj) { *this = obj; }

Request &Request::operator=(const Request &obj) {
  std::memcpy((void *)this, &obj, sizeof(Request));
  return (*this);
}

e_method Request::getMethod(void) const { return _method; }

StrSlice Request::getRequestURI(void) const { return _requestURI; }

StrSlice Request::getHTTPVersion(void) const { return _httpVersion; }

size_t Request::getLen(void) const { return _len; }

StrSlice Request::getHost(void) const { return _host; }

e_connection Request::getConnection(void) const { return _connection; }

StrSlice Request::getContentType(void) const { return _contentType; }

StrSlice Request::getBody(void) const { return _body; }
void Request::setMethod(e_method method) { _method = method; }

void Request::setRequestURI(StrSlice requestURI) { _requestURI = requestURI; }

void Request::setHTTPVersion(StrSlice httpVersion) {
  _httpVersion = httpVersion;
}

void Request::setLen(size_t len) { _len = len; }

void Request::setHost(StrSlice host) { _host = host; }

void Request::setConnection(e_connection connection) {
  _connection = connection;
}

void Request::setContentType(StrSlice contentType) {
  _contentType = contentType;
}

void Request::setBody(StrSlice body) { _body = body; }

static Option<e_method> parseMethod(char *buffer) {
  if (std::strncmp("GET ", buffer, 4) == 0) {
    buffer += 4;
    return (Option<e_method>(GET));
  }

  if (std::strncmp("DELETE ", buffer, 7) == 0) {
    buffer += 7;
    return (Option<e_method>(DELETE));
  }

  if (std::strncmp("POST ", buffer, 5) == 0) {
    buffer += 5;
    return (Option<e_method>(POST));
  }

  return (Option<e_method>(true));
}

static Option<StrSlice> parseURI(char *buffer) {
  for (size_t i = 0; std::strncmp(buffer, "\r\n", 2); i++) {
    if (buffer[i] == ' ') {
      StrSlice rt(buffer, i - 1);
      buffer += i;
      return (Option<StrSlice>(rt));
    }
  }
  return (Option<StrSlice>(false));
}

static Option<StrSlice> parseHttpVersion(char *buffer) {
  if (std::strncmp("HTTP/", buffer, 5) != 0) {
    return (Option<StrSlice>(false));
  }
  if (!std::isdigit(buffer[6])) {
    return (Option<StrSlice>(false));
  }
  if (buffer[7] == '.') {
    return (Option<StrSlice>(false));
  }
  if (!std::isdigit(buffer[8])) {
    return (Option<StrSlice>(false));
  }
  if (std::strncmp(buffer + 9, "\r\n", 2) != 0) {
    return (Option<StrSlice>(false));
  }
  StrSlice rt(buffer, 8);
  return (Option<StrSlice>(rt));
}

static Option<ssize_t> parseLen(char *buffer) {
  char *ptr = strcasestr(buffer, "\r\nContent-Length:");
  if (ptr == NULL) {
    return (Option<ssize_t>(false));
  };
  char *lbuffer = ptr + 17;
  for (; *lbuffer == ' ' || *lbuffer == '\t'; lbuffer++)
    ;
  char *end;
  size_t len = std::strtoul(lbuffer, &end, 10);
  if (std::strncmp(end, "\r\n", 2) != 0) {
    return (Option<ssize_t>(false));
  }
  return (Option<ssize_t>((ssize_t)len));
}

static Option<StrSlice> parseHost(char *buffer) {
  char *ptr = strcasestr(buffer, "\r\nContent-Length:");
  if (ptr == NULL) {
    return (Option<StrSlice>(false));
  };
  for (; *ptr == ' ' || *ptr == '\t'; ptr++)
    ;
  StrSlice rt(ptr, strcasestr(ptr, "\r\n") - ptr);
  return (Option<StrSlice>(rt));
}

static Option<e_connection> parseConnection(char *buffer) {
  char *ptr = strcasestr(buffer, "\r\n:Connection");
  if (ptr == NULL) {
    return (Option<e_connection>(false));
  };
  for (; *ptr == ' ' || *ptr == '\t'; ptr++)
    ;
  if (strncasecmp(ptr, "close\r\n", 7) == 0) {
    return (Option<e_connection>(CLOSE));
  }
  if (strncasecmp(ptr, "keep-alive\r\n", 12) == 0) {
    return (Option<e_connection>(KEEP_ALIVE));
  }
  return (Option<e_connection>(NONE));
}

static Option<StrSlice> parseContentType(char *buffer) {
  char *ptr = strcasestr(buffer, "\r\nContent-Type:");
  if (ptr == NULL) {
    return (Option<StrSlice>(false));
  };
  for (; *ptr == ' ' || *ptr == '\t'; ptr++)
    ;
  StrSlice rt(ptr, std::strstr(ptr, "\r\n") - ptr);
  return (rt);
}

Option<Request> Request::parse(char *buffer) {
  Request req;
  {
    auto maybe = parseMethod(buffer);
    if (maybe.is_none())
      return (Option<Request>(true));
    req.setMethod(maybe.unwrap());
  }
  {
    auto maybe = parseURI(buffer);
    if (maybe.is_none())
      return (Option<Request>(true));
    req.setRequestURI(maybe.unwrap());
  }
  {
    auto maybe = parseHttpVersion(buffer);
    if (maybe.is_none())
      return (Option<Request>(true));
    req.setRequestURI(maybe.unwrap());
  }
  int there_is_header = 0;
  {
    auto maybe = parseLen(buffer);
    if (maybe.is_some()) {
      req.setLen(maybe.unwrap());
      there_is_header = 1;
    } else {
      req.setLen(-1);
    }
  }
  {
    auto maybe = parseHost(buffer);
    if (maybe.is_some()) {
      req.setHost(maybe.unwrap());
      there_is_header = 1;
    }
  }
  {
    auto maybe = parseConnection(buffer);
    if (maybe.is_some()) {
      req.setConnection(maybe.unwrap());
      there_is_header = 1;
    }
  }
  {
    auto maybe = parseContentType(buffer);
    if (maybe.is_some()) {
      req.setContentType(maybe.unwrap());
      there_is_header = 1;
    }
  }
  if (req._len != -1) {
    StrSlice body(std::strstr(buffer, "\r\n\r\n"), req._len);
    req.setBody(body);
  }
  return (Option<Request>(req));
}

std::ostream &operator<<(std::ostream &os, const Request &req) {
  static const char *methodStrings[] = {"NOT_SET", "ERROR", "GET", "POST", "DELETE"};
  static const char *connectionStrings[] = {"NONE", "CLOSE", "KEEP_ALIVE"};

  os << "Request {\n";
  os << "  Method: " << methodStrings[req.getMethod()] << "\n";
  os << "  RequestURI: " << req.getRequestURI() << "\n";
  os << "  HTTPVersion: " << req.getHTTPVersion() << "\n";
  os << "  ContentLength: " << req.getLen() << "\n";
  os << "  Host: " << req.getHost() << "\n";
  os << "  Connection: " << connectionStrings[req.getConnection()] << "\n";
  os << "  ContentType: " << req.getContentType() << "\n";
  os << "  Body: " << req.getBody() << "\n";
  os << "}";
  return os;
}
