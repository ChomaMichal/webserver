#include <cstring>
#include <unistd.h>

bool build_path_no_string(const char* root, StrSlice uri, char* out, size_t cap, size_t& out_len, int& status) {
  out_len = 0;
  status = 200;

  // copy root
  size_t root_len = std::strlen(root);
  if (root_len + 1 >= cap) { status = 500; return false; }
  std::memcpy(out, root, root_len);
  out_len = root_len;

  // ensure one '/'
  if (out_len == 0 || out[out_len - 1] != '/') {
    if (out_len + 1 >= cap) { status = 500; return false; }
    out[out_len++] = '/';
  }

  // append URI (skip leading '/'), stop at query/fragment
  size_t i = 0;
  if (uri.getLen() > 0 && uri[0] == '/') i = 1;

  // default document for "/" or empty
  bool uri_empty = (i >= uri.getLen());
  if (uri_empty) {
    static const char kIndex[] = "index.html";
    size_t n = sizeof(kIndex) - 1;
    if (out_len + n + 1 >= cap) { status = 500; return false; }
    std::memcpy(out + out_len, kIndex, n);
    out_len += n;
  } else {
    for (; i < uri.getLen(); ++i) {
      char c = uri[i];
      if (c == '?' || c == '#') break;

      // simple traversal block
      if (c == '.' && i + 1 < uri.getLen() && uri[i + 1] == '.') {
        status = 403;
        return false;
      }

      if (out_len + 1 >= cap) { status = 414; return false; } // URI too long
      out[out_len++] = c;
    }
  }

  out[out_len] = '\0';

  // existence/readability check
  if (access(out, R_OK) != 0) {
    status = 404;
    return false;
  }
  return true;
}