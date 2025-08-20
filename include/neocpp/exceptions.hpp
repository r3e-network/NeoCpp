#pragma once

#include <exception>
#include <string>
#include <stdexcept>

namespace neocpp {

/// Base exception class for NeoCpp
class NeoException : public std::exception {
protected:
    std::string message_;
    
public:
    explicit NeoException(const std::string& message) : message_(message) {}
    virtual ~NeoException() = default;
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
};

/// Exception thrown when an invalid argument is provided
class IllegalArgumentException : public NeoException {
public:
    explicit IllegalArgumentException(const std::string& message) 
        : NeoException("Illegal argument: " + message) {}
};

/// Exception thrown when an invalid state is encountered
class IllegalStateException : public NeoException {
public:
    explicit IllegalStateException(const std::string& message) 
        : NeoException("Illegal state: " + message) {}
};

/// Exception thrown when serialization fails
class SerializationException : public NeoException {
public:
    explicit SerializationException(const std::string& message) 
        : NeoException("Serialization error: " + message) {}
};

/// Exception thrown when deserialization fails
class DeserializationException : public NeoException {
public:
    explicit DeserializationException(const std::string& message) 
        : NeoException("Deserialization error: " + message) {}
};

/// Exception thrown when cryptographic operations fail
class CryptoException : public NeoException {
public:
    explicit CryptoException(const std::string& message) 
        : NeoException("Crypto error: " + message) {}
};

/// Exception thrown when wallet operations fail
class WalletException : public NeoException {
public:
    explicit WalletException(const std::string& message) 
        : NeoException("Wallet error: " + message) {}
};

/// Exception thrown when transaction operations fail
class TransactionException : public NeoException {
public:
    explicit TransactionException(const std::string& message) 
        : NeoException("Transaction error: " + message) {}
};

/// Exception thrown when contract operations fail
class ContractException : public NeoException {
public:
    explicit ContractException(const std::string& message) 
        : NeoException("Contract error: " + message) {}
};

/// Exception thrown when RPC operations fail
class RpcException : public NeoException {
public:
    explicit RpcException(const std::string& message) 
        : NeoException("RPC error: " + message) {}
};

/// Exception thrown when script operations fail
class ScriptException : public NeoException {
public:
    explicit ScriptException(const std::string& message) 
        : NeoException("Script error: " + message) {}
};

/// Exception thrown when NEP2 operations fail
class NEP2Exception : public NeoException {
public:
    explicit NEP2Exception(const std::string& message) 
        : NeoException("NEP2 error: " + message) {}
};

/// Exception thrown when signing operations fail
class SignException : public NeoException {
public:
    explicit SignException(const std::string& message) 
        : NeoException("Sign error: " + message) {}
};

} // namespace neocpp