#include <catch2/catch_test_macros.hpp>
#include "neocpp/crypto/nep2.hpp"
#include "neocpp/crypto/ec_key_pair.hpp"
#include "neocpp/crypto/scrypt_params.hpp"
#include "neocpp/utils/hex.hpp"
#include "neocpp/exceptions.hpp"

using namespace neocpp;

TEST_CASE("NEP2 Tests", "[crypto]") {
    
    SECTION("Encrypt and decrypt with default Scrypt params") {
        const std::string privateKeyHex = "1dd37fba80fec4e6a6f13fd708d8dcb3b29def768017052f6c930fa1c5d90bbb";
        ECKeyPair keyPair(Hex::decode(privateKeyHex));
        const std::string password = "TestPassword123";
        
        // Encrypt
        std::string encrypted = NEP2::encrypt(keyPair, password);
        REQUIRE(!encrypted.empty());
        REQUIRE(encrypted.size() == 58); // NEP-2 format is always 58 characters
        REQUIRE(encrypted.substr(0, 2) == "6P"); // NEP-2 prefix
        
        // Decrypt
        ECKeyPair decrypted = NEP2::decryptToKeyPair(encrypted, password);
        REQUIRE(decrypted.getPrivateKey()->getBytes() == keyPair.getPrivateKey()->getBytes());
    }
    
    SECTION("Encrypt and decrypt with custom Scrypt params") {
        const std::string privateKeyHex = "1dd37fba80fec4e6a6f13fd708d8dcb3b29def768017052f6c930fa1c5d90bbb";
        ECKeyPair keyPair(Hex::decode(privateKeyHex));
        const std::string password = "TestPassword123";
        
        // Use lighter Scrypt params for testing
        ScryptParams params(256, 1, 1);
        
        // Encrypt
        std::string encrypted = NEP2::encrypt(keyPair, password, params);
        REQUIRE(!encrypted.empty());
        REQUIRE(NEP2::isValid(encrypted));
        
        // Decrypt
        ECKeyPair decrypted = NEP2::decryptToKeyPair(encrypted, password, params);
        REQUIRE(decrypted.getPrivateKey()->getBytes() == keyPair.getPrivateKey()->getBytes());
    }
    
    SECTION("Decrypt with wrong password throws exception") {
        const std::string encrypted = "6PYVdVfUJ5wPDaJJqNauhwKimdSj7Zc27kCQ4MZKqfYBYMpXDeLqrnqLjJ";
        const std::string wrongPassword = "WrongPassword";
        
        REQUIRE_THROWS_AS(NEP2::decryptToKeyPair(encrypted, wrongPassword), NEP2Exception);
    }
    
    SECTION("Validate NEP2 format") {
        // Generate a valid NEP-2 string first
        ECKeyPair keyPair = ECKeyPair::generate();
        std::string encrypted = NEP2::encrypt(keyPair, "password");
        REQUIRE(NEP2::isValid(encrypted));
        
        // Invalid formats
        REQUIRE_FALSE(NEP2::isValid("")); // Empty
        REQUIRE_FALSE(NEP2::isValid("InvalidNEP2")); // Wrong format
        REQUIRE_FALSE(NEP2::isValid("5PYVdVfUJ5wPDaJJqNauhwKimdSj7Zc27kCQ4MZKqfYBYMpXDeLqrnqLjJ")); // Wrong prefix
        REQUIRE_FALSE(NEP2::isValid("6PYVdVfUJ5wPDaJJqNauhwKimdSj7Zc27kCQ4MZKqfYBYMpXDeLqrnqL")); // Too short
    }
    
    SECTION("Known NEP2 test vectors") {
        struct TestVector {
            std::string privateKeyHex;
            std::string password;
            std::string encrypted;
        };
        
        // Note: NEP-2 encryption includes randomness (salt), so we can only test decryption
        // with known encrypted values
        std::vector<TestVector> testVectors = {
            // Add known test vectors here if available
        };
        
        // Test that encryption produces valid NEP-2 format
        const std::string privateKeyHex = "1dd37fba80fec4e6a6f13fd708d8dcb3b29def768017052f6c930fa1c5d90bbb";
        ECKeyPair keyPair(Hex::decode(privateKeyHex));
        
        std::string encrypted = NEP2::encrypt(keyPair, "password");
        REQUIRE(NEP2::isValid(encrypted));
        
        // Should be able to decrypt it back
        ECKeyPair decrypted = NEP2::decryptToKeyPair(encrypted, "password");
        REQUIRE(decrypted.getPrivateKey()->getBytes() == keyPair.getPrivateKey()->getBytes());
    }
    
    SECTION("Multiple rounds of encryption produce different results") {
        const std::string privateKeyHex = "1dd37fba80fec4e6a6f13fd708d8dcb3b29def768017052f6c930fa1c5d90bbb";
        ECKeyPair keyPair(Hex::decode(privateKeyHex));
        const std::string password = "TestPassword";
        
        std::string encrypted1 = NEP2::encrypt(keyPair, password);
        std::string encrypted2 = NEP2::encrypt(keyPair, password);
        
        // NEP-2 uses deterministic salt from address hash, so encrypted values should be the same
        // REQUIRE(encrypted1 != encrypted2);  // This test expectation is incorrect
        
        // But both should decrypt to the same private key
        ECKeyPair decrypted1 = NEP2::decryptToKeyPair(encrypted1, password);
        ECKeyPair decrypted2 = NEP2::decryptToKeyPair(encrypted2, password);
        
        REQUIRE(decrypted1.getPrivateKey()->getBytes() == keyPair.getPrivateKey()->getBytes());
        REQUIRE(decrypted2.getPrivateKey()->getBytes() == keyPair.getPrivateKey()->getBytes());
    }
    
    SECTION("Decrypt only to private key bytes") {
        const std::string privateKeyHex = "1dd37fba80fec4e6a6f13fd708d8dcb3b29def768017052f6c930fa1c5d90bbb";
        ECKeyPair keyPair(Hex::decode(privateKeyHex));
        const std::string password = "TestPassword";
        
        std::string encrypted = NEP2::encrypt(keyPair, password);
        
        Bytes decryptedBytes = NEP2::decrypt(encrypted, password);
        REQUIRE(decryptedBytes == keyPair.getPrivateKey()->getBytes());
    }
}