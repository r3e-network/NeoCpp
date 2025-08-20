#include "neocpp/utils/address.hpp"
#include "neocpp/utils/base58.hpp"
#include "neocpp/crypto/hash.hpp"
#include "neocpp/neo_constants.hpp"
#include "neocpp/exceptions.hpp"

namespace neocpp {

std::string AddressUtils::scriptHashToAddress(const Bytes& scriptHash) {
    if (scriptHash.size() != NeoConstants::HASH160_SIZE) {
        throw IllegalArgumentException("Script hash must be 20 bytes");
    }
    
    Bytes data;
    data.push_back(getAddressVersion());
    data.insert(data.end(), scriptHash.begin(), scriptHash.end());
    
    return Base58::encodeCheck(data);
}

Bytes AddressUtils::addressToScriptHash(const std::string& address) {
    if (!isValidAddress(address)) {
        throw IllegalArgumentException("Invalid Neo address");
    }
    
    Bytes decoded = Base58::decodeCheck(address);
    
    if (decoded.empty() || decoded[0] != getAddressVersion()) {
        throw IllegalArgumentException("Invalid address version");
    }
    
    return Bytes(decoded.begin() + 1, decoded.end());
}

bool AddressUtils::isValidAddress(const std::string& address) {
    if (address.empty() || address.length() != 34) {
        return false;
    }
    
    try {
        Bytes decoded = Base58::decodeCheck(address);
        return !decoded.empty() && 
               decoded.size() == 21 && 
               decoded[0] == getAddressVersion();
    } catch (...) {
        return false;
    }
}

uint8_t AddressUtils::getAddressVersion() {
    return NeoConstants::ADDRESS_VERSION;
}

} // namespace neocpp