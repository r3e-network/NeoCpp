#include "neocpp/contract/neo_name_service.hpp"
#include "neocpp/protocol/neo_rpc_client.hpp"
#include "neocpp/types/contract_parameter.hpp"
#include "neocpp/utils/address.hpp"
#include "neocpp/transaction/transaction_builder.hpp"
#include "neocpp/wallet/account.hpp"
#include "neocpp/exceptions.hpp"

namespace neocpp {

const Hash160 NeoNameService::SCRIPT_HASH = Hash160("0x50ac1c37690cc2cfc594472833cf57505d5f46de");

NeoNameService::NeoNameService(const SharedPtr<NeoRpcClient>& client)
    : SmartContract(SCRIPT_HASH, client) {
}

std::string NeoNameService::getOwner(const std::string& domain) {
    std::vector<ContractParameter> params = {
        ContractParameter::string(domain)
    };
    
    auto result = invokeFunction("ownerOf", params);
    return result["stack"][0]["value"].get<std::string>();
}

std::string NeoNameService::resolve(const std::string& domain, uint16_t type) {
    std::vector<ContractParameter> params = {
        ContractParameter::string(domain),
        ContractParameter::integer(type)
    };
    
    auto result = invokeFunction("resolve", params);
    return result["stack"][0]["value"].get<std::string>();
}

nlohmann::json NeoNameService::getProperties(const std::string& domain) {
    std::vector<ContractParameter> params = {
        ContractParameter::string(domain)
    };
    
    return invokeFunction("properties", params);
}

bool NeoNameService::isAvailable(const std::string& domain) {
    std::vector<ContractParameter> params = {
        ContractParameter::string(domain)
    };
    
    auto result = invokeFunction("isAvailable", params);
    return result["stack"][0]["value"].get<bool>();
}

SharedPtr<TransactionBuilder> NeoNameService::registerName(const SharedPtr<Account>& account, 
                                                           const std::string& domain, 
                                                           int years) {
    auto builder = std::make_shared<TransactionBuilder>(client_);
    
    std::vector<ContractParameter> params = {
        ContractParameter::string(domain),
        ContractParameter::hash160(account->getScriptHash()),
        ContractParameter::integer(years * 365 * 24 * 3600)  // Convert years to seconds
    };
    
    builder->callContract(SCRIPT_HASH, "register", params);
    builder->addSigner(account);
    
    return builder;
}

SharedPtr<TransactionBuilder> NeoNameService::renew(const SharedPtr<Account>& account,
                                                    const std::string& domain,
                                                    int years) {
    auto builder = std::make_shared<TransactionBuilder>(client_);
    
    std::vector<ContractParameter> params = {
        ContractParameter::string(domain),
        ContractParameter::integer(years * 365 * 24 * 3600)  // Convert years to seconds
    };
    
    builder->callContract(SCRIPT_HASH, "renew", params);
    builder->addSigner(account);
    
    return builder;
}

SharedPtr<TransactionBuilder> NeoNameService::setOwner(const SharedPtr<Account>& account,
                                                       const std::string& domain,
                                                       const std::string& newOwner) {
    auto builder = std::make_shared<TransactionBuilder>(client_);
    
    auto ownerBytes = AddressUtils::addressToScriptHash(newOwner);
    Hash160 ownerHash(ownerBytes);
    
    std::vector<ContractParameter> params = {
        ContractParameter::string(domain),
        ContractParameter::hash160(ownerHash)
    };
    
    builder->callContract(SCRIPT_HASH, "setAdmin", params);
    builder->addSigner(account);
    
    return builder;
}

SharedPtr<TransactionBuilder> NeoNameService::setRecord(const SharedPtr<Account>& account,
                                                        const std::string& domain,
                                                        uint16_t type,
                                                        const std::string& value) {
    auto builder = std::make_shared<TransactionBuilder>(client_);
    
    std::vector<ContractParameter> params = {
        ContractParameter::string(domain),
        ContractParameter::integer(type),
        ContractParameter::string(value)
    };
    
    builder->callContract(SCRIPT_HASH, "setRecord", params);
    builder->addSigner(account);
    
    return builder;
}

int64_t NeoNameService::getPrice(int length) {
    std::vector<ContractParameter> params = {
        ContractParameter::integer(length)
    };
    
    auto result = invokeFunction("getPrice", params);
    return result["stack"][0]["value"].get<int64_t>();
}

std::chrono::system_clock::time_point NeoNameService::getExpiration(const std::string& domain) {
    std::vector<ContractParameter> params = {
        ContractParameter::string(domain)
    };
    
    auto result = invokeFunction("expiresAt", params);
    int64_t timestamp = result["stack"][0]["value"].get<int64_t>();
    
    // Convert milliseconds to time_point
    auto duration = std::chrono::milliseconds(timestamp);
    return std::chrono::system_clock::time_point(duration);
}

} // namespace neocpp