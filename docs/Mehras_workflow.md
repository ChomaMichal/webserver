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

  int _body_fd;              // FD for the file being served (streaming files)
  size_t _body_offset;       // Bytes of body already sent
  char _mem_body[8192];      // Fixed size memory buffer for dynamically generated bodies (e.g. Autoindex)
  bool _has_mem_body;        // Flag identifying whether to stream _body_fd or serve _mem_body from RAM
```

### 1.2: Method Handling Requirements (Merged with Code)

#### handleGET()
- **Current Support**: Serves regular files, including binary formats (implemented via `.bin` mapping).
- **Autoindex Behavior (Implemented in `generateDirectoryIndex()`)**: 
  1. **Search**: If `_filepath` is a directory, look for `index.html` inside it.
  2. **Serve**: If `index.html` exists, serve it (read-only) via `_body_fd`.
  3. **Generate**: If `index.html` is missing, dynamically generate an HTML listing of the directory into the `_mem_body` buffer array (no syscall allocations or dynamic string creation).
  4. **Safety**: Uses a `SAFE_MEMCAT` approach internally that truncates autoindex bodies preventing buffer overflow, instantly throwing a `413 Payload Too Large` to the user if buffer capacity is met.

#### handlePOST()
- **Current Support**: Writes `req.getBody()` to `_filepath` using `O_APPEND` to update files instead of overwriting.
- **Missing / Next steps**:
  1. **Conflict Check**: Added parent directory check using `stat()` to return `409 Conflict` if the parent path doesn't exist, and `403 Forbidden` if lacking write permissions.
  2. **Forbidden Check**: If `_filepath` is a directory, return `403 Forbidden`.
  3. **Uploads vs Append**: If the subject expects clean file uploads (where uploading `image.png` twice shouldn't corrupt the file by appending), use `O_TRUNC` instead of `O_APPEND`.
  4. **Status Codes**: Return `201 Created` for newly created files, and `200 OK` or `204 No Content` when updating/overwriting an existing file.
  5. **Location Rules**: Prevent `POST` requests from modifying static website assets by only allowing `POST` in specific configured directories (e.g., an `upload_store`).
  6. **CGI Processing**: If `POST` targets a CGI script (`.py`, `.php`), do NOT save the body to a file. Pass it to the script via `stdin` instead.

#### handleDelete()
- **Current Support**: Placeholder only (returns true). 
- **TODO / Next steps**:
  1. **Validation**: Check if `_filepath` points to a directory. 
  2. **Restriction**: If target is a directory, return `403 Forbidden`.
  3. **Action**: If it is a file, call `unlink(_filepath)`.
  4. **Status Codes**:
     - `204 No Content`: Successful deletion.
     - `404 Not Found`: Target missing.
     - `403 Forbidden`: Target is a directory or permission denied.

### 1.3: Chunker Logic (As Implemented)
The `chunker()` function in [client/Response.cpp](client/Response.cpp#L291) is the core of the streaming mechanism.
- It first exhausts `_header` using `_head_offset`.
- Next, it checks the `_has_mem_body` flag.
  - If `true`, it mem-copies from the internal static `_mem_body` buffer directly.
  - If `false`, it reads from `_body_fd` into the destination buffer.
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

### 3.2: The `sendResponse` Loop (Verified)
The `sendResponse()` implementation correctly pumps bytes from `Response::chunker` to `Stream::write`.

- It checks `POLLOUT` status via `_stream.getFdStatus()`.
- It calls `_response.chunker()` to retrieve the next chunk of data (headers or body).
- It uses `setSendBuffer()` and `_stream.write()` to transmit the data.
- It returns `true` (done) when `chunker` returns 0, and `false` (not done) if it needs to be called again in the next `poll()` cycle.

---

## Phase 4: Missing Enhancements (TODO)

### 4.1: Redirections (301/307)
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

#### Learn More (Sources):
- [MDN Web Docs: Redirections in HTTP](https://developer.mozilla.org/en-US/docs/Web/HTTP/Redirections) - Excellent overview of how browsers handle redirects.
- [MDN Web Docs: 301 Moved Permanently](https://developer.mozilla.org/en-US/docs/Web/HTTP/Status/301)
- [MDN Web Docs: 307 Temporary Redirect](https://developer.mozilla.org/en-US/docs/Web/HTTP/Status/307)
- [RFC 9110 (HTTP Semantics)](https://www.rfc-editor.org/rfc/rfc9110.html#name-redirection-3xx) - The official standard outlining the 3xx status codes.

### 4.2: Directory Indexer Helper
We need a robust way to list files:
```cpp
// Suggested location: utils/directory_utils.cpp
std::string get_html_list(const char* path); 
```

### 4.3: Error Page Customization
Instead of just `404 Not Found` in plain text, look for `root/errors/404.html`.

---

## Implementation Checklist
- [x] Basic GET/POST logic in `Response.cpp`
- [x] Implement Directory Listing (Autoindex)
- [x] Implement `.bin` support (content type `BIN`).
- [x] Implement Parent Directory check for POST (`_filepath` parent existence, permissions, 409 Conflict, 403 Forbidden)
- [x] Chunker-based streaming
- [x] File descriptor safety (automatic close in `reset()` and `chunker()`)
- [x] `sendResponse()` loop properly implemented in `Client`
- [x] Implement Directory Listing (Autoindex)
- [ ] Implement `handleDelete()` logic (currently just a placeholder)
- [ ] Handle 42 Webserv POST scenarios (CGI stdin routing, O_TRUNC/O_APPEND decisions per upload route, `201` vs `200/204` based on file existence)
- [ ] Implement Custom Error Handling (`Client` orchestrating `.html` pages)

---

## Appendix: HTTP Protocol Examples

### A.1: HTTP Request Format
Every request follows this fundamental structure that our `Request` class parses:

**Simple Example (Incoming Request):**
```http
POST /submit-form HTTP/1.1
Host: 127.0.0.1:2222
Content-Length: 12
Connection: close

Hello World!
```

**Key Components:**
1. **Request Line**: `POST /submit-form HTTP/1.1`
2. **Headers**: Key-value pairs followed by `\r\n`.
3. **Blank Line**: Separates headers from the body.
4. **Body**: The raw data (e.g., `Hello World!`).

### A.2: HTTP Response Format
This is what our `Response` class generates using `setHeader()` and `chunker()`:

```http
HTTP/1.1 201 Created\r\n
Server: webserv\r\n
Content-Type: text/plain\r\n
Content-Length: 0\r\n
Connection: close\r\n
\r\n
```
