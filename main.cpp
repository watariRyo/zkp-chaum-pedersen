#include <iostream>

#include "auth_server.hpp"

int main()
{
    std::string server_address("0.0.0.0:50051");
    AuthServer server;
    server.Run(server_address);

    return 0;
}