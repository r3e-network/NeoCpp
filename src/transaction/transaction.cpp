#include "neocpp/transaction/transaction.hpp"
#include "neocpp/transaction/signer.hpp"
#include "neocpp/transaction/witness.hpp"
#include "neocpp/transaction/transaction_attribute.hpp"
#include "neocpp/serialization/binary_writer.hpp"
#include "neocpp/serialization/binary_reader.hpp"
#include "neocpp/crypto/hash.hpp"
#include "neocpp/neo_constants.hpp"
#include "neocpp/exceptions.hpp"
#include <algorithm>
#include <random>
#include <limits>

namespace neocpp {

Transaction::Transaction() 
    : version_(NeoConstants::CURRENT_TX_VERSION),
      nonce_(generateNonce()),
      systemFee_(0),
      networkFee_(0),
      validUntilBlock_(0),
      hashCalculated_(false) {
}

void Transaction::addSigner(const SharedPtr<Signer>& signer) {
    signers_.push_back(signer);
    hashCalculated_ = false;
}

void Transaction::addAttribute(const SharedPtr<TransactionAttribute>& attribute) {
    if (attributes_.size() >= NeoConstants::MAX_TRANSACTION_ATTRIBUTES) {
        throw TransactionException("Maximum number of attributes exceeded");
    }
    attributes_.push_back(attribute);
    hashCalculated_ = false;
}

void Transaction::addWitness(const SharedPtr<Witness>& witness) {
    witnesses_.push_back(witness);
}

const Hash256& Transaction::getHash() const {
    if (!hashCalculated_) {
        hash_ = calculateHash();
        hashCalculated_ = true;
    }
    return hash_;
}

std::string Transaction::getTxId() const {
    return getHash().toString();
}

Hash256 Transaction::calculateHash() const {
    Bytes hashData = getHashData();
    Bytes hash = HashUtils::sha256(hashData);
    return Hash256(hash);
}

Bytes Transaction::getHashData() const {
    BinaryWriter writer;
    serializeUnsigned(writer);
    return writer.toArray();
}

bool Transaction::verify() const {
    // Basic validation
    if (signers_.empty()) {
        return false;
    }
    
    if (systemFee_ < 0 || networkFee_ < 0) {
        return false;
    }
    
    if (script_.empty()) {
        return false;
    }
    
    if (getSize() > NeoConstants::MAX_TRANSACTION_SIZE) {
        return false;
    }
    
    // Check for duplicate signers
    std::vector<Hash160> signerHashes;
    for (const auto& signer : signers_) {
        const Hash160& hash = signer->getAccount();
        if (std::find(signerHashes.begin(), signerHashes.end(), hash) != signerHashes.end()) {
            return false;
        }
        signerHashes.push_back(hash);
    }
    
    // Verify witnesses match signers
    if (witnesses_.size() != signers_.size()) {
        return false;
    }
    
    return true;
}

int64_t Transaction::calculateNetworkFee() const {
    // Basic calculation - should be enhanced with actual fee calculation
    int64_t fee = 0;
    
    // Base fee per byte
    const int64_t feePerByte = 1000; // GAS fractions
    fee += getSize() * feePerByte;
    
    // Verification cost
    for (const auto& witness : witnesses_) {
        // Estimate verification cost
        fee += witness->getVerificationScript().size() * 100;
        fee += witness->getInvocationScript().size() * 100;
    }
    
    return fee;
}

size_t Transaction::getSize() const {
    BinaryWriter writer;
    serialize(writer);
    return writer.size();
}

void Transaction::serialize(BinaryWriter& writer) const {
    serializeUnsigned(writer);
    
    // Write witnesses
    writer.writeVarInt(witnesses_.size());
    for (const auto& witness : witnesses_) {
        witness->serialize(writer);
    }
}

void Transaction::serializeUnsigned(BinaryWriter& writer) const {
    writer.writeUInt8(version_);
    writer.writeUInt32(nonce_);
    writer.writeInt64(systemFee_);
    writer.writeInt64(networkFee_);
    writer.writeUInt32(validUntilBlock_);
    
    // Write signers
    writer.writeVarInt(signers_.size());
    for (const auto& signer : signers_) {
        signer->serialize(writer);
    }
    
    // Write attributes
    writer.writeVarInt(attributes_.size());
    for (const auto& attribute : attributes_) {
        attribute->serialize(writer);
    }
    
    // Write script
    writer.writeVarBytes(script_);
}

SharedPtr<Transaction> Transaction::deserialize(BinaryReader& reader) {
    auto tx = std::make_shared<Transaction>();
    
    tx->version_ = reader.readUInt8();
    tx->nonce_ = reader.readUInt32();
    tx->systemFee_ = reader.readInt64();
    tx->networkFee_ = reader.readInt64();
    tx->validUntilBlock_ = reader.readUInt32();
    
    // Read signers
    uint64_t signerCount = reader.readVarInt();
    for (uint64_t i = 0; i < signerCount; ++i) {
        tx->signers_.push_back(Signer::deserialize(reader));
    }
    
    // Read attributes
    uint64_t attrCount = reader.readVarInt();
    tx->attributes_.clear();
    for (uint64_t i = 0; i < attrCount; ++i) {
        // Read attribute type
        uint8_t type = reader.readByte();
        
        // Neo N3 supports the following attribute types:
        // 0x01 - HighPriority
        // 0x11 - OracleResponse
        // 0x20 - NotValidBefore
        // 0x21 - Conflicts
        
        switch (type) {
            case 0x01: // HighPriority
                // No additional data for HighPriority
                break;
            case 0x11: { // OracleResponse
                [[maybe_unused]] uint64_t id = reader.readUInt64();
                [[maybe_unused]] uint8_t code = reader.readUInt8();
                Bytes result = reader.readVarBytes();
                // Store the oracle response data (would need proper attribute structure)
                break;
            }
            case 0x20: { // NotValidBefore
                [[maybe_unused]] uint32_t height = reader.readUInt32();
                // Store the height
                break;
            }
            case 0x21: { // Conflicts
                Hash256 hash = Hash256::deserialize(reader);
                // Store the conflicting transaction hash
                break;
            }
            default:
                // Unknown attribute type, skip remaining data based on type
                // For safety, throw an exception for unknown types
                throw DeserializationException("Unknown transaction attribute type: " + std::to_string(type));
        }
    }
    
    // Read script
    tx->script_ = reader.readVarBytes();
    
    // Read witnesses
    uint64_t witnessCount = reader.readVarInt();
    for (uint64_t i = 0; i < witnessCount; ++i) {
        tx->witnesses_.push_back(Witness::deserialize(reader));
    }
    
    return tx;
}

uint32_t Transaction::generateNonce() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<uint32_t> dis(1, std::numeric_limits<uint32_t>::max());
    return dis(gen);
}

} // namespace neocpp