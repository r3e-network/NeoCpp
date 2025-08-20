#include "neocpp/crypto/wif.hpp"
#include "neocpp/utils/base58.hpp"
#include "neocpp/neo_constants.hpp"
#include "neocpp/exceptions.hpp"

namespace neocpp {

// Define static constants
const uint8_t WIF::WIF_VERSION;
const uint8_t WIF::COMPRESSED_FLAG;

std::string WIF::encode(const Bytes& privateKey) {
    if (privateKey.size() != NeoConstants::PRIVATE_KEY_SIZE) {
        throw IllegalArgumentException("Private key must be 32 bytes");
    }
    
    Bytes data;
    data.push_back(WIF_VERSION);
    data.insert(data.end(), privateKey.begin(), privateKey.end());
    data.push_back(COMPRESSED_FLAG);
    
    return Base58::encodeCheck(data);
}

Bytes WIF::decode(const std::string& wif) {
    Bytes decoded = Base58::decodeCheck(wif);
    
    if (decoded.empty() || decoded.size() != 34 || decoded[0] != WIF_VERSION || decoded[33] != COMPRESSED_FLAG) {
        throw CryptoException("Invalid WIF string");
    }
    
    return Bytes(decoded.begin() + 1, decoded.begin() + 33);
}

bool WIF::isValid(const std::string& wif) {
    if (wif.empty() || wif.length() != 52) {
        return false;
    }
    
    Bytes decoded = Base58::decodeCheck(wif);
    return !decoded.empty() && 
           decoded.size() == 34 && 
           decoded[0] == WIF_VERSION && 
           decoded[33] == COMPRESSED_FLAG;
}

} // namespace neocpp