#include <catch2/catch_test_macros.hpp>
#include "neocpp/types/hash160.hpp"
#include "neocpp/crypto/ec_key_pair.hpp"
#include "neocpp/utils/hex.hpp"
#include "neocpp/exceptions.hpp"

using namespace neocpp;

TEST_CASE("Hash160 Tests", "[types]") {
    
    SECTION("Create Hash160 from bytes") {
        Bytes hashBytes(20, 0xAB);
        Hash160 hash(hashBytes);
        
        REQUIRE(hash.toArray().size() == 20);
        REQUIRE(hash.toArray() == hashBytes);
    }
    
    SECTION("Create Hash160 from hex string") {
        const std::string hashHex = "23ba2703c53263e8d6e522dc32203339dcd8eee9";
        Hash160 hash(hashHex);
        
        REQUIRE(hash.toString() == hashHex);
        REQUIRE(hash.toArray() == Hex::decode(hashHex));
    }
    
    SECTION("Create Hash160 from hex string with 0x prefix") {
        const std::string hashHex = "0x23ba2703c53263e8d6e522dc32203339dcd8eee9";
        Hash160 hash(hashHex);
        
        REQUIRE(hash.toString() == hashHex.substr(2));
    }
    
    SECTION("Invalid Hash160 size throws exception") {
        Bytes tooShort(19, 0xAB);
        REQUIRE_THROWS_AS(Hash160(tooShort), IllegalArgumentException);
        
        Bytes tooLong(21, 0xAB);
        REQUIRE_THROWS_AS(Hash160(tooLong), IllegalArgumentException);
    }
    
    SECTION("Zero Hash160") {
        Hash160 zero = Hash160::ZERO;
        
        REQUIRE(zero.toString() == "0000000000000000000000000000000000000000");
        REQUIRE(zero.toArray() == Bytes(20, 0x00));
    }
    
    SECTION("Hash160 from public key") {
        const std::string publicKeyHex = "031a8c1c0e2f3b73366c6173f09d2a9684cc5ea4cf23076f584b217be9ca9b8358";
        Bytes publicKey = Hex::decode(publicKeyHex);
        
        Hash160 hash = Hash160::fromPublicKey(publicKey);
        REQUIRE(hash.toArray().size() == 20);
        
        // The hash should be deterministic
        Hash160 hash2 = Hash160::fromPublicKey(publicKey);
        REQUIRE(hash == hash2);
    }
    
    SECTION("Hash160 to address") {
        const std::string hashHex = "23ba2703c53263e8d6e522dc32203339dcd8eee9";
        Hash160 hash(hashHex);
        
        std::string address = hash.toAddress();
        REQUIRE(!address.empty());
        REQUIRE(address[0] == 'N'); // Neo N3 address prefix
    }
    
    SECTION("Hash160 from address") {
        const std::string address = "NZs2zXSPuuv9ZF6TDGSWT1RBmE8rfGj7UW";
        Hash160 hash = Hash160::fromAddress(address);
        
        // Convert back to address should give the same result
        REQUIRE(hash.toAddress() == address);
    }
    
    SECTION("Invalid address throws exception") {
        REQUIRE_THROWS_AS(Hash160::fromAddress("InvalidAddress"), IllegalArgumentException);
        REQUIRE_THROWS_AS(Hash160::fromAddress(""), IllegalArgumentException);
    }
    
    SECTION("Hash160 equality") {
        const std::string hashHex = "23ba2703c53263e8d6e522dc32203339dcd8eee9";
        Hash160 hash1(hashHex);
        Hash160 hash2(hashHex);
        
        REQUIRE(hash1 == hash2);
        
        Hash160 hash3("0000000000000000000000000000000000000000");
        REQUIRE_FALSE(hash1 == hash3);
    }
    
    SECTION("Hash160 comparison") {
        Hash160 hash1("0000000000000000000000000000000000000001");
        Hash160 hash2("0000000000000000000000000000000000000002");
        
        REQUIRE(hash1 < hash2);
        REQUIRE(hash2 > hash1);
        REQUIRE(hash1 <= hash2);
        REQUIRE(hash2 >= hash1);
    }
    
    SECTION("Hash160 serialization") {
        const std::string hashHex = "23ba2703c53263e8d6e522dc32203339dcd8eee9";
        Hash160 hash(hashHex);
        
        // Test size
        REQUIRE(hash.getSize() == 20);
        
        // Test little-endian conversion
        Bytes littleEndian = hash.toLittleEndianArray();
        REQUIRE(littleEndian.size() == 20);
        
        // First byte should be last byte of big-endian
        REQUIRE(littleEndian[0] == 0xe9);
        REQUIRE(littleEndian[19] == 0x23);
    }
    
    SECTION("Hash160 from multiple public keys (multi-sig)") {
        std::vector<SharedPtr<ECPublicKey>> publicKeys;
        
        // Create 3 public keys
        for (int i = 0; i < 3; i++) {
            ECKeyPair keyPair = ECKeyPair::generate();
            publicKeys.push_back(keyPair.getPublicKey());
        }
        
        // Create multi-sig script hash with threshold 2
        Hash160 multiSigHash = Hash160::fromPublicKeys(publicKeys, 2);
        
        REQUIRE(multiSigHash.toArray().size() == 20);
        
        // Same keys and threshold should produce same hash
        Hash160 multiSigHash2 = Hash160::fromPublicKeys(publicKeys, 2);
        REQUIRE(multiSigHash == multiSigHash2);
        
        // Different threshold should produce different hash
        Hash160 multiSigHash3 = Hash160::fromPublicKeys(publicKeys, 1);
        REQUIRE_FALSE(multiSigHash == multiSigHash3);
    }
}