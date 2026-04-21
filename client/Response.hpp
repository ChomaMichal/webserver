#pragma once
#include "fcntl.h"
#include "../utils/option/Option.hpp"
#include "../utils/str_slice/StrSlice.hpp"
#include "Request.hpp"
#include <cstddef>
#include <iostream>

class Response {
public:
 static Option<Response> setResponse(const Request& _reqeust); // mehras
private:
 StrSlice _root;

 StrSlice _httpVersion;
 StrSlice _code;
 StrSlice _reason;
 StrSlice _server;

 StrSlice _body;
};