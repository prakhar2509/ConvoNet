#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <openssl/ssl.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <sys/socket.h>
#endif

class Client {
public:
    Client(const std::string& server_ip, int port);
    ~Client();
    bool connect_to_server();
    void start();

private:
    void init_openssl();
    void cleanup_openssl();
    SSL_CTX* create_context();

    void receive_messages();
    void send_messages();

#ifdef _WIN32
    SOCKET client_socket;
#else
    int client_socket;
#endif
    std::string server_ip;
    int port;
    SSL* ssl;
    SSL_CTX* ctx;
    std::string username;
};

#endif // !CLIENT_H
