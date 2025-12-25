#include <boost/multiprecision/cpp_int.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <utility>

using namespace boost::multiprecision;

//  証明者と検証者の間で交換される公開鍵
struct PublicKeys {
    cpp_int y1;
    cpp_int y2;
};

// 証明者が生成するコミットメント
struct Commitment {
    cpp_int r1;
    cpp_int r2;
};

// 検証者が生成するチャレンジ
struct Challenge {
    cpp_int c;
};

// 証明者が生成するレスポンス
struct Response {
    cpp_int s;
};

/**
 * @fn
 * @brief 1 から upper_bound - 1 までの範囲で乱数を生成する
 * @param upper_bound 乱数生成の上限
 * @return 生成された乱数
 */
cpp_int generate_random(const cpp_int& upper_bound) {
    // 現在時刻をシードとして使い、メルセンヌ・ツイスター乱数生成器を初期化する
    static boost::random::mt19937 gen{static_cast<std::uint32_t>(std::time(nullptr))};
    boost::random::uniform_int_distribution<cpp_int> dist(1, upper_bound - 1);
    return dist(gen);
}

class ChaumPedersen {
    public:
        // 公開パラメータ
        const cpp_int p, q, g, h;

        /**
         * @fn
         * @brief コンストラクタ
         * @param p 素数
         * @param q p-1の素因数
         * @param g 位数qの生成子
         * @param h 位数qの別の生成子
         */
        ChaumPedersen(cpp_int p, cpp_int q, cpp_int g, cpp_int h)
            : p(std::move(p)), q(std::move(q)), g(std::move(g)), h(std::move(h)) {}

        
        /**
         * @fn
         * @brief 公開鍵 y1(g^x mod p), y2(h^x mod p) を計算する
         * @param x 秘密鍵
         * @return 計算された公開鍵 {y1, y2}
         */
        PublicKeys calculate_public_keys(const cpp_int& x) const {
            return {powm(g, x, p), powm(h, x, p)};
        }

        /**
         * @fn
         * @brief コミットメント r1(g^k mod p), r2(h^k mod p) を計算する
         * @param k 一時的な乱数 (Nonce)
         * @return 計算されたコミットメント {r1, r2}
         */
        Commitment create_commitment(const cpp_int& k) const {
            return {powm(g, k, p), powm(h, k, p)};
        }

        /**
         * @fn
         * @brief レスポンス s を計算する: s = k - c * x (mod q)
         * @param k 証明者が生成した一時的な乱数 (Nonce)
         * @param c 検証者から送られたチャレンジ
         * @param x 証明者の秘密鍵
         * @return 計算されたレスポンス {s}
         */
        Response solve_response(const cpp_int& k, const Challenge& c, const cpp_int& x) const {
            cpp_int cx = (c.c * x) % q;
            if (k >= cx) {
                return {(k - cx) % q};
            }
            return {q - (cx -k) % q};
        }

        /**
         * @fn
         * @brief Chaum-Pedersen プロトコルの検証を行う
         * @note  g^s * y1^c == r1 (mod p) かつ h^s * y2^c == r2 (mod p) を確認する
         * @param commitment コミットメント {r1, r2}
         * @param public_keys 公開鍵 {y1, y2}
         * @param challenge チャレンジ {c}
         * @param response レスポンス {s}
         * @return 検証結果 (true: 成功, false: 失敗)
         */
        bool verify_proof(const Commitment& commitment, const PublicKeys& public_keys, const Challenge& challenge, const Response& response) const {
            cpp_int left1 = powm(g, response.s, p) * powm(public_keys.y1, challenge.c, p) % p;
            cpp_int left2 = powm(h, response.s, p) * powm(public_keys.y2, challenge.c, p) % p;

            return left1 == commitment.r1 && left2 == commitment.r2;
        }
    };

