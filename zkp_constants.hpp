#ifndef ZKP_CONSTANTS_HPP
#define ZKP_CONSTANTS_HPP

#include <boost/multiprecision/cpp_int.hpp>

using namespace boost::multiprecision;

// Chaum-Pedersenプロトコルで使用する公開パラメータを保持する構造体
struct ZKPConstants
{
    cpp_int p;
    cpp_int q;
    cpp_int g;
    cpp_int h;
};

/**
 * @fn
 * @brief RFC5114で定義された1024-bit MODP Groupの定数を取得
 * @note hはgとは異なる位数qの生成元。ここでは g^2 mod p で計算する。
 * @return ZKPConstants構造体
 */
inline ZKPConstants get_zkp_constants()
{
    cpp_int p(
        "0xB10B8F96A080E01DDE92DE5EAE5D54EC52C99FBCFB06A3C6"
        "9A6A9DCA52D23B616073E28675A23D189838EF1E2EE652C0"
        "13ECB4AEA906112324975C3CD49B83BFACCBDD7D90C4BD70"
        "98488E9C219A73724EFFD6FAE5644738FAA31A4FF55BCCC0"
        "A151AF5F0DC8B4BD45BF37DF365C1A65E68CFDA76D4DA708"
        "DF1FB2BC2E4A4371");

    cpp_int q("0xF518AA8781A8DF278ABA4E7D64B7CB9D49462353");

    cpp_int g(
        "0xA4D1CBD5C3FD34126765A442EFB99905F8104DD258AC507F"
        "D6406CFF14266D31266FEA1E5C41564B777E690F5504F213"
        "160217B4B01B886A5E91547F9E2749F4D7FBD7D3B9A92EE1"
        "909D0D2263F80A76A6A24C087A091F531DBF0A0169B6A28A"
        "D662A4D18E73AFA32D779D5918D08BC8858F4DCEF97C2A24"
        "855E6EEB22B3B2E5");

    // h は g とは異なる位数qの生成元である必要がある。
    // g^2 mod p で決定論的に生成する。
    cpp_int h = powm(g, 2, p);

    return {p, q, g, h};
}

#endif  // ZKP_CONSTANTS_HPP