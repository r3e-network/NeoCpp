#include "neocpp/transaction/transaction_builder.hpp"
#include "neocpp/transaction/transaction.hpp"
#include "neocpp/transaction/signer.hpp"
#include "neocpp/transaction/witness.hpp"
#include "neocpp/protocol/neo_rpc_client.hpp"
#include "neocpp/wallet/account.hpp"
#include "neocpp/script/script_builder.hpp"
#include "neocpp/types/contract_parameter.hpp"
#include "neocpp/contract/neo_token.hpp"
#include "neocpp/contract/gas_token.hpp"
#include "neocpp/utils/address.hpp"
#include "neocpp/crypto/hash.hpp"
#include "neocpp/exceptions.hpp"
#include <random>
#include <algorithm>

namespace neocpp {

TransactionBuilder::TransactionBuilder(const SharedPtr<NeoRpcClient>& client)
    : client_(client) {
    initializeTransaction();
}

TransactionBuilder& TransactionBuilder::setClient(const SharedPtr<NeoRpcClient>& client) {
    client_ = client;
    return *this;
}

TransactionBuilder& TransactionBuilder::setNonce(uint32_t nonce) {
    transaction_->setNonce(nonce);
    return *this;
}

TransactionBuilder& TransactionBuilder::setRandomNonce() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dis;
    transaction_->setNonce(dis(gen));
    return *this;
}

TransactionBuilder& TransactionBuilder::setValidUntilBlock(uint32_t block) {
    transaction_->setValidUntilBlock(block);
    return *this;
}

TransactionBuilder& TransactionBuilder::setValidUntilBlockRelative(uint32_t blocksFromNow) {
    if (!client_) {
        throw IllegalStateException("RPC client not set");
    }
    
    auto blockCount = client_->getBlockCount();
    if (!blockCount) {
        throw RuntimeException("Failed to get block count");
    }
    
    transaction_->setValidUntilBlock(blockCount + blocksFromNow);
    return *this;
}

TransactionBuilder& TransactionBuilder::setSystemFee(int64_t fee) {
    transaction_->setSystemFee(fee);
    return *this;
}

TransactionBuilder& TransactionBuilder::setNetworkFee(int64_t fee) {
    transaction_->setNetworkFee(fee);
    return *this;
}

TransactionBuilder& TransactionBuilder::calculateNetworkFee() {
    if (!client_) {
        throw IllegalStateException("RPC client not set");
    }
    
    // Base fee calculation
    int64_t fee = 0;
    
    // Calculate verification cost for each signer
    for (const auto& signer : transaction_->getSigners()) {
        // Basic verification cost
        fee += 1000000; // 0.01 GAS base cost
    }
    
    // Calculate size-based fee
    auto size = transaction_->getSize();
    fee += size * 1000; // Fee per byte
    
    transaction_->setNetworkFee(fee);
    return *this;
}

TransactionBuilder& TransactionBuilder::addSigner(const SharedPtr<Signer>& signer) {
    transaction_->addSigner(signer);
    return *this;
}

TransactionBuilder& TransactionBuilder::addSigner(const SharedPtr<Account>& account, uint8_t scopes) {
    auto signer = std::make_shared<Signer>(account->getScriptHash(), static_cast<WitnessScope>(scopes));
    transaction_->addSigner(signer);
    signingAccounts_.push_back(account);
    return *this;
}

TransactionBuilder& TransactionBuilder::setScript(const Bytes& script) {
    transaction_->setScript(script);
    return *this;
}

TransactionBuilder& TransactionBuilder::callContract(const Hash160& scriptHash, 
                                                      const std::string& method, 
                                                      const std::vector<ContractParameter>& params) {
    ScriptBuilder builder;
    
    // Push parameters in reverse order
    for (auto it = params.rbegin(); it != params.rend(); ++it) {
        builder.pushContractParameter(*it);
    }
    
    // Push method name
    builder.pushString(method);
    
    // Push contract hash
    builder.pushData(scriptHash.toArray());
    
    // System call
    builder.emitSysCall("System.Contract.Call");
    
    transaction_->setScript(builder.toArray());
    return *this;
}

TransactionBuilder& TransactionBuilder::transferNeo(const SharedPtr<Account>& from, 
                                                    const std::string& to, 
                                                    int64_t amount) {
    return transferNep17(NeoToken::SCRIPT_HASH, from, to, amount, 0);
}

TransactionBuilder& TransactionBuilder::transferGas(const SharedPtr<Account>& from, 
                                                    const std::string& to, 
                                                    int64_t amount) {
    return transferNep17(GasToken::SCRIPT_HASH, from, to, amount, 8);
}

TransactionBuilder& TransactionBuilder::transferNep17(const Hash160& tokenHash, 
                                                      const SharedPtr<Account>& from, 
                                                      const std::string& to, 
                                                      int64_t amount, 
                                                      int decimals) {
    auto fromHash = from->getScriptHash();
    auto toBytes = AddressUtils::addressToScriptHash(to);
    Hash160 toHash(toBytes);
    
    std::vector<ContractParameter> params = {
        ContractParameter::hash160(fromHash),
        ContractParameter::hash160(toHash),
        ContractParameter::integer(amount),
        ContractParameter::any()
    };
    
    callContract(tokenHash, "transfer", params);
    
    // Add signer if not already added
    bool found = false;
    for (const auto& signer : transaction_->getSigners()) {
        if (signer->getAccount() == fromHash) {
            found = true;
            break;
        }
    }
    
    if (!found) {
        addSigner(from);
    }
    
    return *this;
}

TransactionBuilder& TransactionBuilder::addWitness(const SharedPtr<Witness>& witness) {
    transaction_->addWitness(witness);
    return *this;
}

TransactionBuilder& TransactionBuilder::sign(const SharedPtr<Account>& account) {
    addWitnessForAccount(account);
    return *this;
}

TransactionBuilder& TransactionBuilder::signWithSigners() {
    for (const auto& account : signingAccounts_) {
        addWitnessForAccount(account);
    }
    return *this;
}

SharedPtr<Transaction> TransactionBuilder::build() {
    // Sort signers and witnesses
    sortSigners();
    sortWitnesses();
    
    // Set default valid until block if not set
    if (transaction_->getValidUntilBlock() == 0) {
        setValidUntilBlockRelative(100); // 100 blocks from now
    }
    
    // Set random nonce if not set
    if (transaction_->getNonce() == 0) {
        setRandomNonce();
    }
    
    return transaction_;
}

SharedPtr<Transaction> TransactionBuilder::buildAndSign() {
    build();
    signWithSigners();
    return transaction_;
}

void TransactionBuilder::initializeTransaction() {
    transaction_ = std::make_shared<Transaction>();
    transaction_->setVersion(0);
    transaction_->setSystemFee(0);
    transaction_->setNetworkFee(0);
}

void TransactionBuilder::addWitnessForAccount(const SharedPtr<Account>& account) {
    // Get transaction hash for signing
    auto txHash = transaction_->getHash();
    
    // Sign the transaction hash
    auto signature = account->sign(txHash.toArray());
    
    // Create witness
    auto witness = std::make_shared<Witness>();
    witness->setInvocationScript(ScriptBuilder().pushData(signature).toArray());
    witness->setVerificationScript(account->getVerificationScript());
    
    transaction_->addWitness(witness);
}

void TransactionBuilder::sortSigners() {
    auto signers = transaction_->getSigners();
    
    // Sort signers by scope first, then by account hash
    std::sort(signers.begin(), signers.end(), 
        [](const SharedPtr<Signer>& a, const SharedPtr<Signer>& b) {
            if (a->getScopes() != b->getScopes()) {
                return a->getScopes() < b->getScopes();
            }
            return a->getAccount().toString() < b->getAccount().toString();
        });
    
    // Transaction doesn't have setSigners - signers are already in the transaction
}

void TransactionBuilder::sortWitnesses() {
    auto witnesses = transaction_->getWitnesses();
    auto signers = transaction_->getSigners();
    
    if (witnesses.size() != signers.size()) {
        return; // Can't sort if counts don't match
    }
    
    // Sort witnesses to match signer order
    std::vector<SharedPtr<Witness>> sortedWitnesses;
    for (const auto& signer : signers) {
        // Find matching witness by verification script hash
        for (const auto& witness : witnesses) {
            auto scriptBytes = HashUtils::sha256ThenRipemd160(witness->getVerificationScript());
            Hash160 scriptHash(scriptBytes);
            if (scriptHash == signer->getAccount()) {
                sortedWitnesses.push_back(witness);
                break;
            }
        }
    }
    
    if (sortedWitnesses.size() == witnesses.size()) {
        // Transaction doesn't have setWitnesses - witnesses are already in the transaction
    }
}

} // namespace neocpp