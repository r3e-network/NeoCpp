#pragma once

#include <memory>
#include "neocpp/types/types.hpp"

namespace neocpp {

// Forward declarations
class ECKeyPair;
class ECPrivateKey;
class ECPublicKey;
class ECDSASignature;

/// Signing utilities for Neo
class Sign {
public:
    /// Sign a message with a private key
    /// @param message The message to sign
    /// @param privateKey The private key
    /// @return The signature
    static SharedPtr<ECDSASignature> signMessage(const Bytes& message, const SharedPtr<ECPrivateKey>& privateKey);
    
    /// Sign a message with a key pair
    /// @param message The message to sign
    /// @param keyPair The key pair
    /// @return The signature
    static SharedPtr<ECDSASignature> signMessage(const Bytes& message, const SharedPtr<ECKeyPair>& keyPair);
    
    /// Verify a signature
    /// @param message The original message
    /// @param signature The signature to verify
    /// @param publicKey The public key
    /// @return True if signature is valid
    static bool verifySignature(const Bytes& message, const SharedPtr<ECDSASignature>& signature, 
                                const SharedPtr<ECPublicKey>& publicKey);
    
    /// Sign a hash directly (no additional hashing)
    /// @param hash The hash to sign (32 bytes)
    /// @param privateKey The private key
    /// @return The signature
    static SharedPtr<ECDSASignature> signHash(const Bytes& hash, const SharedPtr<ECPrivateKey>& privateKey);
    
    /// Create a signature for Neo transaction
    /// @param txHash The transaction hash
    /// @param privateKey The private key
    /// @return The signature bytes
    static Bytes signTransaction(const Bytes& txHash, const SharedPtr<ECPrivateKey>& privateKey);
};

} // namespace neocpp