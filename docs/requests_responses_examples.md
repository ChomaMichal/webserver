# Webserv: Requests and Responses for `simple.conf`

Based on standard behavior for the 42 `webserv` project and your `simple.conf` configuration, here is what various HTTP requests and their corresponding expected responses would look like.

## Context from `simple.conf`
- **Server 1**: Listens on `127.0.0.1:8080`, root is `./root/`, `autoindex on`, file uploads enabled to `./root/uploads`, max payload is 10MB.
- **Server 2**: Listens on `127.0.0.2:8080` (similar config but `autoindex off` for `/api`).

---

## 1. Standard GET Request (Serving an `index.html`)

**Request:**
```http
GET / HTTP/1.1
Host: 127.0.0.1:8080
Accept: text/html
```

**Expected Response (200 OK):**
The server maps `/` to `./root/` and looks for the `index.html` file specified by the `index` directive.
```http
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: [size of index.html]
Connection: keep-alive

<html>
... content of ./root/index.html ...
</html>
```

---

## 2. Directory Listing (Autoindex ON)

**Request:**
```http
GET /tmpfiles/ HTTP/1.1
Host: 127.0.0.1:8080
```

**Expected Response (200 OK):**
Because `autoindex on` is set globally for `127.0.0.1:8080`, and assuming there is no `index.html` inside `./root/tmpfiles/`, the server returns a generated HTML directory listing.
```http
HTTP/1.1 200 OK
Content-Type: text/html
Content-Length: [size of generated html]

<html>
<head><title>Index of /tmpfiles/</title></head>
<body>
<h1>Index of /tmpfiles/</h1><hr><pre>
<a href="../">../</a>
<a href="data.json">data.json</a>
<a href="empty.txt">empty.txt</a>
...
</pre><hr></body>
</html>
```

---

## 3. Directory Listing Denied (Autoindex OFF)

**Request:**
```http
GET /api2/ HTTP/1.1
Host: 127.0.0.1:8080
```

**Expected Response (403 Forbidden):**
The `/api2` location block explicitly specifies `autoindex off`. If there is no `index.html` in `./root/api2/`, direct access to the directory is forbidden.
```http
HTTP/1.1 403 Forbidden
Content-Type: text/html
Content-Length: ...

<html>
<head><title>403 Forbidden</title></head>
...
</html>
```

---

## 4. Custom Error Page (404 Not Found)

**Request:**
```http
GET /does-not-exist.txt HTTP/1.1
Host: 127.0.0.1:8080
```

**Expected Response (404 Not Found):**
The file doesn't exist in `./root/does-not-exist.txt`. The server uses the `404 /errors/404.html` directive.
```http
HTTP/1.1 404 Not Found
Content-Type: text/html
Content-Length: [size of 404.html]

<html>
... content of ./root/errors/404.html ...
</html>
```

---

## 5. File Upload (POST Request)

**Request:**
```http
POST /upload_endpoint HTTP/1.1
Host: 127.0.0.1:8080
Content-Type: text/plain
Content-Length: 13

Hello World!
```
*(Note: Upload semantics depend heavily on how your 42 setup specifically handles the POST endpoint, but it's generally stored at the `upload_location`)*

**Expected Response (201 Created):**
The file is saved to `./root/uploads/`.
```http
HTTP/1.1 201 Created
Location: /uploads/new_file.txt
Content-Length: 0

```

---

## 6. Payload Too Large (413 Entity Too Large)

**Request:**
```http
POST / HTTP/1.1
Host: 127.0.0.1:8080
Content-Type: text/plain
Content-Length: 15000000

... (15 Megabytes of data) ...
```

**Expected Response (413 Payload Too Large):**
Because `max_payload_size 10485760` restricts bodies to 10MB, the server immediately rejects the request.
```http
HTTP/1.1 413 Payload Too Large
Content-Type: text/plain
Content-Length: ...

Payload Too Large
```

---

## 7. Location Blocks (`/api` and `/api2` root overrides)

In your config, specific routes (`/api` and `/api2`) have their own `root` directives. How does the server know which one to pick? The rule standard in `webserv` (and Nginx) is **Longest Prefix Match**. The server compares the request URI to all `location` blocks and chooses the one that has the longest matching starting characters.

### Scenario A: Hitting `/api2`

**Request:**
```http
GET /api2/data.json HTTP/1.1
Host: 127.0.0.1:8080
```

**Why it picks `/api2`:**
1. The requested URI is `/api2/data.json`.
2. The server has location blocks for `/`, `/api`, and `/api2`.
3. Both `/` and `/api` might seem like they could match in a sloppy parser, but `/api2` is an exact longest-prefix match for the starting route. Therefore, it applies the `/api2` block rules.

**Expected Response (200 OK):**
The server uses the overridden `root ./root/api2`. It searches for `./root/api2/api2/data.json` (or `./root/api2/data.json` if your webserv implementation treats `root` as an `alias` replacing the URI part).
```http
HTTP/1.1 200 OK
Content-Type: application/json
...

[Content of the data.json file inside the api2 root]
```

### Scenario B: Directory Access on `/api2`

**Request:**
```http
GET /api2/ HTTP/1.1
Host: 127.0.0.1:8080
```

**Expected Response (403 Forbidden):**
Because the server matched the `/api2` location block, it sees the explicit directive `autoindex off` (which overrides the global `autoindex on`). If there's no `index.html` inside the `/api2` directory, it cannot list the contents.
```http
HTTP/1.1 403 Forbidden
Content-Type: text/html
...
[403 Forbidden Error Page]
```

### Scenario C: Hitting `/api`

**Request:**
```http
GET /api/hello.txt HTTP/1.1
Host: 127.0.0.1:8080
```

**Why it picks `/api` instead of `/api2`:**
1. The requested URI is `/api/hello.txt`.
2. It matches the `/api` block perfectly.
3. It does *not* match the `/api2` block because the URI does not contain the "2". The server routes it purely based on the text prefix of the URI matching the `location` declaration.

**Expected Response (200 OK):**
The server matches the `location /api` block. It uses the `root ./root/api` instead of the global `./root/`. Assuming the file exists, it serves it.
```http
HTTP/1.1 200 OK
Content-Type: text/plain
...

[Content of the hello.txt file inside the api root]
```

---

## 8. Server Name and Port Collisions (Two Servers on `127.0.0.1:8080`)

In your `simple.conf`, there are effectively three server blocks, but **Server 1** and **Server 3** share the exact same configuration for matching incoming connections:
- Both listen on `127.0.0.1:8080`.
- Both have `server_name example.com`.

**What happens when a conflict like this exists?**

According to standard Nginx/webserv rules for resolving a request:
1. **Match the IP and Port:** The server first finds all `server` blocks matching the requested IP address and port (e.g., `127.0.0.1:8080`).
2. **Match the `Host` Header:** If there are multiple blocks for that IP:Port, it looks at the `Server_name` matching the `Host` header strictly.
3. **Handle Duplicates:** If there are duplicate combinations of `listen` IP/Port AND `server_name`, the server will always default to the **first defined block** in the configuration file.

### Example Conflict Resolution

**Request to Server 3's logic:**
```http
GET /api/ HTTP/1.1
Host: example.com
```

**What ACTUALLY happens:**
Even though Server 3 explicitly sets `autoindex off` for `/api`, this request will **never reach Server 3**. 
Because Server 1 is defined first in `simple.conf` and has the exact same IP, port, and `server_name`, it "shadows" Server 3. Server 1 will handle the request. Since Server 1 inherits `autoindex on` for `/api`, it will return a directory listing (200 OK) instead of the 403 Forbidden that Server 3 would have given.

### What if a file exists on Server 3 but not Server 1?

**Request:**
```http
GET /only_on_server_3.txt HTTP/1.1
Host: 127.0.0.1:8080
```

**What happens:**
Routing happens **before** the filesystem is ever checked. 
1. The request comes in.
2. The server maps it to `Server 1` because it is the first matching block for `127.0.0.1:8080`.
3. The server looks for `./root/only_on_server_3.txt` (Server 1's root).
4. **It is not found.**

Even though `only_on_server_3.txt` might perfectly exist inside Server 3's root directory, the server **will not** fall back to check Server 3. It simply returns Server 1's 404 page (`/errors/404.html`) immediately. Once a request is bound to a server block, it stays locked to that block's rules.
