#ifndef AUTH_SERVICE_IMPL_HPP
#define AUTH_SERVICE_IMPL_HPP

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <mutex>
#include <unordered_map>
#include <grpcpp/grpcpp.h>
#include "zkp_auth.grpc.pb.h"

using namespace boost::multiprecision;

class AuthServiceImpl final : public zkp_auth::Auth::Service {
public:
    /**
     * @fn
     * @brief ユーザー登録を行う。
     * @param context gRPCのサーバコンテキスト
     * @param request 登録リクエスト（ユーザー名、y1、y2を含む）
     * @param response 登録レスポンス（成功/失敗を含む）
     */
    grpc::Status Register(grpc::ServerContext* context,
                          const zkp_auth::RegisterRequest* request,
                          zkp_auth::RegisterResponse* response) override;
    /**
     * @fn
     * @brief 認証チャレンジを生成する。
     * @param context gRPCのサーバコンテキスト
     * @param request チャレンジリクエスト(ユーザー名、r1、r2を含む)
     * @param response チャレンジレスポンス（auth_id、c)
     */
    grpc::Status CreateAuthenticationChallenge(
        grpc::ServerContext* context,
        const zkp_auth::AuthenticationChallengeRequest* request,
        zkp_auth::AuthenticationChallengeResponse* response) override;

    /**
     * @fn
     * @brief 認証回答を検証する。
     * @param context gRPCのサーバコンテキスト
     * @param request 認証回答リクエスト（auth_id、sを含む）
     * @param response 認証回答レスポンス（成功/失敗)
     */
    grpc::Status VerifyAuthentication(
        grpc::ServerContext* context,
        const zkp_auth::AuthenticationAnswerRequest* request,
        zkp_auth::AuthenticationAnswerResponse* response) override;
private:
    /**
     * @fn
     * @brief バイト列をcpp_intに変換する。
     * @param s 変換するバイト列
     * @return 変換後のcpp_int
     */
    cpp_int bytes_to_cpp_int(const std::string& s);

    /**
     * @fn
     * @brief cpp_intをバイト列に変換する。
     * @param n 変換するcpp_int
     * @return 変換後のバイト列
     */
    std::string cpp_int_to_bytes(const cpp_int& n);

    /**
     * @fn
     * @brief 一意な認証IDを生成する。
     * @return 生成された認証ID
     */
    std::string generate_auth_id() {
        boost::uuids::uuid uuid = boost::uuids::random_generator()();
        return boost::uuids::to_string(uuid);
    }

    struct UserInfo {
        // registration
        std::string name;
        cpp_int y1;
        cpp_int y2;
        // // authorization
        // cpp_int r1;
        // cpp_int r2;
        // // verification
        // cpp_int c;
        // cpp_int s;
        // std::string session_id;
    };

    struct AuthSession {
        std::string user;
        // authorization
        cpp_int r1;
        cpp_int r2;
        // verification
        cpp_int c;
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

    struct SessionStore {
        std::mutex mutex;
        std::unordered_map<std::string, AuthSession> sessions;

        template<typename Func>
        auto access(Func f) {
            std::lock_guard<std::mutex> lock(mutex);
            return f(sessions);
        }
    };

    // NOTE: 実際のgRPCでは、std::mutex + std::unordered_mapではなく、
    // tbb::concurrent_hash_mapを使ったほうが良い。
    // ロック粒度が細かく（コンテナ内部で並列化される）、パフォーマンス面でスレッドが増えてもロック競合が起きにくい。

    UserStore user_store_;
    SessionStore session_store_;
};

#endif // AUTH_SERVICE_IMPL_HPP