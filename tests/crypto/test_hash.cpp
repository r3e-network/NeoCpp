#include <catch2/catch_test_macros.hpp>
#include "neocpp/crypto/hash.hpp"
#include "neocpp/utils/hex.hpp"

using namespace neocpp;

TEST_CASE("Hash Tests", "[crypto]") {
    
    SECTION("SHA256 hashing") {
        Bytes input = {0x01, 0x02, 0x03, 0x04};
        Bytes result = HashUtils::sha256(input);
        
        REQUIRE(result.size() == 32);
        const std::string expected = "9f64a747e1b97f131fabb6b447296c9b6f0201e79fb3c5356e6c77e89b6a806a";
        REQUIRE(Hex::encode(result) == expected);
    }
    
    SECTION("SHA256 empty input") {
        Bytes input;
        Bytes result = HashUtils::sha256(input);
        
        REQUIRE(result.size() == 32);
        const std::string expected = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
        REQUIRE(Hex::encode(result) == expected);
    }
    
    SECTION("SHA256 string input") {
        std::string input = "Hello, Neo!";
        Bytes inputBytes(input.begin(), input.end());
        Bytes result = HashUtils::sha256(inputBytes);
        
        REQUIRE(result.size() == 32);
    }
    
    SECTION("Double SHA256") {
        Bytes input = {0x01, 0x02, 0x03, 0x04};
        Bytes result = HashUtils::doubleSha256(input);
        
        REQUIRE(result.size() == 32);
        // doubleSha256 is SHA256(SHA256(input))
        Bytes intermediate = HashUtils::sha256(input);
        Bytes expected = HashUtils::sha256(intermediate);
        REQUIRE(result == expected);
    }
    
    SECTION("RIPEMD160 hashing") {
        Bytes input = {0x01, 0x02, 0x03, 0x04};
        Bytes result = HashUtils::ripemd160(input);
        
        REQUIRE(result.size() == 20);
        const std::string expected = "179bb366e5e224b8bf4ce302cefc5744961839c5";
        REQUIRE(Hex::encode(result) == expected);
    }
    
    SECTION("RIPEMD160 empty input") {
        Bytes input;
        Bytes result = HashUtils::ripemd160(input);
        
        REQUIRE(result.size() == 20);
        const std::string expected = "9c1185a5c5e9fc54612808977ee8f548b2258d31";
        REQUIRE(Hex::encode(result) == expected);
    }
    
    SECTION("Hash160 (SHA256 + RIPEMD160)") {
        Bytes input = {0x01, 0x02, 0x03, 0x04};
        Bytes result = HashUtils::sha256ThenRipemd160(input);
        
        REQUIRE(result.size() == 20);
        // sha256ThenRipemd160 is RIPEMD160(SHA256(input))
        Bytes intermediate = HashUtils::sha256(input);
        Bytes expected = HashUtils::ripemd160(intermediate);
        REQUIRE(result == expected);
    }
    
    SECTION("SHA256 large input") {
        Bytes input(1000, 0xFF);
        Bytes result = HashUtils::sha256(input);
        
        REQUIRE(result.size() == 32);
        // Verify it doesn't crash with large input
    }
    
    SECTION("RIPEMD160 large input") {
        Bytes input(1000, 0xFF);
        Bytes result = HashUtils::ripemd160(input);
        
        REQUIRE(result.size() == 20);
        // Verify it doesn't crash with large input
    }
    
    SECTION("Hash consistency") {
        Bytes input = {0x01, 0x02, 0x03, 0x04};
        
        // Same input should produce same output
        Bytes result1 = HashUtils::sha256(input);
        Bytes result2 = HashUtils::sha256(input);
        REQUIRE(result1 == result2);
        
        Bytes result3 = HashUtils::ripemd160(input);
        Bytes result4 = HashUtils::ripemd160(input);
        REQUIRE(result3 == result4);
    }
}