#if defined(_WIN32)
#include <openssl/applink.c>
#endif

#include "Server.h"
#include <iostream>

int main() {
    const int PORT = 8080;

    try {
        Server chat_server(PORT);
        chat_server.start();
    }
    catch (const std::exception& e) {
        std::cerr << "An exception occurred: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "An unknown error occurred." << std::endl;
        return 1;
    }

    return 0;
}
