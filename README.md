# ConvoNet: Secure SSL/TLS Chat Application

A modern, multithreaded client-server chat application built with **C++20** that provides secure real-time communication using SSL/TLS encryption. ConvoNet enables multiple clients to connect to a central server and communicate in real-time with end-to-end encryption.

## Features

- **🔐 SSL/TLS Encryption**: All communication between clients and server is encrypted using OpenSSL
- **👥 Multi-Client Support**: Server handles multiple concurrent clients using multithreading
- **💬 Real-time Messaging**: Instant message broadcasting to all connected users
- **🚀 Cross-Platform**: Supports both Windows and Linux/Unix systems
- **🏷️ Username System**: Users can set custom usernames for identification
- **📤 Private Messaging**: Framework for direct user-to-user communication (expandable)
- **🔌 Graceful Connection Handling**: Proper client connect/disconnect notifications
- **🧵 Thread-Safe Architecture**: Concurrent client management with mutex protection

## Technical Specifications

- **Language**: C++20
- **Build System**: CMake (minimum 3.10, tested with 3.31.6)
- **Generator**: Ninja
- **Dependencies**: OpenSSL, Threads library
- **Networking**: TCP sockets with SSL/TLS layer
- **Architecture**: Multi-threaded server with thread-safe client management
- **Platform Support**: Windows (Winsock), Linux/Unix (POSIX sockets)

## Prerequisites

Before building ConvoNet, ensure you have the following installed:

### Required Dependencies
- **C++20 compatible compiler**:
  - GCC 10+ (Linux/macOS)
  - Clang 12+ (Linux/macOS)
  - MSVC 2019+ (Windows)
- **CMake** 3.10 or higher
- **OpenSSL** development libraries
- **Ninja** build system (recommended)

### Installing Dependencies

#### Ubuntu/Debiansudo apt update
sudo apt install build-essential cmake ninja-build libssl-dev
#### CentOS/RHEL/Fedorasudo yum groupinstall "Development Tools"
sudo yum install cmake ninja-build openssl-devel
#### macOS (with Homebrew)brew install cmake ninja openssl
#### Windows
- Install Visual Studio 2019+ with C++ development tools
- Download and install OpenSSL from [OpenSSL for Windows](https://slproweb.com/products/Win32OpenSSL.html)
- Install CMake from [cmake.org](https://cmake.org/download/)
- Install Ninja: `choco install ninja` (with Chocolatey) or download from [ninja-build.org](https://ninja-build.org/)

## Installation & Build

### Step 1: Clone the Repositorygit clone https://github.com/yourusername/ConvoNet.git
cd ConvoNet
### Step 2: Generate SSL Certificates
Before running the server, you need to generate SSL certificates. Navigate to the `server/` directory:cd server
openssl req -x509 -nodes -days 365 -newkey rsa:4096 -keyout key.pem -out cert.pem
When prompted, you can use the following example values:
- Country Name: US
- State or Province Name: California
- Locality Name: San Francisco
- Organization Name: ConvoNet
- Common Name: localhost

### Step 3: Build the Project
From the project root directory:cmake -B build -G Ninja
cmake --build build
For traditional make:cmake -B build
make -C build
#### Windows Build Notes
If OpenSSL is installed in a custom location:cmake -DOPENSSL_ROOT_DIR="C:\Program Files\OpenSSL-Win64" -B build
cmake --build build
## Usage

### Starting the Server
From the build directory:./server
The server will start listening on port 8080 by default and display:Server started on port 8080
### Connecting Clients
In separate terminal windows, connect clients using:./client
For local testing:./client localhost
### Example Session

**Server Output:**[INFO] Client connected: Alice
[INFO] Client connected: Bob
[INFO] Alice: Hello, everyone!
[INFO] Bob: Hi Alice!
**Client Output (Alice):**Welcome to ConvoNet!
Enter your username: Alice
[Bob]: Hi Alice!
**Client Output (Bob):**Welcome to ConvoNet!
Enter your username: Bob
[Alice]: Hello, everyone!
## Architecture Overview

### Server Architecture
- **Main Thread**: Accepts incoming SSL connections on port 8080
- **Client Threads**: Each connected client is handled by a dedicated thread using `std::thread`
- **Thread Safety**: Client list and username mapping protected by `std::mutex`
- **SSL Context**: Shared SSL context (`SSL_CTX*`) for all client connections
- **Message Broadcasting**: Thread-safe message distribution to all connected clients

### Client Architecture
- **Main Thread**: Handles user input and sends messages to server
- **Receive Thread**: Continuously listens for incoming messages from server using `std::thread`
- **SSL Connection**: Maintains encrypted connection to server with automatic reconnection handling
- **User Interface**: Simple terminal-based interface with real-time message display

### Message Flow
1. Client establishes SSL connection to server
2. Server requests username, client responds
3. Server broadcasts join notification to all other clients
4. Messages are broadcast to all connected clients except sender
5. Server handles graceful disconnections and notifies remaining clients

### Key Components
- **`Server` class**: Manages SSL context, client connections, and message broadcasting
- **`Client` class**: Handles SSL connection, message sending/receiving, and user input
- **SSL/TLS layer**: Encrypts all communication using OpenSSL
- **Cross-platform sockets**: Windows (Winsock) and Unix (POSIX) compatibility

## Security Features

### SSL/TLS Implementation
- **Protocol**: TLS 1.2+ (OpenSSL default configuration)
- **Certificate**: Self-signed X.509 certificate (can be replaced with CA-signed)
- **Key Size**: RSA 4096-bit encryption
- **Certificate Validation**: Server certificate validation (configurable)
- **Cipher Suites**: Modern cipher suites with forward secrecy

### Security Considerations
- Private keys (`key.pem`) are automatically excluded from version control via `.gitignore`
- SSL context properly initialized with secure defaults
- Memory is properly cleared after use with `memset`
- Connection timeouts and comprehensive error handling implemented
- Thread-safe operations
