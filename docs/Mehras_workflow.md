# HTTP Response Implementation - Workflow & TODOs

## Overview
Implement HTTP response creation and sending through the client. Response is serialized into chunks and sent via `_stream` repeatedly until complete.

---

## Phase 1: Response Class Alignment

### 1.1: Current State of Response.hpp (Verified)
The `Response` class uses the following members to track state:
```cpp
private:
  bool _header_sent;         // Whether headers have been sent
  size_t _head_offset;       // Bytes of header already sent
  size_t _header_size;       // Total size of the generated header
  char _header[MAX_HEADER_SIZE]; 
  
  char _filepath[MAX_FILE_PATH]; // Full path of target file
  int _status_code;          // e.g., 200, 404, 201, 204
  e_content_type _content_type; 
  ssize_t _content_len;      // Size of the body (file size or 0)

  int _body_fd;              // FD for the file being served
  size_t _body_offset;       // Bytes of body already sent
```

### 1.2: Method Handling Requirements (Merged with Code)

#### handleGET()
- **Current Support**: Serves regular files.
- **Autoindex Behavior**: 
  1. **Search**: If `_filepath` is a directory, look for `index.html` inside it.
  2. **Serve**: If `index.html` exists, serve it (read-only).
  3. **Generate**: If `index.html` is missing, **dynamically generate** an HTML listing of the directory contents (do not create a file on disk).
- **helper needed**: `Result<bool> generateDirectoryIndex(const char *dir_path)` - should generate HTML string, set `_status_code = 200`, and provide it to `_response`.

#### handlePOST()
- **Current Support**: Writes `req.getBody()` to `_filepath`.
- **Missing / Next steps**:
  1. **Conflict Check**: If parent directory of `_filepath` doesn't exist, return `409 Conflict`.
  2. **Forbidden Check**: If `_filepath` is a directory, return `403 Forbidden`.
  3. **helper needed**: `bool parentDirectoryExists(const char *path)` using `stat()`.

#### handleDelete()
- **Current Support**: Deletes file at `_filepath`. Correctly prevents directory deletion.
- **Verified Status Codes**:
  - `204 No Content`: Successful deletion.
  - `404 Not Found`: Target missing.
  - `403 Forbidden`: Target is a directory or permission denied.

### 1.3: Chunker Logic (As Implemented)
The `chunker()` function in [client/Response.cpp](client/Response.cpp#L291) is the core of the streaming mechanism.
- It first exhausts `_header` using `_head_offset`.
- Then it reads from `_body_fd` into the personal buffer.
- It returns the number of bytes read/written to the buffer.
- It automatically closes `_body_fd` when finished.

---

## Phase 2: Path Resolution Strategy (Centralized in Client)

### 2.1: Client Logic for Path Resolution
To handle complex requirements (error pages, POST/DELETE restrictions, and root switching), the **Client** should act as the authority for path resolution before calling `Response`.

**Responsibilities of `Client::setFilePath()`:**
1. **Method-Based Rooting**: 
   - `GET` -> `./root`
   - `POST/DELETE` -> `./root/tmpfiles`
2. **Path Sanitization**: Ensure the URI doesn't "break out" of the root (e.g., handles `..`).
3. **Internal Error Handling**: 
   - If `Response::handleRequest` returns an error, the `Client` checks for a custom error page (e.g., `./root/errors/404.html`).
   - If the custom file exists, the `Client` updates `_filepath` and calls `Response::handleGet` to serve the error page as a body.
   - If no custom page exists, fallback to `Response::handleError()` for a plain-text response.

### 2.2: Response Logic for File Access
The **Response** class should remain "dumb" regarding where files live. It only cares if the path provided by the `Client` is valid and accessible.

**Responsibilities of `Response` handlers:**
1. **Validation**: Call `stat()` on the `_filepath` provided.
2. **Action**: `open()` for reading (GET), `open(O_CREAT)` for writing (POST), or `unlink()` (DELETE).
3. **Feedback**: Return a specific status code if the operation fails.
4. **handleError()**: The final fallback that generates a minimal plain-text header/body (no file I/O).

---

## Phase 3: Client Class Integration

### 3.1: Client State (Verified)
The `Client` class orchestrates the lifecycle:
```cpp
private:
  void setFilePath(); // Logic for method roots and error pages
  Stream _stream;     
  Request _request;   
  Response _response; 
  bool _response_ready;
```

### 2.2: The `sendResponse` Loop (TODO)
The `sendResponse()` implementation needs to correctly pump bytes from `Response::chunker` to `Stream::write`.

```cpp
Result<bool> Client::sendResponse(void) {
  if (!_response_ready) {
    return Result<bool>("Response not ready");
  }

  char buf[MAX_SEND_BUFFER];
  size_t bytes = _response.chunker(buf, MAX_SEND_BUFFER);

  if (bytes > 0) {
    // _stream needs a way to take a raw buffer or we use setSendBuffer
    setSendBuffer(buf, bytes); 
    auto res = _stream.write();    // Sends current buffer
    if (res.is_error()) return res;
    return Result<bool>(false);    // Not fully sent
  }

  return Result<bool>(true); // Fully sent
}
```

---

## Phase 3: Missing Enhancements (TODO)

### 3.1: Redirections (301/307)
Redirections are used to tell the client that the resource they requested is at a different URI.

#### When and Why?
- **301 Moved Permanently**: Used when a resource has a new permanent home. Search engines will update their links. (e.g., `/old-page` -> `/new-page`).
- **307 Temporary Redirect**: Used when the move is temporary. Crucially, `307` guarantees that the client **will not change the HTTP method** (e.g., a `POST` must remain a `POST` at the new location).

#### How it should be handled:
1. **Detection (Client)**: In `Client::setFilePath()`, if the URI matches a redirection rule (configured or hardcoded), set the status code and internal redirect flag.
2. **Construction (Response)**: The `Response` must add a `Location` header pointing to the new URI.
3. **Execution**: The server sends the header and **no body** (or a minimal informative body).

**Example Scenario:**
If a user tries to `GET /old-index`, the `Client` logic detects this:
```cpp
// Inside Client::setFilePath()
if (uri == "/old-index") {
    _response.setStatusCode(301);
    _response.addHeader("Location", "/index.html");
    return;
}
```

The resulting HTTP response sent to the browser:
```http
HTTP/1.1 301 Moved Permanently
Server: webserv
Location: /index.html
Content-Length: 0
Connection: close
```

### 3.2: Directory Indexer Helper
We need a robust way to list files:
```cpp
// Suggested location: utils/directory_utils.cpp
std::string get_html_list(const char* path); 
```

### 3.3: Error Page Customization
Instead of just `404 Not Found` in plain text, look for `root/errors/404.html`.

---
