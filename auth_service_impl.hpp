#ifndef AUTH_SERVICE_IMPL_HPP
#define AUTH_SERVICE_IMPL_HPP

#include <boost/multiprecision/cpp_int.hpp>
#include <mutex>
#include <unordered_map>
#include <grpcpp/grpcpp.h>
#include "zkp_auth.grpc.pb.h"

using namespace boost::multiprecision;

class AuthServiceImpl final : public zkp_auth::Auth::Service {
public:
    grpc::Status Register(grpc::ServerContext* context,
                          const zkp_auth::RegisterRequest* request,
                          zkp_auth::RegisterResponse* response) override;

    grpc::Status CreateAuthenticationChallenge(
        grpc::ServerContext* context,
        const zkp_auth::AuthenticationChallengeRequest* request,
        zkp_auth::AuthenticationChallengeResponse* response) override;

    grpc::Status VerifyAuthentication(
        grpc::ServerContext* context,
        const zkp_auth::AuthenticationAnswerRequest* request,
        zkp_auth::AuthenticationAnswerResponse* response) override;
private:
    cpp_int bytes_to_cpp_int(const std::string& s);

    struct UserInfo {
        // registration
        std::string name;
        cpp_int y1;
        cpp_int y2;
        // authorization
        cpp_int r1;
        cpp_int r2;
        // verification
        cpp_int c;
        cpp_int s;
        std::string session_id;
    };
    
    // 複数リクエストからの同時アクセス保護のためのユーザーストア
    struct UserStore {
        std::mutex mutex;
        std::unordered_map<std::string, UserInfo> user_info;

        // RAIIパターンで例外発生時も必ずロックを解放する
        template<typename Func>
        auto access(Func f) {
            std::lock_guard<std::mutex> lock(mutex);
            return f(user_info);
        }
    };

    UserStore user_store_;
};

#endif // AUTH_SERVICE_IMPL_HPP