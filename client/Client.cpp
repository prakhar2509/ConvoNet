#include "Client.h"
#include <iostream>
#include <thread>
#include <cstring>
#include <openssl/err.h>
#include <mutex>
#include <atomic>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #define close closesocket
    #define perror(x) std::cerr << x << ": " << WSAGetLastError() << std::endl
#else
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <sys/socket.h>
#endif

// Global variables to handle prompt display
std::mutex display_mutex;
std::atomic<bool> showing_prompt{false};

Client::Client(const std::string& server_ip, int port)
    : server_ip(server_ip), port(port), client_socket(-1), ctx(nullptr), ssl(nullptr) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        exit(EXIT_FAILURE);
    }
#endif
    init_openssl();
    ctx = create_context();
}

Client::~Client() {
    if (ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
#ifdef _WIN32
    if (client_socket != INVALID_SOCKET) {
#else
    if (client_socket != -1) {
#endif
        close(client_socket);
    }
    if (ctx) {
        SSL_CTX_free(ctx);
    }
    cleanup_openssl();
#ifdef _WIN32
    WSACleanup();
#endif
}

void Client::init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void Client::cleanup_openssl() {
    EVP_cleanup();
}

SSL_CTX* Client::create_context() {
    const SSL_METHOD* method = TLS_client_method();
    SSL_CTX* context = SSL_CTX_new(method);
    if (!context) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return context;
}

bool Client::connect_to_server() {
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
    if (client_socket == INVALID_SOCKET) {
#else
    if (client_socket == -1) {
#endif
        perror("Socket creation failed");
        return false;
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    inet_pton(AF_INET, server_ip.c_str(), &server_address.sin_addr);

    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Connection Failed");
        return false;
    }

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client_socket);
    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        return false;
    }

    std::cout << "Connected to the server securely!" << std::endl;
    return true;
}

void Client::start() {
    // Get username first
    char buffer[4096];
    int bytes_received = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        std::cout << buffer; // Print "Enter your username: "
        
        std::getline(std::cin, username);
        SSL_write(ssl, username.c_str(), username.length());
    }
    
    std::thread receiver_thread(&Client::receive_messages, this);
    receiver_thread.detach();
    send_messages();
}

void Client::receive_messages() {
    char buffer[4096];
    while (true) {
        int bytes_received = SSL_read(ssl, buffer, sizeof(buffer) - 1);
        if (bytes_received <= 0) {
            std::cout << "Disconnected from server." << std::endl;
            exit(0);
        }
        buffer[bytes_received] = '\0';
        std::cout << buffer;
    }
}

void Client::send_messages() {
    std::string line;
    while (std::getline(std::cin, line)) {
        if (!line.empty()) {
            // Show user's own message without username prefix
            std::cout << line << std::endl;
            
            // Send to server
            SSL_write(ssl, line.c_str(), line.length());
        }
    }
}
