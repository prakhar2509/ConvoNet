#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <map>
#include <openssl/ssl.h>

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

struct client_t {
#ifdef _WIN32
    SOCKET socket;
#else
    int socket;
#endif
    std::string username;
    SSL* ssl;
};

class Server {
public:
    Server(int port);
    ~Server();
    void start();

private:
    void initSSL();
    void cleanup_openssl();
    SSL_CTX* create_context();
    void configure_context(SSL_CTX* ctx);

#ifdef _WIN32
    void handleClient(SOCKET client_socket, SSL* ssl);
#else
    void handleClient(int client_socket, SSL* ssl);
#endif
    void broadcastMessage(const std::string& message, SSL* sender_ssl);
    void send_private_message(const std::string& message, const std::string& recipient, SSL* sender_ssl);
    void removeClient(SSL* client_ssl);

#ifdef _WIN32
    SOCKET server_socket;
#else
    int server_socket;
#endif
    int port;
    SSL_CTX* ctx;
    std::vector<client_t> clients;
    std::map<std::string, SSL*> username_ssl_map;
    std::mutex clients_mutex;
};

#endif // !SERVER_H
