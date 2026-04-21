# HTTP Response Implementation - Workflow & TODOs

## Overview
Implement HTTP response creation and sending through the client. Response is serialized into chunks and sent via `_send_buffer` repeatedly until complete.

---

## Phase 1: Enhance Response Class

### TODO 1.1: Add new private members to Response.hpp
Track serialization state:
```cpp
private:
  size_t _body_offset;      // How many bytes of body already sent
  size_t _total_size;       // Total response size (headers + body)
  bool _headers_sent;       // Whether headers have been serialized yet
  int _status_code;         // HTTP status: 200, 404, 500, etc.
  StrSlice _content_type;   // e.g., "text/html", "text/plain"
  // File descriptors or buffer for body reading
```

### TODO 1.2: Implement `setResponseWithFile()` in Response.cpp
- **Input**: Request object + root directory path
- **Logic**:
  1. Parse request path from `_request` 
  2. Try to open file from `root/` + request path
  3. If file exists & readable → set `_status_code = 200`, read file
  4. If file not found → set `_status_code = 404`, set error body
  5. If read error → set `_status_code = 500`, set error body
  6. Calculate `Content-Length` from file size + headers
  7. Set appropriate `Content-Type` based on file extension

**Example:**
```cpp
Result<void> Response::setResponseWithFile(const Request& req, const std::string& root_path) {
  // Pseudo-code
  std::string full_path = root_path + req.getPath();
  int fd = open(full_path.c_str(), O_RDONLY);
  
  if (fd == -1) {
    _status_code = 404;
    _body = StrSlice("404 Not Found");
  } else {
    _status_code = 200;
    _body = read_file_to_strslice(fd);
    close(fd);
  }
  
  _total_size = calculate_header_size() + _body.len();
  _body_offset = 0;
  _headers_sent = false;
  return Result<void>();
}
```

### TODO 1.3: Implement `serialize()` in Response.cpp
- **Input**: char buffer, max_len
- **Output**: number of bytes written
- **Logic**:
  1. If headers not sent yet → serialize headers first:
     - `HTTP/1.1 200 OK\r\n`
     - `Content-Type: text/html\r\n`
     - `Content-Length: <size>\r\n`
     - `\r\n`
  2. Once headers done → serialize body in chunks
  3. Auto-increment `_body_offset`
  4. Return bytes written

**Example:**
```cpp
size_t Response::serialize(char* buffer, size_t max_len) {
  if (!_headers_sent) {
    std::string headers = format_headers();
    size_t to_copy = std::min(headers.size(), max_len);
    std::memcpy(buffer, headers.c_str(), to_copy);
    _headers_sent = true;
    return to_copy;
  }
  
  // Send body in chunks
  size_t remaining = _body.len() - _body_offset;
  size_t to_copy = std::min(remaining, max_len);
  std::memcpy(buffer, _body.data() + _body_offset, to_copy);
  _body_offset += to_copy;
  return to_copy;
}
```

### TODO 1.4: Add helper methods to Response
```cpp
public:
  bool isFullySent() const;        // Returns true if all bytes serialized
  void reset();                    // Reset for next response
  StrSlice getStatusReason();      // "200 OK", "404 Not Found", etc.
  void setContentType(const char* type);  // e.g., "text/html"
```

### TODO 1.5: Add request dispatch method to Response.hpp
```cpp
public:
  // Main entry point - dispatches to appropriate HTTP method handler
  Result<void> handleRequest(const Request& req, const std::string& root_path);

private:
  // HTTP method handlers
  Result<void> handleGET(const Request& req, const std::string& root_path);
  Result<void> handlePOST(const Request& req, const std::string& root_path);
  Result<void> handleDELETE(const Request& req, const std::string& root_path);
```

### TODO 1.6: Implement `handleRequest()` in Response.cpp
Dispatcher that routes to correct handler based on HTTP method:

```cpp
Result<void> Response::handleRequest(const Request& req, const std::string& root_path) {
  StrSlice method = req.getMethod();  // Returns "GET", "POST", or "DELETE"
  
  if (method == "GET") {
    return handleGET(req, root_path);
  } 
  else if (method == "POST") {
    return handlePOST(req, root_path);
  }
  else if (method == "DELETE") {
    return handleDELETE(req, root_path);
  }
  else {
    _status_code = 405;  // Method Not Allowed
    _body = StrSlice("405 Method Not Allowed");
    _total_size = _body.len() + header_size;
    return Result<void>("Unsupported HTTP method");
  }
}
```

### TODO 1.7: Implement `handleGET()` in Response.cpp
Handle GET requests - retrieve and return file content:

```cpp
Result<void> Response::handleGET(const Request& req, const std::string& root_path) {
  // Build full file path
  std::string file_path = root_path + req.getPath();
  // e.g. "./root" + "/index.html" = "./root/index.html"
  
  // Try to open file
  int fd = open(file_path.c_str(), O_RDONLY);
  
  if (fd == -1) {
    // File not found
    _status_code = 404;
    _body = StrSlice("404 Not Found");
    _content_type = StrSlice("text/plain");
    _total_size = _body.len() + calculate_header_size();
    return Result<void>();  // Not an error, valid response
  }
  
  // File found - read it
  // TODO: Implement file reading (may need to store fd for streaming)
  _status_code = 200;
  _body = read_file_to_strslice(fd);  // Helper to read file
  _content_type = determine_content_type(file_path);  // e.g., "text/html"
  _total_size = _body.len() + calculate_header_size();
  
  close(fd);
  return Result<void>();
}
```

**Status codes:**
- `200 OK` - file found and readable
- `404 Not Found` - file does not exist
- `500 Internal Server Error` - file exists but cannot read

### TODO 1.8: Implement `handlePOST()` in Response.cpp
Handle POST requests - write/create file with request body:

```cpp
Result<void> Response::handlePOST(const Request& req, const std::string& root_path) {
  // Build target file path
  std::string file_path = root_path + req.getPath();
  
  // Get request body data
  StrSlice body = req.getBody();
  
  // Try to create/write file
  int fd = open(file_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
  
  if (fd == -1) {
    _status_code = 500;
    _body = StrSlice("500 Internal Server Error");
    _content_type = StrSlice("text/plain");
    _total_size = _body.len() + calculate_header_size();
    return Result<void>("Failed to create/write file");
  }
  
  // Write body to file
  ssize_t written = write(fd, body.data(), body.len());
  close(fd);
  
  if (written != (ssize_t)body.len()) {
    _status_code = 500;
    _body = StrSlice("500 Internal Server Error");
    _total_size = _body.len() + calculate_header_size();
    return Result<void>("Failed to write body to file");
  }
  
  // Success - return 201 Created or 200 OK
  _status_code = 201;  // 201 Created
  _body = StrSlice("");  // No body for success
  _total_size = calculate_header_size();
  
  return Result<void>();
}
```

**Status codes:**
- `201 Created` - file successfully written
- `500 Internal Server Error` - write failed

### TODO 1.9: Implement `handleDELETE()` in Response.cpp
Handle DELETE requests - remove file from disk:

```cpp
Result<void> Response::handleDELETE(const Request& req, const std::string& root_path) {
  // Build target file path
  std::string file_path = root_path + req.getPath();
  
  // Try to check if file exists
  if (access(file_path.c_str(), F_OK) != 0) {
    // File does not exist
    _status_code = 404;
    _body = StrSlice("404 Not Found");
    _content_type = StrSlice("text/plain");
    _total_size = _body.len() + calculate_header_size();
    return Result<void>();
  }
  
  // Try to delete file
  if (unlink(file_path.c_str()) != 0) {
    // Cannot delete (permission, etc)
    _status_code = 403;  // Forbidden
    _body = StrSlice("403 Forbidden");
    _content_type = StrSlice("text/plain");
    _total_size = _body.len() + calculate_header_size();
    return Result<void>("Cannot delete file");
  }
  
  // Success - return 204 No Content
  _status_code = 204;  // No Content
  _body = StrSlice("");  // No body
  _total_size = calculate_header_size();
  
  return Result<void>();
}
```

**Status codes:**
- `204 No Content` - file successfully deleted
- `404 Not Found` - file does not exist
- `403 Forbidden` - cannot delete (permission denied)

---

## Phase 2: Modify Client Class

### TODO 2.1: Add tracking member to Client.hpp
```cpp
private:
  size_t _response_sent_bytes;  // Total bytes sent so far
```

### TODO 2.2: Modify `sendResponse()` in Client.cpp
- **Current behavior**: Just calls `_stream.write()` and creates Response
- **New behavior**:
  1. If Response not created → call `_response.handleRequest(_request, "./root")`
  2. Call `_response.serialize()` to get next chunk
  3. Use `setSendBuffer()` to copy chunk to `_send_buffer`
  4. Call `_stream.write()` to actually send
  5. Update `_response_sent_bytes`
  6. Return error if any step fails

**Example flow:**
```cpp
Result<bool> Client::sendResponse(void) {
  // Step 1: Create response if needed
  if (_response_sent_bytes == 0) {
    auto err = _response.handleRequest(_request, "./root");
    if (err.is_error()) return Result<bool>(err.get_error());
  }
  
  // Step 2: Serialize next chunk
  char temp_buffer[MAX_SEND_BUFFER];
  size_t bytes = _response.serialize(temp_buffer, MAX_SEND_BUFFER);
  
  if (bytes == 0) {
    // Response fully sent
    return Result<bool>(true);
  }
  
  // Step 3: Copy to send buffer and send
  setSendBuffer(temp_buffer, bytes);
  auto err = _stream.write();
  
  _response_sent_bytes += bytes;
  return err;
}
```

### TODO 2.3: Add helper method to Client
```cpp
public:
  bool isResponseFullySent() const {
    return _response.isFullySent();
  }
```

---

## Phase 3: Main Client Implementation

### TODO 3.1: Update main_client.cpp
Implement response sending loop:

**Example:**
```cpp
#include "client/Client.hpp"

int main() {
  Stream stream; // Already connected
  Client client(stream);
  
  // Step 1: Receive and parse request
  auto recv_result = client.recieveRequest();
  if (recv_result.is_error()) {
    std::cerr << "Error receiving request\n";
    return 1;
  }
  
  // Step 2: Send response in chunks
  while (true) {
    auto send_result = client.sendResponse();
    
    if (send_result.is_error()) {
      std::cerr << "Error sending response\n";
      break;
    }
    
    if (client.isResponseFullySent()) {
      std::cout << "Response fully sent\n";
      break;
    }
  }
  
  client.close();
  return 0;
}
```

---

## Implementation Order

1. **First** → TODO 1.6, 1.7, 1.8, 1.9: HTTP method handlers (GET, POST, DELETE)
2. **Second** → TODO 1.2 & 1.3: Core serialize logic in Response
3. **Third** → TODO 1.1 & 1.4: Response member variables and helpers
4. **Fourth** → TODO 2.1 & 2.2: Modify Client to use Response serialize
5. **Fifth** → TODO 3.1: Update main_client loop

---

## Key Design Points

| Aspect | Decision |
|--------|----------|
| **Serialization** | Happens in `Response::serialize()` - Client just calls it |
| **HTTP Methods** | GET/POST/DELETE handled via `Response::handleRequest()` dispatcher |
| **GET requests** | Read file from `root/` directory, return 200/404/500 |
| **POST requests** | Write request body to file, return 201/500 |
| **DELETE requests** | Remove file from disk, return 204/404/403 |
| **Large files** | Sent in MAX_SEND_BUFFER chunks via loop |
| **Headers vs Body** | Headers sent first, tracked with `_headers_sent` flag |
| **Status codes** | Response determines them based on operation success |
| **Memory** | No full file loading - stream body directly from file |
| **StrSlice** | Used for headers and small strings; body might need separate handling |

