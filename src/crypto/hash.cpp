#include "neocpp/crypto/hash.hpp"
#include <openssl/sha.h>
#include <openssl/ripemd.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <cstring>

namespace neocpp {

Bytes HashUtils::sha256(const Bytes& data) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    Bytes result(SHA256_DIGEST_LENGTH);
    unsigned int len = SHA256_DIGEST_LENGTH;
    
    EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr);
    EVP_DigestUpdate(ctx, data.data(), data.size());
    EVP_DigestFinal_ex(ctx, result.data(), &len);
    
    EVP_MD_CTX_free(ctx);
    return result;
}

Bytes HashUtils::doubleSha256(const Bytes& data) {
    return sha256(sha256(data));
}

Bytes HashUtils::ripemd160(const Bytes& data) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    Bytes result(20); // RIPEMD160 produces 20 bytes
    unsigned int len = 20;
    
    EVP_DigestInit_ex(ctx, EVP_ripemd160(), nullptr);
    EVP_DigestUpdate(ctx, data.data(), data.size());
    EVP_DigestFinal_ex(ctx, result.data(), &len);
    
    EVP_MD_CTX_free(ctx);
    return result;
}

Bytes HashUtils::sha256ThenRipemd160(const Bytes& data) {
    return ripemd160(sha256(data));
}

Bytes HashUtils::keccak256(const Bytes& data) {
    Bytes result(32);
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    const EVP_MD* md = EVP_sha3_256();
    
    EVP_DigestInit_ex(ctx, md, nullptr);
    EVP_DigestUpdate(ctx, data.data(), data.size());
    unsigned int len = 32;
    EVP_DigestFinal_ex(ctx, result.data(), &len);
    
    EVP_MD_CTX_free(ctx);
    return result;
}

Bytes HashUtils::hmacSha256(const Bytes& key, const Bytes& data) {
    Bytes result(SHA256_DIGEST_LENGTH);
    unsigned int len = SHA256_DIGEST_LENGTH;
    
    HMAC(EVP_sha256(), key.data(), key.size(), 
         data.data(), data.size(), result.data(), &len);
    
    return result;
}

} // namespace neocpp