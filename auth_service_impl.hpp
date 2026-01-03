#ifndef AUTH_SERVICE_IMPL_HPP
#define AUTH_SERVICE_IMPL_HPP

#include <grpcpp/grpcpp.h>
#include "zkp_auth.grpc.pb.h"

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
};

#endif // AUTH_SERVICE_IMPL_HPP