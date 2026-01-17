#include <grpcpp/grpcpp.h>

#include <iostream>
#include <memory>
#include <string>

#include "chaum_pedersen.hpp"
#include "zkp_auth.grpc.pb.h"
#include "zkp_constants.hpp"

using namespace boost::multiprecision;

// サーバと同じ変換関数をクライアント側にも用意
cpp_int bytes_to_cpp_int(const std::string& s)
{
    if (s.empty())
    {
        return 0;
    }
    // 16進数文字列からcpp_intに変換
    return cpp_int("0x" + s);
}

std::string cpp_int_to_bytes(const cpp_int& n)
{
    std::stringstream ss;
    ss << std::hex << n;
    return ss.str();
}

class AuthClient
{
   public:
    AuthClient(std::shared_ptr<grpc::Channel> channel) : stub_(zkp_auth::Auth::NewStub(channel)) {}

    cpp_int register_flow(const std::string user)
    {
        const auto constants = get_zkp_constants();
        ChaumPedersen cp(constants.p, constants.q, constants.g, constants.h);

        // Providerの秘密の知識X
        const cpp_int x = generate_random(constants.q);

        std::cout << "Registering user: " << user << std::endl;
        if (!register_user(cp, user, x))
        {
            std::cerr << "User registration failed or already exist." << std::endl;
            return -1;
        }
        std::cout << "User registered successfully." << std::endl;
        std::cout << "!!! IMPORTANT !!!" << std::endl;
        std::cout << "Your secret key (x) is: " << std::hex << x << std::endl;

        return x;
    }

    void login_flow(const std::string& user, const cpp_int& x)
    {
        const auto constants = get_zkp_constants();
        ChaumPedersen cp(constants.p, constants.q, constants.g, constants.h);

        std::cout << "Client starting authentication flow for user: " << user << std::endl;

        // CreateChallenge
        std::cout << "Creating authentication challenge..." << std::endl;
        // ランダムなNonce k を生成
        const cpp_int k = generate_random(constants.q);
        // コミットメントを作成
        Commitment commitment = cp.create_commitment(k);

        std::string auth_id;
        cpp_int challenge_c;
        if (!create_auth_challenge(user, commitment, auth_id, challenge_c))
        {
            std::cerr << "Failed to create authentication challenge." << std::endl;
            return;
        }
        std::cout << "Authentication challenge created. Auth ID: " << auth_id << std::endl;

        // VerifyAuthentication
        std::cout << "Verifying authentication..." << std::endl;
        Challenge challenge_c_struct{challenge_c};
        const Response response_s = cp.solve_response(k, challenge_c_struct, x);

        std::string session_id;
        if (!verify_authentication(auth_id, response_s, session_id))
        {
            std::cerr << "Authentication failed." << std::endl;
            return;
        }

        std::cout << "Authentication successful for user: " << user << ", session_id: " << session_id << std::endl;
    }
    ~AuthClient() {}

   private:
    bool register_user(const ChaumPedersen& cp, const std::string& user, const cpp_int& x)
    {
        zkp_auth::RegisterRequest request;
        request.set_user(user);

        // 公開鍵 y1, y2 を計算してセット
        PublicKeys public_keys = cp.calculate_public_keys(x);
        request.set_y1(cpp_int_to_bytes(public_keys.y1));
        request.set_y2(cpp_int_to_bytes(public_keys.y2));

        zkp_auth::RegisterResponse response;
        grpc::ClientContext context;
        grpc::Status status = stub_->Register(&context, request, &response);

        if (!status.ok())
        {
            if (status.error_code() != grpc::ALREADY_EXISTS)
            {
                std::cerr << "Register RPC failed: " << status.error_message() << std::endl;
            }
            return false;
        }
        return true;
    }

    bool create_auth_challenge(const std::string& user, const Commitment& commitment, std::string& out_auth_id,
                               cpp_int& out_c)
    {
        zkp_auth::AuthenticationChallengeRequest request;
        request.set_user(user);
        request.set_r1(cpp_int_to_bytes(commitment.r1));
        request.set_r2(cpp_int_to_bytes(commitment.r2));

        zkp_auth::AuthenticationChallengeResponse response;
        grpc::ClientContext context;
        grpc::Status status = stub_->CreateAuthenticationChallenge(&context, request, &response);

        if (!status.ok())
        {
            std::cerr << "CreateAuthenticationChallenge RPC failed: " << status.error_message() << std::endl;
            return false;
        }

        out_auth_id = response.auth_id();
        out_c = bytes_to_cpp_int(response.c());
        return true;
    }

    bool verify_authentication(const std::string& auth_id, const Response& response_s, std::string& out_session_id)
    {
        zkp_auth::AuthenticationAnswerRequest request;
        request.set_auth_id(auth_id);
        request.set_s(cpp_int_to_bytes(response_s.s));

        zkp_auth::AuthenticationAnswerResponse response;
        grpc::ClientContext context;
        grpc::Status status = stub_->VerifyAuthentication(&context, request, &response);

        if (!status.ok())
        {
            std::cerr << "VerifyAuthentication RPC failed: " << status.error_message() << std::endl;
            return false;
        }

        out_session_id = response.session_id();
        return true;
    }

    std::unique_ptr<zkp_auth::Auth::Stub> stub_;
};

void print_usage()
{
    std::cerr << "Usage:\n"
              << "  ./auth_client register <username>\n"
              << "  ./auth_client login <username> <secret_key_hex>\n";
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        print_usage();
        return 1;
    }

    std::string target("0.0.0.0:50051");
    AuthClient client(grpc::CreateChannel(target, grpc::InsecureChannelCredentials()));

    std::string mode = argv[1];
    std::string user = argv[2];

    if (mode == "register")
    {
        cpp_int x = client.register_flow(user);
        if (x == -1)
        {
            return 1;
        }
        client.login_flow(user, x);
    }
    else if (mode == "login")
    {
        if (argc < 4)
        {
            print_usage();
            return 1;
        }
        // 16進数文字列として秘密鍵xを受け取り変換
        cpp_int x("0x" + std::string(argv[3]));
        client.login_flow(user, x);
    }
    else
    {
        print_usage();
        return 1;
    }

    return 0;
}