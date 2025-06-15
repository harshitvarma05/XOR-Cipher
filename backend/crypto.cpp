#include "Crypto.h"
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <chrono>
#include <fstream>
#include <vector>
#include <cstdlib>

static std::vector<unsigned char> readFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    return { std::istreambuf_iterator<char>(in), {} };
}

static void writeFile(const std::string& path, const std::vector<unsigned char>& data) {
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(data.data()), data.size());
}

static void handleOpenSSLErr() {
    ERR_print_errors_fp(stderr);
    std::exit(1);
}

static RSA* loadRSA(const std::string& path, bool isPublic) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) { perror("fopen"); std::exit(1); }
    RSA* r = isPublic
      ? PEM_read_RSA_PUBKEY(f, nullptr, nullptr, nullptr)
      : PEM_read_RSAPrivateKey(f, nullptr, nullptr, nullptr);
    fclose(f);
    if (!r) handleOpenSSLErr();
    return r;
}

static std::vector<unsigned char> aesEncrypt(
    const std::vector<unsigned char>& pt,
    const std::vector<unsigned char>& key,
    const std::vector<unsigned char>& iv
) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data());

    std::vector<unsigned char> ct(pt.size() + EVP_CIPHER_block_size(EVP_aes_256_cbc()));
    int len1=0,len2=0;
    EVP_EncryptUpdate(ctx, ct.data(), &len1, pt.data(), pt.size());
    EVP_EncryptFinal_ex(ctx, ct.data()+len1, &len2);
    ct.resize(len1+len2);
    EVP_CIPHER_CTX_free(ctx);
    return ct;
}

static std::vector<unsigned char> aesDecrypt(
    const std::vector<unsigned char>& ct,
    const std::vector<unsigned char>& key,
    const std::vector<unsigned char>& iv
) {
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key.data(), iv.data());

    std::vector<unsigned char> pt(ct.size());
    int len1=0,len2=0;
    EVP_DecryptUpdate(ctx, pt.data(), &len1, ct.data(), ct.size());
    EVP_DecryptFinal_ex(ctx, pt.data()+len1, &len2);
    pt.resize(len1+len2);
    EVP_CIPHER_CTX_free(ctx);
    return pt;
}

static std::vector<unsigned char> rsaWrapKey(RSA* rsaPub,
    const std::vector<unsigned char>& key)
{
    int sz = RSA_size(rsaPub);
    std::vector<unsigned char> out(sz);
    int len = RSA_public_encrypt(
      key.size(), key.data(), out.data(), rsaPub, RSA_PKCS1_OAEP_PADDING
    );
    if (len < 0) handleOpenSSLErr();
    out.resize(len);
    return out;
}

static std::vector<unsigned char> rsaUnwrapKey(RSA* rsaPriv,
    const std::vector<unsigned char>& wrapped)
{
    int sz = RSA_size(rsaPriv);
    std::vector<unsigned char> out(sz);
    int len = RSA_private_decrypt(
      wrapped.size(), wrapped.data(), out.data(), rsaPriv, RSA_PKCS1_OAEP_PADDING
    );
    if (len < 0) handleOpenSSLErr();
    out.resize(len);
    return out;
}


double hybridEncrypt(
    const std::string& inPath,
    const std::string& outPath,
    const std::string& pubKeyPath
) {
    auto pt = readFile(inPath);
    RSA* rsaPub = loadRSA(pubKeyPath, true);

   
    std::vector<unsigned char> aesKey(32), iv(16);
    RAND_bytes(aesKey.data(), aesKey.size());
    RAND_bytes(iv.data(),    iv.size());

   
    auto t0 = std::chrono::high_resolution_clock::now();
    auto ct = aesEncrypt(pt, aesKey, iv);
    auto wrappedKey = rsaWrapKey(rsaPub, aesKey);
    auto t1 = std::chrono::high_resolution_clock::now();

   
    uint32_t keyLen = static_cast<uint32_t>(wrappedKey.size());
    std::vector<unsigned char> out;
    out.reserve(4 + wrappedKey.size() + iv.size() + ct.size());
    out.push_back((keyLen >> 24) & 0xFF);
    out.push_back((keyLen >> 16) & 0xFF);
    out.push_back((keyLen >> 8 ) & 0xFF);
    out.push_back((keyLen      ) & 0xFF);
    out.insert(out.end(), wrappedKey.begin(), wrappedKey.end());
    out.insert(out.end(), iv.begin(), iv.end());
    out.insert(out.end(), ct.begin(), ct.end());
    writeFile(outPath, out);

    RSA_free(rsaPub);
    return std::chrono::duration<double,std::milli>(t1 - t0).count();
}

double hybridDecrypt(
    const std::string& inPath,
    const std::string& outPath,
    const std::string& privKeyPath
) {
    auto data = readFile(inPath);
    size_t pos = 0;

   
    uint32_t keyLen =
        (uint32_t(data[pos]) << 24) |
        (uint32_t(data[pos+1]) << 16) |
        (uint32_t(data[pos+2]) << 8) |
         uint32_t(data[pos+3]);
    pos += 4;

   
    std::vector<unsigned char> wrappedKey(
      data.begin()+pos, data.begin()+pos+keyLen
    );
    pos += keyLen;

   
    std::vector<unsigned char> iv(data.begin()+pos, data.begin()+pos+16);
    pos += 16;

   
    std::vector<unsigned char> ct(data.begin()+pos, data.end());

    RSA* rsaPriv = loadRSA(privKeyPath, false);

   
    auto t0 = std::chrono::high_resolution_clock::now();
    auto aesKey = rsaUnwrapKey(rsaPriv, wrappedKey);
    auto pt     = aesDecrypt(ct, aesKey, iv);
    auto t1 = std::chrono::high_resolution_clock::now();

    writeFile(outPath, pt);
    RSA_free(rsaPriv);
    return std::chrono::duration<double,std::milli>(t1 - t0).count();
}
