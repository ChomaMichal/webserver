#pragma once

class Request {
public:
  Request(void);
  Request(Request &other);
  Request(int fd);
  Request &operator=(Request &other);
  ~Request();

private:
  int fd;
};
