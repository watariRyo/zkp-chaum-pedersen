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

TEST(ChaumPedersenTest, VerifyProof1024bitConstance) {
    //  https://datatracker.ietf.org/doc/html/rfc5114
    //  1024-bit MODP Group with 160-bit Prime Order Subgroup
    // 1. Setup
    cpp_int p("0xB10B8F96A080E01DDE92DE5EAE5D54EC52C99FBCFB06A3C6"
              "9A6A9DCA52D23B616073E28675A23D189838EF1E2EE652C0"
              "13ECB4AEA906112324975C3CD49B83BFACCBDD7D90C4BD70"
              "98488E9C219A73724EFFD6FAE5644738FAA31A4FF55BCCC0"
              "A151AF5F0DC8B4BD45BF37DF365C1A65E68CFDA76D4DA708"
              "DF1FB2BC2E4A4371");

    cpp_int q("0xF518AA8781A8DF278ABA4E7D64B7CB9D49462353");

    cpp_int g("0xA4D1CBD5C3FD34126765A442EFB99905F8104DD258AC507F"
              "D6406CFF14266D31266FEA1E5C41564B777E690F5504F213"
              "160217B4B01B886A5E91547F9E2749F4D7FBD7D3B9A92EE1"
              "909D0D2263F80A76A6A24C087A091F531DBF0A0169B6A28A"
              "D662A4D18E73AFA32D779D5918D08BC8858F4DCEF97C2A24"
              "855E6EEB22B3B2E5");

    // h は g とは異なる位数qの生成元でないといけない。
    // g^i mod p で求められる
    cpp_int h = powm(g, generate_random(q), p);

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