#ifndef AUTH_SERVER_HPP
#define AUTH_SERVER_HPP

#include <grpcpp/grpcpp.h>

#include <memory>
#include <string>

namespace grpc
{
class Server;
}

namespace zkp_auth
{
class AuthServer
{
   public:
    AuthServer() = default;
    ~AuthServer();

    /**
     * @fn
     * @brief サーバを起動し、指定されたアドレスでリクエストを待ち受ける。
     * @param server_address サーバのアドレス（ex."0.0.0.0"）
     */
    void Run(const std::string& server_address);

   private:
    std::unique_ptr<grpc::Server> server_;
};
}  // namespace zkp_auth

#endif  // AUTH_SERVER_HPP