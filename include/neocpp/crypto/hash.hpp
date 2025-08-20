#pragma once

#include <string>
#include "neocpp/types/types.hpp"

namespace neocpp {

/// Hash utilities for cryptographic operations
class HashUtils {
public:
    /// Compute SHA256 hash
    /// @param data The data to hash
    /// @return The SHA256 hash (32 bytes)
    static Bytes sha256(const Bytes& data);
    
    /// Compute double SHA256 hash (SHA256(SHA256(data)))
    /// @param data The data to hash
    /// @return The double SHA256 hash (32 bytes)
    static Bytes doubleSha256(const Bytes& data);
    
    /// Compute RIPEMD160 hash
    /// @param data The data to hash
    /// @return The RIPEMD160 hash (20 bytes)
    static Bytes ripemd160(const Bytes& data);
    
    /// Compute SHA256 then RIPEMD160 hash
    /// @param data The data to hash
    /// @return The hash (20 bytes)
    static Bytes sha256ThenRipemd160(const Bytes& data);
    
    /// Compute Keccak256 hash (used in Ethereum)
    /// @param data The data to hash
    /// @return The Keccak256 hash (32 bytes)
    static Bytes keccak256(const Bytes& data);
    
    /// Compute HMAC-SHA256
    /// @param key The HMAC key
    /// @param data The data to authenticate
    /// @return The HMAC-SHA256 (32 bytes)
    static Bytes hmacSha256(const Bytes& key, const Bytes& data);
};

} // namespace neocpp