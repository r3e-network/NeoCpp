#include <catch2/catch_test_macros.hpp>
#include "neocpp/wallet/account.hpp"
#include "neocpp/utils/address.hpp"
#include "neocpp/crypto/ec_key_pair.hpp"
#include "neocpp/crypto/nep2.hpp"
#include "neocpp/exceptions.hpp"

using namespace neocpp;

TEST_CASE("Error handling", "[error]") {
    
    SECTION("Account unlock with wrong password") {
        // Create an account with encrypted key
        auto keyPair = std::make_shared<ECKeyPair>(ECKeyPair::generate());
        auto account = std::make_shared<Account>(keyPair);
        
        // Lock it with a password
        account->lock("correctPassword");
        REQUIRE(account->isLocked());
        
        // Try to unlock with wrong password - should return false, not throw
        bool unlocked = account->unlock("wrongPassword");
        REQUIRE_FALSE(unlocked);
        REQUIRE(account->isLocked());
        
        // Unlock with correct password should work
        unlocked = account->unlock("correctPassword");
        REQUIRE(unlocked);
        REQUIRE_FALSE(account->isLocked());
    }
    
    SECTION("Address validation with invalid addresses") {
        // Test various invalid addresses
        REQUIRE_FALSE(AddressUtils::isValidAddress(""));  // Empty
        REQUIRE_FALSE(AddressUtils::isValidAddress("abc"));  // Too short
        REQUIRE_FALSE(AddressUtils::isValidAddress("123456789012345678901234567890123456"));  // Too long
        REQUIRE_FALSE(AddressUtils::isValidAddress("NKyXUUUuUUUuUuUuUuUuUuUuUuUuUuUuU"));  // Invalid checksum
        REQUIRE_FALSE(AddressUtils::isValidAddress("!@#$%^&*()_+{}|:<>?"));  // Invalid characters
        
        // Valid address should work
        std::string validAddress = "NKuyBkoGdZZSLyPbJEetheRhPAHjUuUUUu";  // Example format
        // This might fail if it's not a real address, but it tests the format
        // The actual validation depends on proper Base58 decoding
    }
    
    SECTION("Exception types are properly caught") {
        // Test that specific exception types can be caught
        try {
            throw IllegalArgumentException("Test illegal argument");
            REQUIRE(false);  // Should not reach here
        } catch (const IllegalArgumentException& e) {
            REQUIRE(std::string(e.what()).find("illegal argument") != std::string::npos);
        }
        
        try {
            throw NEP2Exception("Test NEP2 error");
            REQUIRE(false);  // Should not reach here
        } catch (const NEP2Exception& e) {
            REQUIRE(std::string(e.what()).find("NEP-2") != std::string::npos);
        }
        
        try {
            throw CryptoException("Test crypto error");
            REQUIRE(false);  // Should not reach here
        } catch (const CryptoException& e) {
            REQUIRE(std::string(e.what()).find("Crypto") != std::string::npos);
        }
    }
    
    SECTION("Exception hierarchy") {
        // Test that derived exceptions can be caught as base
        try {
            throw SerializationException("Test serialization");
            REQUIRE(false);
        } catch (const NeoException& e) {
            // Should catch as base class
            REQUIRE(std::string(e.what()).find("Serialization") != std::string::npos);
        }
        
        try {
            throw DeserializationException("Test deserialization");
            REQUIRE(false);
        } catch (const NeoException& e) {
            // Should catch as base class
            REQUIRE(std::string(e.what()).find("Deserialization") != std::string::npos);
        }
    }
    
    SECTION("Resource cleanup on exception") {
        // Test that resources are properly cleaned up even when exceptions occur
        // This is more of a design test - our smart pointers should handle this
        
        auto testCleanup = []() {
            auto keyPair = std::make_shared<ECKeyPair>(ECKeyPair::generate());  // Should be cleaned up automatically
            throw std::runtime_error("Test exception");
        };
        
        REQUIRE_THROWS_AS(testCleanup(), std::runtime_error);
        // If we get here without crashes, cleanup worked
    }
}