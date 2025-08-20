#pragma once

#include <memory>
#include <vector>
#include "neocpp/types/types.hpp"
#include "neocpp/types/hash256.hpp"
#include "neocpp/serialization/neo_serializable.hpp"

namespace neocpp {

// Forward declarations
class BinaryWriter;
class BinaryReader;

/// Transaction attribute types
enum class TransactionAttributeType : uint8_t {
    HIGH_PRIORITY = 0x01,
    ORACLE_RESPONSE = 0x11,
    NOT_VALID_BEFORE = 0x20,
    CONFLICTS = 0x21
};

/// Base class for transaction attributes
class TransactionAttribute : public NeoSerializable {
public:
    virtual ~TransactionAttribute() = default;
    
    /// Get the attribute type
    virtual TransactionAttributeType getType() const = 0;
    
    /// Serialize to binary writer
    void serialize(BinaryWriter& writer) const override;
    
    /// Create from type and reader
    static SharedPtr<TransactionAttribute> deserialize(BinaryReader& reader);
    
protected:
    /// Serialize the attribute data (without type byte)
    virtual void serializeWithoutType(BinaryWriter& writer) const = 0;
};

/// High priority attribute - transaction with this attribute has priority to be included in a block
class HighPriorityAttribute : public TransactionAttribute {
public:
    TransactionAttributeType getType() const override {
        return TransactionAttributeType::HIGH_PRIORITY;
    }
    
    size_t getSize() const override {
        return 1; // Just the type byte
    }
    
protected:
    void serializeWithoutType(BinaryWriter& writer) const override {
        // No additional data for high priority
    }
};

/// Oracle response attribute
class OracleResponseAttribute : public TransactionAttribute {
private:
    uint64_t id_;
    uint8_t code_;
    Bytes result_;
    
public:
    OracleResponseAttribute(uint64_t id, uint8_t code, const Bytes& result)
        : id_(id), code_(code), result_(result) {}
    
    TransactionAttributeType getType() const override {
        return TransactionAttributeType::ORACLE_RESPONSE;
    }
    
    uint64_t getId() const { return id_; }
    uint8_t getCode() const { return code_; }
    const Bytes& getResult() const { return result_; }
    
    size_t getSize() const override;
    
protected:
    void serializeWithoutType(BinaryWriter& writer) const override;
};

/// Not valid before attribute - transaction is not valid before specified height
class NotValidBeforeAttribute : public TransactionAttribute {
private:
    uint32_t height_;
    
public:
    explicit NotValidBeforeAttribute(uint32_t height) : height_(height) {}
    
    TransactionAttributeType getType() const override {
        return TransactionAttributeType::NOT_VALID_BEFORE;
    }
    
    uint32_t getHeight() const { return height_; }
    
    size_t getSize() const override {
        return 1 + 4; // Type byte + uint32
    }
    
protected:
    void serializeWithoutType(BinaryWriter& writer) const override;
};

/// Conflicts attribute - transaction conflicts with another transaction
class ConflictsAttribute : public TransactionAttribute {
private:
    Hash256 hash_;
    
public:
    explicit ConflictsAttribute(const Hash256& hash) : hash_(hash) {}
    
    TransactionAttributeType getType() const override {
        return TransactionAttributeType::CONFLICTS;
    }
    
    const Hash256& getHash() const { return hash_; }
    
    size_t getSize() const override {
        return 1 + 32; // Type byte + hash256
    }
    
protected:
    void serializeWithoutType(BinaryWriter& writer) const override;
};

} // namespace neocpp