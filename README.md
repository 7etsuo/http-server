<p align="center">
  <img src="https://github.com/user-attachments/assets/07113e1e-408c-4c65-9bd9-132ba4030af8" alt="Logo" width="200" />
</p>

<h1 align="center">HTTP Server in C</h1>

<p align="center">
  <strong>This project implements a simple HTTP server in C. It handles various HTTP methods, processes requests, and generates responses based on client requests. The server is built using low-level socket programming and custom HTTP request/response handling.</strong>
</p>

---

<h3 align="center">Connect with me:</h3>
<p align="center">
  <a href="https://www.x.com/7etsuo" target="_blank">
    <img src="https://img.shields.io/badge/X-7etsuo-blue?style=flat&logo=x" alt="X Profile" />
  </a>
  &nbsp;
  <a href="https://www.discord.gg/c-asm" target="_blank">
    <img src="https://img.shields.io/badge/Discord-c--asm-7289DA?style=flat&logo=discord" alt="Discord Server" />
  </a>
</p>

---

### Features:
- **HTTP Request Parsing:** Support for multiple HTTP methods including GET, POST, PUT, DELETE, and more.
- **Request Dispatching:** Dispatches requests based on URI and method.
- **Non-Blocking I/O:** Uses non-blocking sockets to handle multiple clients concurrently.
- **Customizable Response:** Dynamically builds and sends HTTP responses based on the request and server logic.
- **Modular Design:** Organized into multiple files for handling sockets, HTTP requests, responses, and server operations.

### Files Overview:
- `http_handler.c` / `http_handler.h`: Manages the request handling and dispatch logic.
- `my_socket.c` / `my_socket.h`: Contains functions for setting up and managing server sockets.
- `request.c` / `request.h`: Responsible for parsing HTTP requests and managing request headers and body.
- `response.c` / `response.h`: Handles building and formatting HTTP responses, including headers and body.
- `server.c`: Main entry point of the server, manages client connections and the server loop.
- `my_http.h`: Contains common HTTP-related constants and definitions used across the project.

### Usage:
1. **Clone the repository:**
   `git clone [repository URL]`
2. **Compile the project:**
   Navigate to the project directory and run `make`.
3. **Run the server:**
   Execute `./server` and handle incoming HTTP requests on the specified port.

### Example:
Start the server and visit `http://localhost:4221/` to interact with it. It handles routes like `/`, `/echo/`, and `/user-agent` and responds with the appropriate content.

### Requirements:
- **Compiler:** GCC (or another C compiler)
- **Environment:** POSIX-compliant system (Linux/macOS)

This project serves as a base for further extensions like adding more HTTP methods, handling file uploads, or implementing a full-fledged web server.

---

