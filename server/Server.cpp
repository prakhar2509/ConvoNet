#include "Server.h"
#include <iostream>
#include <algorithm>
#include <cstring>
#include <openssl/err.h>
#include <cctype>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #define close closesocket
    #define perror(x) std::cerr << x << ": " << WSAGetLastError() << std::endl
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
#endif

Server::Server(int port) : port(port), server_socket(-1), ctx(nullptr) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        exit(EXIT_FAILURE);
    }
#endif

    initSSL();
    ctx = create_context();
    configure_context(ctx);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
#ifdef _WIN32
    if (server_socket == INVALID_SOCKET) {
#else
    if (server_socket == -1) {
#endif
        perror("Could not create socket");
        exit(EXIT_FAILURE);
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if(bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
    if(listen(server_socket, 5) < 0) {
        perror("Listen failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }
}

Server::~Server() {
#ifdef _WIN32
    if( server_socket != INVALID_SOCKET ) {
#else
    if( server_socket != -1 ) {
#endif
        close(server_socket);
    }
    if (ctx) SSL_CTX_free(ctx);

    cleanup_openssl();
#ifdef _WIN32
    WSACleanup();
#endif
}

void Server::initSSL() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void Server::cleanup_openssl() {
    EVP_cleanup();
}

SSL_CTX* Server::create_context() {
    const SSL_METHOD* method = TLS_server_method();
    SSL_CTX* context = SSL_CTX_new(method);
    if (!context) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return context;
}

void Server::configure_context(SSL_CTX* context) {
    // Load the server's certificate and private key from the server directory
    if (SSL_CTX_use_certificate_file(context, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    if (SSL_CTX_use_PrivateKey_file(context, "key.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

void Server::start() {
    std::cout << "Server is listening on port " << port << " ..." << std::endl;
    while (true) {
        sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);
#ifdef _WIN32
        SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_len);
        if (client_socket == INVALID_SOCKET) {
#else
        int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_len);
        if (client_socket < 0) {
#endif
            perror("Accept failed");
            continue;
        }
        std::cout << "Client connected from " << inet_ntoa(client_address.sin_addr) << std::endl;
        SSL* ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client_socket);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            close(client_socket);
        }
        else {
            std::thread client_thread(&Server::handleClient, this, client_socket, ssl);
            client_thread.detach();
        }
    }
}

#ifdef _WIN32
void Server::handleClient(SOCKET client_socket, SSL* ssl) {
#else
void Server::handleClient(int client_socket, SSL* ssl) {
#endif
    char buffer[4096];
    std::string username;

    SSL_write(ssl, "Enter your username: ", 21);
    int bytes_received = SSL_read(ssl, buffer, sizeof(buffer) - 1);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        username = std::string(buffer);
        std::erase_if(username, [](char c) {return std::isspace(static_cast<unsigned char>(c)); });
    }
    else {
        std::cerr << "Failed to read username." << std::endl;
        SSL_free(ssl);
        close(client_socket);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back({ client_socket, username, ssl });
        username_ssl_map[username] = ssl;
    }

    std::string welcome_message = "Server: " + username + " has joined the chat.";
    std::cout << welcome_message << std::endl;
    
    // Send join message to everyone EXCEPT the person who just joined
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        std::string formatted = welcome_message + "\n";
        for (const auto& client : clients) {
            if (client.ssl != ssl) {
                SSL_write(client.ssl, formatted.c_str(), formatted.length());
            }
        }
    }

    while (true) {
        memset(buffer, 0, sizeof(buffer));
        bytes_received = SSL_read(ssl, buffer, sizeof(buffer) - 1);

        if (bytes_received <= 0) {
            break;
        }

        buffer[bytes_received] = '\0';
        std::string message(buffer);
        std::erase_if(message, [](char c) { return c == '\r' || c == '\n'; });

        if (message.rfind("/pm", 0) == 0) {
            // Private message logic can be expanded here
        }
        else {
            // DON'T send message back to sender - let client handle it
            // Only send the message with username to everyone else
            std::string broadcast_msg = username + ": " + message;
            broadcastMessage(broadcast_msg, ssl); // EXCLUDE the sender
        }
    }

    removeClient(ssl);
    std::string disconnect_message = "Server: " + username + " has left the chat.";
    broadcastMessage(disconnect_message, nullptr);
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(client_socket);
}

void Server::broadcastMessage(const std::string& message, SSL* sender_ssl) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    std::string formatted_message = message + "\n";
    
    for (const auto& client : clients) {
        // Send to all clients EXCEPT the sender
        if (sender_ssl == nullptr || client.ssl != sender_ssl) {
            SSL_write(client.ssl, formatted_message.c_str(), formatted_message.length());
        }
    }
}

void Server::send_private_message(const std::string& message, const std::string& recipient_username, SSL* sender_ssl) {
    // Implementation left as an exercise
}

void Server::removeClient(SSL* client_ssl) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    std::string username_to_remove;
    std::erase_if(clients, [&](const client_t& client) {
        if (client.ssl == client_ssl) {
            username_to_remove = client.username;
            return true;
        }
        return false;
        });
    if (!username_to_remove.empty()) {
        username_ssl_map.erase(username_to_remove);
    }
}