#ifndef AUTH_SERVER_HPP
#define AUTH_SERVER_HPP

#include <string>
#include <memory>
#include <grpcpp/grpcpp.h>

namespace grpc {
    class Server;
}

class AuthServer {
    public:
        AuthServer() = default;
        ~AuthServer();

        void Run(const std::string& server_address);

    private:
        std::unique_ptr<grpc::Server> server_;
};

#endif // AUTH_SERVER_HPP