#include "neocpp/contract/policy_contract.hpp"
#include "neocpp/protocol/neo_rpc_client.hpp"
#include "neocpp/types/contract_parameter.hpp"
#include "neocpp/transaction/transaction_builder.hpp"
#include "neocpp/exceptions.hpp"

namespace neocpp {

const Hash160 PolicyContract::SCRIPT_HASH = Hash160("0xcc5e4edd9f5f8dba8bb65734541df7a1c081c67b");
const std::string PolicyContract::NAME = "PolicyContract";

PolicyContract::PolicyContract(const SharedPtr<NeoRpcClient>& client)
    : SmartContract(SCRIPT_HASH, client) {
}

SharedPtr<PolicyContract> PolicyContract::create(const SharedPtr<NeoRpcClient>& client) {
    return std::make_shared<PolicyContract>(client);
}

int64_t PolicyContract::getFeePerByte() {
    auto result = invokeFunction("getFeePerByte");
    return result["stack"][0]["value"].get<int64_t>();
}

int32_t PolicyContract::getExecFeeFactor() {
    auto result = invokeFunction("getExecFeeFactor");
    return result["stack"][0]["value"].get<int32_t>();
}

int64_t PolicyContract::getStoragePrice() {
    auto result = invokeFunction("getStoragePrice");
    return result["stack"][0]["value"].get<int64_t>();
}

uint32_t PolicyContract::getMaxTransactionsPerBlock() {
    auto result = invokeFunction("getMaxTransactionsPerBlock");
    return result["stack"][0]["value"].get<uint32_t>();
}

uint32_t PolicyContract::getMaxBlockSize() {
    auto result = invokeFunction("getMaxBlockSize");
    return result["stack"][0]["value"].get<uint32_t>();
}

int64_t PolicyContract::getMaxBlockSystemFee() {
    auto result = invokeFunction("getMaxBlockSystemFee");
    return result["stack"][0]["value"].get<int64_t>();
}

bool PolicyContract::isBlocked(const Hash160& account) {
    std::vector<ContractParameter> params = {
        ContractParameter::hash160(account)
    };
    
    auto result = invokeFunction("isBlocked", params);
    return result["stack"][0]["value"].get<bool>();
}

SharedPtr<TransactionBuilder> PolicyContract::setFeePerByte(int64_t value) {
    auto builder = std::make_shared<TransactionBuilder>(client_);
    std::vector<ContractParameter> params = {
        ContractParameter::integer(value)
    };
    builder->callContract(SCRIPT_HASH, "setFeePerByte", params);
    return builder;
}

SharedPtr<TransactionBuilder> PolicyContract::setExecFeeFactor(int32_t value) {
    auto builder = std::make_shared<TransactionBuilder>(client_);
    std::vector<ContractParameter> params = {
        ContractParameter::integer(value)
    };
    builder->callContract(SCRIPT_HASH, "setExecFeeFactor", params);
    return builder;
}

SharedPtr<TransactionBuilder> PolicyContract::setStoragePrice(int64_t value) {
    auto builder = std::make_shared<TransactionBuilder>(client_);
    std::vector<ContractParameter> params = {
        ContractParameter::integer(value)
    };
    builder->callContract(SCRIPT_HASH, "setStoragePrice", params);
    return builder;
}

SharedPtr<TransactionBuilder> PolicyContract::blockAccount(const Hash160& account) {
    auto builder = std::make_shared<TransactionBuilder>(client_);
    std::vector<ContractParameter> params = {
        ContractParameter::hash160(account)
    };
    builder->callContract(SCRIPT_HASH, "blockAccount", params);
    return builder;
}

SharedPtr<TransactionBuilder> PolicyContract::unblockAccount(const Hash160& account) {
    auto builder = std::make_shared<TransactionBuilder>(client_);
    std::vector<ContractParameter> params = {
        ContractParameter::hash160(account)
    };
    builder->callContract(SCRIPT_HASH, "unblockAccount", params);
    return builder;
}

} // namespace neocpp