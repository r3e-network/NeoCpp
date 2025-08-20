#include "neocpp/contract/gas_token.hpp"
#include "neocpp/protocol/neo_rpc_client.hpp"
#include "neocpp/types/contract_parameter.hpp"
#include "neocpp/wallet/account.hpp"
#include "neocpp/utils/address.hpp"
#include "neocpp/exceptions.hpp"

namespace neocpp {

const Hash160 GasToken::SCRIPT_HASH = Hash160("0xd2a4cff31913016155e38e474a2c06d08be276cf");

GasToken::GasToken(const SharedPtr<NeoRpcClient>& client)
    : FungibleToken(SCRIPT_HASH, client) {
}

SharedPtr<TransactionBuilder> GasToken::claim(const SharedPtr<Account>& account) {
    std::vector<ContractParameter> params = {
        ContractParameter::hash160(account->getScriptHash()),
        ContractParameter::integer(0)
    };
    
    return buildInvokeTx("claim", params, account);
}

int64_t GasToken::getFeePerByte() {
    auto result = invokeFunction("getFeePerByte");
    return result["stack"][0]["value"].get<int64_t>();
}

int32_t GasToken::getExecFeeFactor() {
    auto result = invokeFunction("getExecFeeFactor");
    return result["stack"][0]["value"].get<int32_t>();
}

int64_t GasToken::getStoragePrice() {
    auto result = invokeFunction("getStoragePrice");
    return result["stack"][0]["value"].get<int64_t>();
}

} // namespace neocpp