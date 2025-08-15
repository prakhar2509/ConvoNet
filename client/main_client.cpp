#include "Client.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <port>" << std::endl;
        return 1;
    }

    const std::string server_ip = argv[1];
    int port;
    try {
        port = std::stoi(argv[2]);
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "Error: Invalid port number." << std::endl;
        return 1;
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Error: Port number out of range." << std::endl;
        return 1;
    }

    Client client(server_ip, port);

    if (client.connect_to_server()) {
        client.start();
    }
    else {
        std::cerr << "Could not connect to the server. Exiting." << std::endl;
        return 1;
    }

    return 0;
}
