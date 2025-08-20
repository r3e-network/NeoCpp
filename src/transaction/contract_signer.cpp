#include "neocpp/transaction/contract_signer.hpp"

namespace neocpp {

ContractSigner::ContractSigner(const Hash160& contractHash, WitnessScope scopes)
    : Signer(contractHash, scopes) {
}

ContractSigner::ContractSigner(const Hash160& contractHash, const Bytes& verificationScript, WitnessScope scopes)
    : Signer(contractHash, scopes), verificationScript_(verificationScript) {
}

} // namespace neocpp