#include "auth_service_impl.hpp"
#include <iostream>
#include <boost/multiprecision/cpp_int.hpp>

using namespace boost::multiprecision;

cpp_int AuthServiceImpl::bytes_to_cpp_int(const std::string& s) {
    if (s.empty()) {
        return 0;
    }
    cpp_int n;
    // import_bitsはビッグエンディアンのバイト列を期待する
    import_bits(n, s.begin(), s.end());
    return n;
}


grpc::Status AuthServiceImpl::Register(grpc::ServerContext* context,
                                        const zkp_auth::RegisterRequest* request,
                                        zkp_auth::RegisterResponse* response) {
    // Implementation of user registration
    std::cout << "Registering user: " << request->user() << std::endl;

    const std::string& user = request->user();
    if (user.empty()) {
        return grpc::Status(grpc::INVALID_ARGUMENT, "Username cannot be empty.");
    }

    cpp_int y1 = bytes_to_cpp_int(request->y1());
    cpp_int y2 = bytes_to_cpp_int(request->y2());

    // accessメソッドを使い、マップをスレッドセーフに操作する
    bool success = user_store_.access([&](auto& users) {
        if (users.count(user)) {
            return false;
        }

        UserInfo user_info = {
            .name = user,
            .y1 = y1,
            .y2 = y2
        };

        users[user] = std::move(user_info);
        return true;
    });

    if (!success) {
        return grpc::Status(grpc::ALREADY_EXISTS, "User already registered.");
    }

    return grpc::Status::OK;
}

grpc::Status AuthServiceImpl::CreateAuthenticationChallenge(
    grpc::ServerContext* context,
    const zkp_auth::AuthenticationChallengeRequest* request,
    zkp_auth::AuthenticationChallengeResponse* response) {
    // Implementation of creating authentication challenge
    std::cout << "Creating authentication challenge for user. usewr: " << request->user()  << std::endl;
    return grpc::Status::OK;
}

grpc::Status AuthServiceImpl::VerifyAuthentication(
    grpc::ServerContext* context,
    const zkp_auth::AuthenticationAnswerRequest* request,
    zkp_auth::AuthenticationAnswerResponse* response) {
    // Implementation of verifying authentication
    std::cout << "Verifying authentication for auth_id: " << request->auth_id() << std::endl;
    return grpc::Status::OK;
}