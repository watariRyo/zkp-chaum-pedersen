#include "auth_service_impl.hpp"

#include <boost/multiprecision/cpp_int.hpp>
#include <iostream>

#include "chaum_pedersen.hpp"
#include "zkp_constants.hpp"

using namespace boost::multiprecision;

cpp_int AuthServiceImpl::bytes_to_cpp_int(const std::string& s)
{
    if (s.empty())
    {
        return 0;
    }
    // 16進数文字列からcpp_intに変換
    return cpp_int("0x" + s);
}

std::string AuthServiceImpl::cpp_int_to_bytes(const cpp_int& n)
{
    std::stringstream ss;
    ss << std::hex << n;
    return ss.str();
}

grpc::Status AuthServiceImpl::Register(grpc::ServerContext* context, const zkp_auth::RegisterRequest* request,
                                       zkp_auth::RegisterResponse* response)
{
    // Implementation of user registration
    std::cout << "Registering user: " << request->user() << std::endl;

    const std::string& user = request->user();
    if (user.empty())
    {
        return grpc::Status(grpc::INVALID_ARGUMENT, "Username cannot be empty.");
    }

    cpp_int y1 = bytes_to_cpp_int(request->y1());
    cpp_int y2 = bytes_to_cpp_int(request->y2());

    // accessメソッドを使い、マップをスレッドセーフに操作する
    bool success = user_store_.access(
        [&](auto& users)
        {
            if (users.count(user))
            {
                return false;
            }

            UserInfo user_info = {.name = user, .y1 = y1, .y2 = y2};

            users[user] = std::move(user_info);
            return true;
        });

    if (!success)
    {
        return grpc::Status(grpc::ALREADY_EXISTS, "User already registered.");
    }

    return grpc::Status::OK;
}

grpc::Status AuthServiceImpl::CreateAuthenticationChallenge(grpc::ServerContext* context,
                                                            const zkp_auth::AuthenticationChallengeRequest* request,
                                                            zkp_auth::AuthenticationChallengeResponse* response)
{
    // Implementation of creating authentication challenge
    std::cout << "Creating authentication challenge for user. user: " << request->user() << std::endl;

    const std::string& user = request->user();
    if (user.empty())
    {
        return grpc::Status(grpc::INVALID_ARGUMENT, "Username cannot be empty.");
    }

    // RFC5114のqを使用
    auto constants = get_zkp_constants();
    cpp_int q = constants.q;
    cpp_int c = generate_random(q);
    std::string auth_id = generate_auth_id();

    bool user_found = user_store_.access(
        [&](auto& users)
        {
            auto it = users.find(user);
            if (it == users.end())
            {
                return false;
            }
            session_store_.access(
                [&](auto& sessions)
                {
                    AuthSession session = {.user = user,
                                           .r1 = bytes_to_cpp_int(request->r1()),
                                           .r2 = bytes_to_cpp_int(request->r2()),
                                           .c = c};
                    sessions[auth_id] = std::move(session);
                });
            return true;
        });

    if (!user_found)
    {
        return grpc::Status(grpc::NOT_FOUND, "User not found.");
    }

    response->set_auth_id(auth_id);
    response->set_c(cpp_int_to_bytes(c));  // 16進数文字列としてセット

    return grpc::Status::OK;
}

grpc::Status AuthServiceImpl::VerifyAuthentication(grpc::ServerContext* context,
                                                   const zkp_auth::AuthenticationAnswerRequest* request,
                                                   zkp_auth::AuthenticationAnswerResponse* response)
{
    // Implementation of verifying authentication
    std::cout << "Verifying authentication for auth_id: " << request->auth_id() << std::endl;

    const std::string& auth_id = request->auth_id();
    if (auth_id.empty())
    {
        return grpc::Status(grpc::INVALID_ARGUMENT, "INVALID REQUEST.");
    }

    // 1. セッション情報を取得
    std::optional<AuthSession> session_opt;
    session_store_.access(
        [&](const auto& sessions)
        {
            auto it = sessions.find(auth_id);
            if (it != sessions.end())
            {
                session_opt = it->second;
            }
        });

    if (!session_opt)
    {
        return grpc::Status(grpc::NOT_FOUND, "Authentication session not found or expired.");
    }
    const AuthSession& session = *session_opt;

    // 2. ユーザー情報を取得
    std::optional<UserInfo> user_info_opt;
    user_store_.access(
        [&](const auto& users)
        {
            auto it = users.find(session.user);
            if (it != users.end())
            {
                user_info_opt = it->second;
            }
        });

    if (!user_info_opt)
    {
        // セッションは存在したが、対応するユーザーがいない（通常は起こり得ない）
        session_store_.access([&](auto& sessions) { sessions.erase(auth_id); });
        return grpc::Status(grpc::INTERNAL, "User associated with the session not found.");
    }
    const UserInfo& user_info = *user_info_opt;

    // 3. Chaum-Pedersen検証を実行
    const auto constants = get_zkp_constants();
    ChaumPedersen cp(constants.p, constants.q, constants.g, constants.h);

    Commitment commitment = {session.r1, session.r2};
    PublicKeys public_keys = {user_info.y1, user_info.y2};
    Challenge challenge = {session.c};
    Response response_s = {bytes_to_cpp_int(request->s())};

    bool is_verified = cp.verify_proof(commitment, public_keys, challenge, response_s);

    // 4. 検証後、セッションを削除する
    session_store_.access([&](auto& sessions) { sessions.erase(auth_id); });

    if (!is_verified)
    {
        return grpc::Status(grpc::PERMISSION_DENIED, "Authentication failed.");
    }

    // 5. セッションIDを生成して返す
    std::string session_id = generate_auth_id();  // UUIDをセッションIDとして再利用
    response->set_session_id(session_id);

    std::cout << "Authentication successful for user: " << session.user << ", session_id: " << session_id << std::endl;

    return grpc::Status::OK;
}