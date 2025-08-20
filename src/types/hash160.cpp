#include "neocpp/types/hash160.hpp"
#include "neocpp/types/types.hpp"
#include "neocpp/serialization/binary_writer.hpp"
#include "neocpp/serialization/binary_reader.hpp"
#include "neocpp/exceptions.hpp"
#include "neocpp/utils/address.hpp"
#include "neocpp/crypto/hash.hpp"
#include "neocpp/script/script_builder.hpp"
#include <algorithm>
#include <cstring>

namespace neocpp {

// Static member initialization
const Hash160 Hash160::ZERO = Hash160();

Hash160::Hash160() {
    hash_.fill(0);
}

Hash160::Hash160(const Bytes& hash) {
    if (hash.size() != NeoConstants::HASH160_SIZE) {
        throw IllegalArgumentException("Hash must be " + std::to_string(NeoConstants::HASH160_SIZE) + 
                                      " bytes long but was " + std::to_string(hash.size()) + " bytes.");
    }
    std::copy(hash.begin(), hash.end(), hash_.begin());
}

Hash160::Hash160(const std::array<uint8_t, NeoConstants::HASH160_SIZE>& hash) : hash_(hash) {
}

Hash160::Hash160(const std::string& hash) {
    Bytes bytes = ByteUtils::fromHex(hash);
    if (bytes.size() != NeoConstants::HASH160_SIZE) {
        throw IllegalArgumentException("Hash must be " + std::to_string(NeoConstants::HASH160_SIZE) + 
                                      " bytes long but was " + std::to_string(bytes.size()) + " bytes.");
    }
    std::copy(bytes.begin(), bytes.end(), hash_.begin());
}

std::string Hash160::toString() const {
    return ByteUtils::toHex(Bytes(hash_.begin(), hash_.end()), false);
}

Bytes Hash160::toArray() const {
    return Bytes(hash_.begin(), hash_.end());
}

Bytes Hash160::toLittleEndianArray() const {
    Bytes result(hash_.begin(), hash_.end());
    std::reverse(result.begin(), result.end());
    return result;
}

std::string Hash160::toAddress() const {
    return AddressUtils::scriptHashToAddress(toArray());
}

Hash160 Hash160::fromAddress(const std::string& address) {
    return Hash160(AddressUtils::addressToScriptHash(address));
}

Hash160 Hash160::fromScript(const Bytes& script) {
    Bytes hash = HashUtils::sha256ThenRipemd160(script);
    std::reverse(hash.begin(), hash.end());
    return Hash160(hash);
}

Hash160 Hash160::fromScript(const std::string& script) {
    return fromScript(ByteUtils::fromHex(script));
}

Hash160 Hash160::fromPublicKey(const Bytes& encodedPublicKey) {
    return fromScript(ScriptBuilder::buildVerificationScript(encodedPublicKey));
}

Hash160 Hash160::fromPublicKeys(const std::vector<SharedPtr<ECPublicKey>>& pubKeys, int signingThreshold) {
    return fromScript(ScriptBuilder::buildVerificationScript(pubKeys, signingThreshold));
}

size_t Hash160::getSize() const {
    return NeoConstants::HASH160_SIZE;
}

void Hash160::serialize(BinaryWriter& writer) const {
    Bytes littleEndian = toLittleEndianArray();
    writer.writeBytes(littleEndian);
}

Hash160 Hash160::deserialize(BinaryReader& reader) {
    Bytes bytes = reader.readBytes(NeoConstants::HASH160_SIZE);
    std::reverse(bytes.begin(), bytes.end());
    return Hash160(bytes);
}

bool Hash160::operator==(const Hash160& other) const {
    return hash_ == other.hash_;
}

bool Hash160::operator!=(const Hash160& other) const {
    return !(*this == other);
}

bool Hash160::operator<(const Hash160& other) const {
    return hash_ < other.hash_;
}

bool Hash160::operator<=(const Hash160& other) const {
    return hash_ <= other.hash_;
}

bool Hash160::operator>(const Hash160& other) const {
    return hash_ > other.hash_;
}

bool Hash160::operator>=(const Hash160& other) const {
    return hash_ >= other.hash_;
}

size_t Hash160::Hasher::operator()(const Hash160& hash) const {
    size_t result = 0;
    const auto& array = hash.hash_;
    for (size_t i = 0; i < std::min(sizeof(size_t), array.size()); ++i) {
        result |= static_cast<size_t>(array[i]) << (i * 8);
    }
    return result;
}

} // namespace neocpp