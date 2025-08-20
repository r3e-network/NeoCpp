#pragma once

#include <string>
#include <stdexcept>

namespace neocpp {

/// Base error class for Neo SDK
class NeoSwiftError : public std::runtime_error {
public:
    explicit NeoSwiftError(const std::string& message) 
        : std::runtime_error(message) {}
};

/// Contract error
class ContractError : public NeoSwiftError {
public:
    explicit ContractError(const std::string& message) 
        : NeoSwiftError("Contract error: " + message) {}
};

/// Transaction error
class TransactionError : public NeoSwiftError {
public:
    explicit TransactionError(const std::string& message) 
        : NeoSwiftError("Transaction error: " + message) {}
};

/// Protocol error
class ProtocolError : public NeoSwiftError {
public:
    explicit ProtocolError(const std::string& message) 
        : NeoSwiftError("Protocol error: " + message) {}
};

/// Wallet error
class WalletError : public NeoSwiftError {
public:
    explicit WalletError(const std::string& message) 
        : NeoSwiftError("Wallet error: " + message) {}
};

/// Crypto error
class CryptoError : public NeoSwiftError {
public:
    explicit CryptoError(const std::string& message) 
        : NeoSwiftError("Crypto error: " + message) {}
};

/// NEP2 error
class NEP2Error : public CryptoError {
public:
    explicit NEP2Error(const std::string& message) 
        : CryptoError("NEP2: " + message) {}
};

/// Sign error
class SignError : public CryptoError {
public:
    explicit SignError(const std::string& message) 
        : CryptoError("Sign: " + message) {}
};

/// Script error
class ScriptError : public NeoSwiftError {
public:
    explicit ScriptError(const std::string& message) 
        : NeoSwiftError("Script error: " + message) {}
};

/// Serialization error
class SerializationError : public NeoSwiftError {
public:
    explicit SerializationError(const std::string& message) 
        : NeoSwiftError("Serialization error: " + message) {}
};

/// Validation error
class ValidationError : public NeoSwiftError {
public:
    explicit ValidationError(const std::string& message) 
        : NeoSwiftError("Validation error: " + message) {}
};

/// Network error
class NetworkError : public NeoSwiftError {
public:
    explicit NetworkError(const std::string& message) 
        : NeoSwiftError("Network error: " + message) {}
};

/// RPC error
class RpcError : public NetworkError {
public:
    RpcError(int code, const std::string& message) 
        : NetworkError("RPC error " + std::to_string(code) + ": " + message), 
          code_(code) {}
    
    int getCode() const { return code_; }
    
private:
    int code_;
};

} // namespace neocpp