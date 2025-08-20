#include "neocpp/contract/contract_management.hpp"
#include "neocpp/contract/nef_file.hpp"
#include "neocpp/contract/contract_manifest.hpp"
#include "neocpp/protocol/neo_rpc_client.hpp"
#include "neocpp/types/contract_parameter.hpp"
#include "neocpp/transaction/transaction_builder.hpp"
#include "neocpp/exceptions.hpp"

namespace neocpp {

const Hash160 ContractManagement::SCRIPT_HASH = Hash160("0xfffdc93764dbaddd97c48f252a53ea4643faa3fd");
const std::string ContractManagement::NAME = "ContractManagement";

ContractManagement::ContractManagement(const SharedPtr<NeoRpcClient>& client)
    : SmartContract(SCRIPT_HASH, client) {
}

SharedPtr<ContractManagement> ContractManagement::create(const SharedPtr<NeoRpcClient>& client) {
    return std::make_shared<ContractManagement>(client);
}

SharedPtr<TransactionBuilder> ContractManagement::deploy(const SharedPtr<NefFile>& nef,
                                                         const SharedPtr<ContractManifest>& manifest) {
    std::vector<ContractParameter> params = {
        ContractParameter::byteArray(nef->toBytes()),
        ContractParameter::string(manifest->toJson().dump())
    };
    
    auto builder = std::make_shared<TransactionBuilder>(client_);
    builder->callContract(scriptHash_, "deploy", params);
    return builder;
}

SharedPtr<TransactionBuilder> ContractManagement::update(const Hash160& scriptHash,
                                                         const SharedPtr<NefFile>& nef,
                                                         const SharedPtr<ContractManifest>& manifest) {
    std::vector<ContractParameter> params = {
        ContractParameter::hash160(scriptHash)
    };
    
    if (nef) {
        params.push_back(ContractParameter::byteArray(nef->toBytes()));
    } else {
        params.push_back(ContractParameter::any());
    }
    
    if (manifest) {
        params.push_back(ContractParameter::string(manifest->toJson().dump()));
    } else {
        params.push_back(ContractParameter::any());
    }
    
    auto builder = std::make_shared<TransactionBuilder>(client_);
    builder->callContract(scriptHash_, "update", params);
    return builder;
}

SharedPtr<TransactionBuilder> ContractManagement::destroy(const Hash160& scriptHash) {
    std::vector<ContractParameter> params = {
        ContractParameter::hash160(scriptHash)
    };
    
    auto builder = std::make_shared<TransactionBuilder>(client_);
    builder->callContract(scriptHash_, "destroy", params);
    return builder;
}

nlohmann::json ContractManagement::getContract(const Hash160& scriptHash) {
    std::vector<ContractParameter> params = {
        ContractParameter::hash160(scriptHash)
    };
    
    return invokeFunction("getContract", params);
}

bool ContractManagement::hasMethod(const Hash160& scriptHash, const std::string& method, int paramCount) {
    std::vector<ContractParameter> params = {
        ContractParameter::hash160(scriptHash),
        ContractParameter::string(method),
        ContractParameter::integer(paramCount)
    };
    
    auto result = invokeFunction("hasMethod", params);
    return result["stack"][0]["value"].get<bool>();
}

int64_t ContractManagement::getMinimumDeploymentFee() {
    auto result = invokeFunction("getMinimumDeploymentFee");
    return result["stack"][0]["value"].get<int64_t>();
}

} // namespace neocpp