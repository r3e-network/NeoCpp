#include "neocpp/crypto/ec_point.hpp"
#include "neocpp/types/types.hpp"
#include "neocpp/serialization/binary_writer.hpp"
#include "neocpp/serialization/binary_reader.hpp"
#include "neocpp/exceptions.hpp"
#include <openssl/ec.h>
#include <openssl/bn.h>
#include <openssl/obj_mac.h>
#include <cstring>

namespace neocpp {

// Static member initialization
const ECPoint ECPoint::INFINITY_POINT = ECPoint();

ECPoint::ECPoint(const Bytes& encoded) : encoded_(encoded), is_infinity_(false) {
    if (encoded.empty()) {
        is_infinity_ = true;
        return;
    }
    
    if (encoded.size() != 33 && encoded.size() != 65) {
        throw IllegalArgumentException("Invalid EC point encoding");
    }
    
    // Validate the point on curve
    if (!isValid()) {
        throw IllegalArgumentException("EC point not on curve");
    }
}

ECPoint::ECPoint(const std::string& hex) : ECPoint(ByteUtils::fromHex(hex)) {
}

ECPoint::ECPoint() : is_infinity_(true) {
    encoded_.push_back(0x00);  // Infinity point encoding
}

Bytes ECPoint::getEncodedCompressed() const {
    if (is_infinity_) {
        return Bytes{0x00};
    }
    
    if (encoded_.size() == 33) {
        return encoded_;
    }
    
    // Convert uncompressed to compressed using OpenSSL
    EC_KEY* eckey = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (!eckey) {
        throw CryptoException("Failed to create EC_KEY");
    }
    
    const EC_GROUP* group = EC_KEY_get0_group(eckey);
    EC_POINT* point = EC_POINT_new(group);
    BN_CTX* ctx = BN_CTX_new();
    
    if (EC_POINT_oct2point(group, point, encoded_.data(), encoded_.size(), ctx) != 1) {
        BN_CTX_free(ctx);
        EC_POINT_free(point);
        EC_KEY_free(eckey);
        throw CryptoException("Failed to parse public key");
    }
    
    Bytes compressed(33);
    size_t len = EC_POINT_point2oct(group, point, POINT_CONVERSION_COMPRESSED, 
                                    compressed.data(), 33, ctx);
    
    BN_CTX_free(ctx);
    EC_POINT_free(point);
    EC_KEY_free(eckey);
    
    if (len != 33) {
        throw CryptoException("Failed to compress public key");
    }
    
    return compressed;
}

Bytes ECPoint::getX() const {
    if (is_infinity_) {
        return Bytes();
    }
    
    if (encoded_.size() == 33 || encoded_.size() == 65) {
        // X coordinate starts at byte 1 (after the prefix byte)
        return Bytes(encoded_.begin() + 1, encoded_.begin() + 33);
    }
    
    return Bytes();
}

Bytes ECPoint::getY() const {
    if (is_infinity_) {
        return Bytes();
    }
    
    if (encoded_.size() == 33) {
        // For compressed points, decompress to get Y coordinate
        EC_KEY* eckey = EC_KEY_new_by_curve_name(NID_secp256k1);
        if (!eckey) {
            return Bytes();
        }
        
        const EC_GROUP* group = EC_KEY_get0_group(eckey);
        EC_POINT* point = EC_POINT_new(group);
        BN_CTX* ctx = BN_CTX_new();
        
        if (!EC_POINT_oct2point(group, point, encoded_.data(), encoded_.size(), ctx)) {
            BN_CTX_free(ctx);
            EC_POINT_free(point);
            EC_KEY_free(eckey);
            return Bytes();
        }
        
        // Get uncompressed form to extract Y
        Bytes uncompressed(65);
        size_t len = EC_POINT_point2oct(group, point, POINT_CONVERSION_UNCOMPRESSED,
                                        uncompressed.data(), 65, ctx);
        
        BN_CTX_free(ctx);
        EC_POINT_free(point);
        EC_KEY_free(eckey);
        
        if (len == 65) {
            return Bytes(uncompressed.begin() + 33, uncompressed.begin() + 65);
        }
        return Bytes();
    } else if (encoded_.size() == 65) {
        // Y coordinate starts at byte 33 for uncompressed points
        return Bytes(encoded_.begin() + 33, encoded_.begin() + 65);
    }
    
    return Bytes();
}

std::string ECPoint::toHex() const {
    return ByteUtils::toHex(encoded_, false);
}

ECPoint ECPoint::fromHex(const std::string& hex) {
    return ECPoint(hex);
}

bool ECPoint::isValid() const {
    if (is_infinity_) {
        return true;
    }
    
    EC_KEY* eckey = EC_KEY_new_by_curve_name(NID_secp256k1);
    if (!eckey) {
        return false;
    }
    
    const EC_GROUP* group = EC_KEY_get0_group(eckey);
    EC_POINT* point = EC_POINT_new(group);
    BN_CTX* ctx = BN_CTX_new();
    
    bool valid = false;
    if (EC_POINT_oct2point(group, point, encoded_.data(), encoded_.size(), ctx) == 1) {
        // Also verify the point is on the curve
        valid = EC_POINT_is_on_curve(group, point, ctx) == 1;
    }
    
    BN_CTX_free(ctx);
    EC_POINT_free(point);
    EC_KEY_free(eckey);
    
    return valid;
}

size_t ECPoint::getSize() const {
    return encoded_.size();
}

void ECPoint::serialize(BinaryWriter& writer) const {
    writer.writeBytes(encoded_);
}

ECPoint ECPoint::deserialize(BinaryReader& reader) {
    uint8_t firstByte = reader.readByte();
    
    if (firstByte == 0x00) {
        return ECPoint::INFINITY_POINT;
    }
    
    Bytes encoded;
    encoded.push_back(firstByte);
    
    if (firstByte == 0x02 || firstByte == 0x03) {
        // Compressed
        Bytes rest = reader.readBytes(32);
        encoded.insert(encoded.end(), rest.begin(), rest.end());
    } else if (firstByte == 0x04) {
        // Uncompressed
        Bytes rest = reader.readBytes(64);
        encoded.insert(encoded.end(), rest.begin(), rest.end());
    } else {
        throw DeserializationException("Invalid EC point prefix");
    }
    
    return ECPoint(encoded);
}

bool ECPoint::operator==(const ECPoint& other) const {
    return is_infinity_ == other.is_infinity_ && encoded_ == other.encoded_;
}

bool ECPoint::operator!=(const ECPoint& other) const {
    return !(*this == other);
}

bool ECPoint::operator<(const ECPoint& other) const {
    if (is_infinity_ != other.is_infinity_) {
        return is_infinity_;
    }
    return encoded_ < other.encoded_;
}

} // namespace neocpp