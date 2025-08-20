#include "neocpp/crypto/ec_key_pair.hpp"
#include "neocpp/crypto/ecdsa_signature.hpp"
#include "neocpp/crypto/hash.hpp"
#include "neocpp/crypto/wif.hpp"
#include "neocpp/utils/hex.hpp"
#include "neocpp/utils/address.hpp"
#include "neocpp/script/script_builder.hpp"
#include "neocpp/exceptions.hpp"
#include <openssl/ec.h>
#include <openssl/ecdsa.h>
#include <openssl/evp.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include <random>
#include <cstring>
#include <algorithm>

namespace neocpp {

// ECPrivateKey implementation

ECPrivateKey ECPrivateKey::generate() {
    EC_KEY* eckey = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (!eckey) {
        throw CryptoException("Failed to create EC_KEY");
    }
    
    if (EC_KEY_generate_key(eckey) != 1) {
        EC_KEY_free(eckey);
        throw CryptoException("Failed to generate EC key");
    }
    
    const BIGNUM* priv_bn = EC_KEY_get0_private_key(eckey);
    std::array<uint8_t, 32> key;
    key.fill(0);
    BN_bn2binpad(priv_bn, key.data(), 32);
    
    EC_KEY_free(eckey);
    return ECPrivateKey(key);
}

ECPrivateKey::ECPrivateKey(const Bytes& bytes) {
    if (bytes.size() != NeoConstants::PRIVATE_KEY_SIZE) {
        throw IllegalArgumentException("Private key must be 32 bytes");
    }
    std::copy(bytes.begin(), bytes.end(), key_.begin());
    
    // Verify the private key is valid using OpenSSL
    EC_KEY* eckey = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (!eckey) {
        throw CryptoException("Failed to create EC_KEY");
    }
    
    BIGNUM* priv_bn = BN_bin2bn(key_.data(), 32, nullptr);
    if (!priv_bn || EC_KEY_set_private_key(eckey, priv_bn) != 1) {
        if (priv_bn) BN_free(priv_bn);
        EC_KEY_free(eckey);
        throw IllegalArgumentException("Invalid private key");
    }
    
    BN_free(priv_bn);
    EC_KEY_free(eckey);
}

ECPrivateKey::ECPrivateKey(const std::array<uint8_t, NeoConstants::PRIVATE_KEY_SIZE>& key) 
    : key_(key) {
    EC_KEY* eckey = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (!eckey) {
        throw CryptoException("Failed to create EC_KEY");
    }
    
    BIGNUM* priv_bn = BN_bin2bn(key_.data(), 32, nullptr);
    if (!priv_bn || EC_KEY_set_private_key(eckey, priv_bn) != 1) {
        if (priv_bn) BN_free(priv_bn);
        EC_KEY_free(eckey);
        throw IllegalArgumentException("Invalid private key");
    }
    
    BN_free(priv_bn);
    EC_KEY_free(eckey);
}

ECPrivateKey::ECPrivateKey(const std::string& hex) 
    : ECPrivateKey(ByteUtils::fromHex(hex)) {
}

Bytes ECPrivateKey::getBytes() const {
    return Bytes(key_.begin(), key_.end());
}

std::string ECPrivateKey::toHex() const {
    return ByteUtils::toHex(getBytes(), false);
}

SharedPtr<ECPublicKey> ECPrivateKey::getPublicKey() const {
    EC_KEY* eckey = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (!eckey) {
        throw CryptoException("Failed to create EC_KEY");
    }
    
    BIGNUM* priv_bn = BN_bin2bn(key_.data(), 32, nullptr);
    if (!priv_bn || EC_KEY_set_private_key(eckey, priv_bn) != 1) {
        if (priv_bn) BN_free(priv_bn);
        EC_KEY_free(eckey);
        throw CryptoException("Failed to set private key");
    }
    BN_free(priv_bn);
    
    // Generate public key from private key
    const EC_GROUP* group = EC_KEY_get0_group(eckey);
    EC_POINT* pub_point = EC_POINT_new(group);
    BN_CTX* ctx = BN_CTX_new();
    
    if (!EC_POINT_mul(group, pub_point, EC_KEY_get0_private_key(eckey), nullptr, nullptr, ctx)) {
        BN_CTX_free(ctx);
        EC_POINT_free(pub_point);
        EC_KEY_free(eckey);
        throw CryptoException("Failed to generate public key");
    }
    
    EC_KEY_set_public_key(eckey, pub_point);
    
    // Get compressed public key (33 bytes)
    size_t len = 33;
    Bytes encoded(len);
    EC_POINT_point2oct(group, pub_point, POINT_CONVERSION_COMPRESSED, encoded.data(), len, ctx);
    
    BN_CTX_free(ctx);
    EC_POINT_free(pub_point);
    EC_KEY_free(eckey);
    return std::make_shared<ECPublicKey>(encoded);
}

SharedPtr<ECDSASignature> ECPrivateKey::sign(const Bytes& message) const {
    EC_KEY* eckey = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (!eckey) {
        throw CryptoException("Failed to create EC_KEY");
    }
    
    BIGNUM* priv_bn = BN_bin2bn(key_.data(), 32, nullptr);
    if (!priv_bn || EC_KEY_set_private_key(eckey, priv_bn) != 1) {
        if (priv_bn) BN_free(priv_bn);
        EC_KEY_free(eckey);
        throw CryptoException("Failed to set private key");
    }
    BN_free(priv_bn);
    
    // Hash the message
    Bytes hash = HashUtils::sha256(message);
    
    // Sign the hash
    ECDSA_SIG* sig = ECDSA_do_sign(hash.data(), hash.size(), eckey);
    if (!sig) {
        EC_KEY_free(eckey);
        throw SignException("Failed to sign message");
    }
    
    // Get r and s components
    const BIGNUM* r;
    const BIGNUM* s;
    ECDSA_SIG_get0(sig, &r, &s);
    
    // Convert to compact format (64 bytes: 32 for r, 32 for s)
    Bytes signature(64);
    BN_bn2binpad(r, signature.data(), 32);
    BN_bn2binpad(s, signature.data() + 32, 32);
    
    ECDSA_SIG_free(sig);
    EC_KEY_free(eckey);
    return std::make_shared<ECDSASignature>(signature);
}

// ECPublicKey implementation

ECPublicKey::ECPublicKey(const ECPoint& point) : point_(point) {
}

ECPublicKey::ECPublicKey(const Bytes& encoded) : point_(encoded) {
}

ECPublicKey::ECPublicKey(const std::string& hex) : point_(hex) {
}

Bytes ECPublicKey::getEncoded() const {
    return point_.getEncodedCompressed();
}

Bytes ECPublicKey::getEncodedUncompressed() const {
    return point_.getEncoded();
}

std::string ECPublicKey::toHex() const {
    return point_.toHex();
}

bool ECPublicKey::verify(const Bytes& message, const ECDSASignature& signature) const {
    EC_KEY* eckey = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (!eckey) {
        return false;
    }
    
    // Set public key
    const EC_GROUP* group = EC_KEY_get0_group(eckey);
    EC_POINT* pub_point = EC_POINT_new(group);
    Bytes encoded = getEncoded();
    
    if (!EC_POINT_oct2point(group, pub_point, encoded.data(), encoded.size(), nullptr)) {
        EC_POINT_free(pub_point);
        EC_KEY_free(eckey);
        return false;
    }
    
    EC_KEY_set_public_key(eckey, pub_point);
    EC_POINT_free(pub_point);
    
    // Parse signature
    Bytes sigBytes = signature.getBytes();
    ECDSA_SIG* sig = ECDSA_SIG_new();
    if (!sig) {
        EC_KEY_free(eckey);
        return false;
    }
    
    BIGNUM* r = BN_bin2bn(sigBytes.data(), 32, nullptr);
    BIGNUM* s = BN_bin2bn(sigBytes.data() + 32, 32, nullptr);
    ECDSA_SIG_set0(sig, r, s);
    
    // Hash the message
    Bytes hash = HashUtils::sha256(message);
    
    // Verify
    int valid = ECDSA_do_verify(hash.data(), hash.size(), sig, eckey);
    
    ECDSA_SIG_free(sig);
    EC_KEY_free(eckey);
    return valid == 1;
}

Bytes ECPublicKey::getScriptHash() const {
    Bytes script = ScriptBuilder::buildVerificationScript(std::make_shared<ECPublicKey>(*this));
    // Hash the script and return as little-endian
    Bytes hash = HashUtils::sha256ThenRipemd160(script);
    std::reverse(hash.begin(), hash.end());
    return hash;
}

std::string ECPublicKey::getAddress() const {
    return AddressUtils::scriptHashToAddress(getScriptHash());
}

bool ECPublicKey::operator==(const ECPublicKey& other) const {
    return point_ == other.point_;
}

bool ECPublicKey::operator!=(const ECPublicKey& other) const {
    return !(*this == other);
}

bool ECPublicKey::operator<(const ECPublicKey& other) const {
    return point_ < other.point_;
}

// ECKeyPair implementation

ECKeyPair ECKeyPair::generate() {
    return ECKeyPair(std::make_shared<ECPrivateKey>(ECPrivateKey::generate()));
}

ECKeyPair::ECKeyPair(const SharedPtr<ECPrivateKey>& privateKey) 
    : privateKey_(privateKey), publicKey_(privateKey->getPublicKey()) {
}

ECKeyPair::ECKeyPair(const Bytes& privateKeyBytes) 
    : ECKeyPair(std::make_shared<ECPrivateKey>(privateKeyBytes)) {
}

ECKeyPair ECKeyPair::fromWIF(const std::string& wif) {
    Bytes privateKeyBytes = WIF::decode(wif);
    return ECKeyPair(privateKeyBytes);
}

std::string ECKeyPair::exportAsWIF() const {
    return WIF::encode(privateKey_->getBytes());
}

SharedPtr<ECDSASignature> ECKeyPair::sign(const Bytes& message) const {
    return privateKey_->sign(message);
}

Bytes ECKeyPair::getScriptHash() const {
    return publicKey_->getScriptHash();
}

std::string ECKeyPair::getAddress() const {
    return publicKey_->getAddress();
}

} // namespace neocpp