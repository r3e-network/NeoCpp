#include "neocpp/crypto/ecdsa_signature.hpp"
#include "neocpp/types/types.hpp"
#include "neocpp/exceptions.hpp"
#include <openssl/ecdsa.h>
#include <openssl/bn.h>
#include <cstring>

namespace neocpp {

ECDSASignature::ECDSASignature(const Bytes& signature) {
    if (signature.size() != NeoConstants::SIGNATURE_SIZE) {
        throw IllegalArgumentException("Signature must be 64 bytes");
    }
    std::copy(signature.begin(), signature.end(), signature_.begin());
}

ECDSASignature::ECDSASignature(const std::array<uint8_t, NeoConstants::SIGNATURE_SIZE>& signature) 
    : signature_(signature) {
}

ECDSASignature::ECDSASignature(const Bytes& r, const Bytes& s) {
    if (r.size() != 32 || s.size() != 32) {
        throw IllegalArgumentException("R and S must each be 32 bytes");
    }
    std::copy(r.begin(), r.end(), signature_.begin());
    std::copy(s.begin(), s.end(), signature_.begin() + 32);
}

ECDSASignature::ECDSASignature(const std::string& hex) 
    : ECDSASignature(ByteUtils::fromHex(hex)) {
}

Bytes ECDSASignature::getBytes() const {
    return Bytes(signature_.begin(), signature_.end());
}

Bytes ECDSASignature::getR() const {
    return Bytes(signature_.begin(), signature_.begin() + 32);
}

Bytes ECDSASignature::getS() const {
    return Bytes(signature_.begin() + 32, signature_.end());
}

std::string ECDSASignature::toHex() const {
    return ByteUtils::toHex(getBytes(), false);
}

Bytes ECDSASignature::toDER() const {
    ECDSA_SIG* sig = ECDSA_SIG_new();
    if (!sig) {
        throw CryptoException("Failed to create ECDSA_SIG");
    }
    
    // Set r and s values
    BIGNUM* r = BN_bin2bn(signature_.data(), 32, nullptr);
    BIGNUM* s = BN_bin2bn(signature_.data() + 32, 32, nullptr);
    ECDSA_SIG_set0(sig, r, s);
    
    // Serialize to DER
    unsigned char* der = nullptr;
    int derLen = i2d_ECDSA_SIG(sig, &der);
    if (derLen <= 0) {
        ECDSA_SIG_free(sig);
        throw CryptoException("Failed to serialize signature to DER");
    }
    
    Bytes result(der, der + derLen);
    OPENSSL_free(der);
    ECDSA_SIG_free(sig);
    return result;
}

ECDSASignature ECDSASignature::fromDER(const Bytes& der) {
    const unsigned char* p = der.data();
    ECDSA_SIG* sig = d2i_ECDSA_SIG(nullptr, &p, der.size());
    if (!sig) {
        throw CryptoException("Failed to parse DER signature");
    }
    
    // Get r and s values
    const BIGNUM* r;
    const BIGNUM* s;
    ECDSA_SIG_get0(sig, &r, &s);
    
    // Convert to compact format
    Bytes compact(64);
    BN_bn2binpad(r, compact.data(), 32);
    BN_bn2binpad(s, compact.data() + 32, 32);
    
    ECDSA_SIG_free(sig);
    return ECDSASignature(compact);
}

bool ECDSASignature::isCanonical() const {
    // In canonical form, S must be <= half the curve order
    // For secp256k1, the half order is:
    // 0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF5D576E7357A4501DDFE92F46681B20A0
    
    Bytes s = getS();
    
    // Half curve order for secp256k1
    static const uint8_t HALF_CURVE_ORDER[32] = {
        0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0x5D, 0x57, 0x6E, 0x73, 0x57, 0xA4, 0x50, 0x1D,
        0xDF, 0xE9, 0x2F, 0x46, 0x68, 0x1B, 0x20, 0xA0
    };
    
    // Compare S with half curve order
    for (size_t i = 0; i < 32; ++i) {
        if (s[i] < HALF_CURVE_ORDER[i]) {
            return true;  // S is definitely less than half order
        }
        if (s[i] > HALF_CURVE_ORDER[i]) {
            return false; // S is definitely greater than half order
        }
        // If equal, continue to next byte
    }
    
    // S equals half order, which is still canonical
    return true;
}

ECDSASignature ECDSASignature::makeCanonical() const {
    if (isCanonical()) {
        return *this;
    }
    
    // If S is not canonical, we need to negate it modulo the curve order
    // S' = n - S where n is the curve order
    
    // Full curve order for secp256k1
    static const uint8_t CURVE_ORDER[32] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
        0xBA, 0xAE, 0xDC, 0xE6, 0xAF, 0x48, 0xA0, 0x3B,
        0xBF, 0xD2, 0x5E, 0x8C, 0xD0, 0x36, 0x41, 0x41
    };
    
    Bytes r = getR();
    Bytes s = getS();
    
    // Compute n - s using OpenSSL's BIGNUM
    BIGNUM* bn_s = BN_bin2bn(s.data(), 32, nullptr);
    BIGNUM* bn_order = BN_bin2bn(CURVE_ORDER, 32, nullptr);
    BIGNUM* bn_result = BN_new();
    
    BN_sub(bn_result, bn_order, bn_s); // result = order - s
    
    // Convert back to bytes
    Bytes new_s(32);
    BN_bn2binpad(bn_result, new_s.data(), 32);
    
    BN_free(bn_s);
    BN_free(bn_order);
    BN_free(bn_result);
    
    // Create new signature with canonical S
    return ECDSASignature(r, new_s);
}

bool ECDSASignature::operator==(const ECDSASignature& other) const {
    return signature_ == other.signature_;
}

bool ECDSASignature::operator!=(const ECDSASignature& other) const {
    return !(*this == other);
}

} // namespace neocpp