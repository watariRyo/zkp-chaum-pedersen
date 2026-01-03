#include "auth_server.hpp"
#include <iostream>

int main() {
    std::string server_address("0.0.0.0:50051");
    AuthServer server;
    server.Run(server_address);

    return 0;
}