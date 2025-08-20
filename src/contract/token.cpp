#include "neocpp/contract/token.hpp"

namespace neocpp {

Token::Token(const Hash160& scriptHash, const SharedPtr<NeoRpcClient>& client)
    : SmartContract(scriptHash, client) {
}

} // namespace neocpp