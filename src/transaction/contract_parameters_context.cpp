#include "neocpp/transaction/contract_parameters_context.hpp"
#include "neocpp/transaction/transaction.hpp"
#include "neocpp/transaction/witness.hpp"
#include "neocpp/transaction/signer.hpp"
#include "neocpp/wallet/account.hpp"
#include "neocpp/crypto/ec_key_pair.hpp"
#include "neocpp/script/script_builder.hpp"
#include "neocpp/script/op_code.hpp"
#include "neocpp/crypto/hash.hpp"
#include "neocpp/utils/hex.hpp"
#include "neocpp/exceptions.hpp"
#include "neocpp/serialization/binary_writer.hpp"
#include "neocpp/serialization/binary_reader.hpp"
#include <algorithm>

namespace neocpp {

ContractParametersContext::ContractParametersContext(const SharedPtr<Transaction>& transaction)
    : transaction_(transaction) {
    if (!transaction) {
        throw IllegalArgumentException("Transaction cannot be null");
    }
    
    // Initialize verification scripts from signers
    for (const auto& signer : transaction->getSigners()) {
        // Would need to fetch verification script from blockchain or cache
        // For now, leave empty
        verificationScripts_[signer->getAccount()] = Bytes();
    }
}

void ContractParametersContext::addSignature(const SharedPtr<Account>& account, const Bytes& signature) {
    auto scriptHash = account->getScriptHash();
    auto publicKey = account->getKeyPair()->getPublicKey()->getEncoded();
    
    addSignature(scriptHash, publicKey, signature);
    
    // Store verification script if not already stored
    if (verificationScripts_[scriptHash].empty()) {
        verificationScripts_[scriptHash] = account->getVerificationScript();
        parseVerificationScript(scriptHash, account->getVerificationScript());
    }
}

void ContractParametersContext::addSignature(const Hash160& scriptHash, const Bytes& publicKey, const Bytes& signature) {
    // Add signature to the collection
    signatures_[scriptHash].push_back(signature);
}

bool ContractParametersContext::sign(const SharedPtr<Account>& account) {
    auto txHash = transaction_->getHash();
    auto signature = account->sign(txHash.toArray());
    
    addSignature(account, signature);
    return true;
}

bool ContractParametersContext::isComplete() const {
    for (const auto& signer : transaction_->getSigners()) {
        if (!isComplete(signer->getAccount())) {
            return false;
        }
    }
    return true;
}

bool ContractParametersContext::isComplete(const Hash160& scriptHash) const {
    int required = getRequiredSignatures(scriptHash);
    int collected = getCollectedSignatures(scriptHash);
    return collected >= required;
}

std::vector<SharedPtr<Witness>> ContractParametersContext::getWitnesses() const {
    std::vector<SharedPtr<Witness>> witnesses;
    
    for (const auto& signer : transaction_->getSigners()) {
        auto witness = getWitness(signer->getAccount());
        if (witness) {
            witnesses.push_back(witness);
        }
    }
    
    return witnesses;
}

SharedPtr<Witness> ContractParametersContext::getWitness(const Hash160& scriptHash) const {
    auto sigIt = signatures_.find(scriptHash);
    auto scriptIt = verificationScripts_.find(scriptHash);
    
    if (sigIt == signatures_.end() || scriptIt == verificationScripts_.end()) {
        return nullptr;
    }
    
    // Build invocation script
    ScriptBuilder builder;
    for (const auto& sig : sigIt->second) {
        builder.pushData(sig);
    }
    
    auto witness = std::make_shared<Witness>();
    witness->setInvocationScript(builder.toArray());
    witness->setVerificationScript(scriptIt->second);
    
    return witness;
}

nlohmann::json ContractParametersContext::toJson() const {
    nlohmann::json json;
    
    // Serialize transaction to bytes then encode to hex
    BinaryWriter writer;
    transaction_->serialize(writer);
    json["transaction"] = Hex::encode(writer.toArray());
    
    // Serialize signatures
    nlohmann::json sigs;
    for (const auto& [scriptHash, signatures] : signatures_) {
        nlohmann::json sigArray;
        for (const auto& sig : signatures) {
            sigArray.push_back(Hex::encode(sig));
        }
        sigs[scriptHash.toString()] = sigArray;
    }
    json["signatures"] = sigs;
    
    // Serialize verification scripts
    nlohmann::json scripts;
    for (const auto& [scriptHash, script] : verificationScripts_) {
        scripts[scriptHash.toString()] = Hex::encode(script);
    }
    json["verificationScripts"] = scripts;
    
    return json;
}

SharedPtr<ContractParametersContext> ContractParametersContext::fromJson(const nlohmann::json& json) {
    // Deserialize transaction
    auto txHex = json["transaction"].get<std::string>();
    auto txBytes = Hex::decode(txHex);
    BinaryReader reader(txBytes);
    auto transaction = Transaction::deserialize(reader);
    
    auto context = std::make_shared<ContractParametersContext>(transaction);
    
    // Deserialize signatures
    if (json.contains("signatures")) {
        for (const auto& [hashStr, sigArray] : json["signatures"].items()) {
            Hash160 scriptHash(hashStr);
            for (const auto& sigHex : sigArray) {
                auto sig = Hex::decode(sigHex.get<std::string>());
                context->signatures_[scriptHash].push_back(sig);
            }
        }
    }
    
    // Deserialize verification scripts
    if (json.contains("verificationScripts")) {
        for (const auto& [hashStr, scriptHex] : json["verificationScripts"].items()) {
            Hash160 scriptHash(hashStr);
            auto script = Hex::decode(scriptHex.get<std::string>());
            context->verificationScripts_[scriptHash] = script;
        }
    }
    
    return context;
}

void ContractParametersContext::parseVerificationScript(const Hash160& scriptHash, const Bytes& script) {
    // Basic parsing to determine if single-sig or multi-sig
    // This is simplified - real implementation would need full script parsing
    
    if (script.empty()) {
        return;
    }
    
    // Check for standard verification scripts
    // Single-sig: PUSH21 <pubkey> PUSH1 PUSH1 PUSH_SYSCALL <checksig>
    // Multi-sig: PUSH1 <m> PUSH21 <pubkey1> ... PUSH1 <n> PUSH_SYSCALL <checkmultisig>
    
    // For now, assume single-sig for simplicity
    // Real implementation would parse the script properly
}

int ContractParametersContext::getRequiredSignatures(const Hash160& scriptHash) const {
    // Parse verification script to determine required signatures
    // For now, return 1 for single-sig
    return 1;
}

int ContractParametersContext::getCollectedSignatures(const Hash160& scriptHash) const {
    auto it = signatures_.find(scriptHash);
    if (it == signatures_.end()) {
        return 0;
    }
    return static_cast<int>(it->second.size());
}

} // namespace neocpp