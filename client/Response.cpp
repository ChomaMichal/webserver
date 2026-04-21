#include "Response.hpp"

Option<int> ft_open(const std::string &filename) {
  int fd = open(filename.c_str(), O_RDWR);

  if (fd == -1) {
    Option<int> rt(false);
    return (rt);
  }
  Option<int> rt(fd);
  return rt;
}

Option<Response> Response::setResponse(const Request& req) {
  Response res;
  {
    res._httpVersion = req.getHTTPVersion(); // add set mehras
  }
  {
    res._code = "200";
  }
  return (Option<Response>(res));
}