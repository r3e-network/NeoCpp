#include "neocpp/transaction/witness.hpp"
#include "neocpp/types/hash160.hpp"
#include "neocpp/serialization/binary_writer.hpp"
#include "neocpp/serialization/binary_reader.hpp"
#include "neocpp/script/script_builder.hpp"
#include "neocpp/crypto/hash.hpp"
#include "neocpp/crypto/ec_key_pair.hpp"

namespace neocpp {

Witness::Witness(const Bytes& invocationScript, const Bytes& verificationScript)
    : invocationScript_(invocationScript), verificationScript_(verificationScript) {
}

Hash160 Witness::getScriptHash() const {
    if (verificationScript_.empty()) {
        return Hash160();
    }
    return Hash160::fromScript(verificationScript_);
}

SharedPtr<Witness> Witness::fromSignature(const Bytes& signature, const Bytes& publicKey) {
    Bytes invocation = ScriptBuilder::buildInvocationScript({signature});
    Bytes verification = ScriptBuilder::buildVerificationScript(publicKey);
    return std::make_shared<Witness>(invocation, verification);
}

SharedPtr<Witness> Witness::fromMultiSignature(const std::vector<Bytes>& signatures,
                                              const std::vector<Bytes>& publicKeys,
                                              int signingThreshold) {
    Bytes invocation = ScriptBuilder::buildInvocationScript(signatures);
    
    // Convert Bytes to ECPublicKey for verification script
    std::vector<SharedPtr<ECPublicKey>> pubKeys;
    for (const auto& key : publicKeys) {
        pubKeys.push_back(std::make_shared<ECPublicKey>(key));
    }
    
    Bytes verification = ScriptBuilder::buildVerificationScript(pubKeys, signingThreshold);
    return std::make_shared<Witness>(invocation, verification);
}

size_t Witness::getSize() const {
    BinaryWriter writer;
    writer.writeVarInt(invocationScript_.size());
    writer.writeVarInt(verificationScript_.size());
    return writer.size() + invocationScript_.size() + verificationScript_.size();
}

void Witness::serialize(BinaryWriter& writer) const {
    writer.writeVarBytes(invocationScript_);
    writer.writeVarBytes(verificationScript_);
}

SharedPtr<Witness> Witness::deserialize(BinaryReader& reader) {
    Bytes invocation = reader.readVarBytes();
    Bytes verification = reader.readVarBytes();
    return std::make_shared<Witness>(invocation, verification);
}

bool Witness::operator==(const Witness& other) const {
    return invocationScript_ == other.invocationScript_ &&
           verificationScript_ == other.verificationScript_;
}

bool Witness::operator!=(const Witness& other) const {
    return !(*this == other);
}

} // namespace neocpp