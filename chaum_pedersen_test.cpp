#include <gtest/gtest.h>
#include "chaum_pedersen.hpp" 

TEST(ChaumPedersenTest, SolveResponse) {
    ChaumPedersen cp(0, 71, 0, 0); // qのみ使用
    cpp_int k = 10;
    Challenge c = {2};
    cpp_int x = 30;

    // s = 10 - 2 * 30 = -50
    // -50 mod 71 は 21
    Response res = cp.solve_response(k, c, x);
    EXPECT_EQ(res.s, 21);
}

TEST(ChaumPedersenTest, VerifyProofSuccessful) {
    // 1. Setup
    cpp_int p = 23;
    cpp_int q = 11;
    cpp_int g = 4;
    cpp_int h = 9;
    ChaumPedersen cp(p, q, g, h);

    // 2. Prover's secret
    cpp_int x = generate_random(q);

    // 3. y1, y2の計算
    PublicKeys public_keys = cp.calculate_public_keys(x);

    // 4. ランダムのnonce k を生成し、コミットメント r1, r2 を計算する
    cpp_int k = generate_random(q);
    Commitment commitment = cp.create_commitment(k);

    // 5. Verifier は challenge c を送る
    Challenge challenge = {generate_random(q)};

    // 6. Proverはレスポンスを計算
    Response response = cp.solve_response(k, challenge, x);

    // 7. 検証
    EXPECT_TRUE(cp.verify_proof(commitment, public_keys, challenge, response));

    // 失敗ケース: 不正なレスポンス s' = s + 1
    Response invalid_response = {(response.s + 1) % q};
    EXPECT_FALSE(cp.verify_proof(commitment, public_keys, challenge, invalid_response));
}