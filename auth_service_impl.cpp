#include "auth_service_impl.hpp"
#include <iostream>

grpc::Status AuthServiceImpl::Register(grpc::ServerContext* context,
                                        const zkp_auth::RegisterRequest* request,
                                        zkp_auth::RegisterResponse* response) {
    // Implementation of user registration
    std::cout << "Registering user: " << request->user() << std::endl;
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