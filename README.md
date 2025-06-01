# Hashd Server

A lightweight C++ server application that responds to client requests with the SHA-256 hash of the received data. It supports long request payloads and is designed to handle a large number of concurrent clients efficiently.

The server configuration includes options for setting the port, poll timeout, and the number of worker threads. It uses multithreading and handles shutdown signals (`SIGINT`, `SIGTERM`) gracefully.

## Features

- Configurable server port
- Adjustable poll timeout (in milliseconds)
- Customizable number of worker threads
- Graceful shutdown via signal handling
- Uses OpenSSL library to compute SHA-256 hashes

## Requirements

- C++17 or newer
- POSIX-compatible system (Linux/macOS)
- CMake (recommended) or manual compilation
- OpenSSL development libraries

## Build Instructions

### build binaries

```bash
mkdir build
cd build
cmake ..
make
make install
```

### build and run docker container
```bash
docker build -t hashd_server 
docker run -it hashd_server
```

## Usage

```
./hashd [-p port] [-t timeout_ms] [-w workers] [-h]
```

### Options
```
-p <port>: Port number to bind the server (default: 11111)
-t <timeout>: Poll timeout in milliseconds (default: 100)
-w <workers>: Number of worker threads (default: 4)
-h: Show help message and exit
```

### Example
```
./hashd -p 8080 -t 200 -w 8
```
Starts the server on port 8080, with a poll timeout of 200 ms and 8 worker threads.

### Graceful Shutdown

The server listens for termination signals like Ctrl+C (SIGINT) or SIGTERM and shuts down cleanly when they are received.

## License
This project is licensed under the MIT License. See [LICENSE](https://opensource.org/license/mit) for details.
