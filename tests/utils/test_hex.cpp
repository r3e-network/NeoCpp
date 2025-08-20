#include <catch2/catch_test_macros.hpp>
#include "neocpp/utils/hex.hpp"
#include <string>
#include <vector>

using namespace neocpp;

TEST_CASE("Hex Tests", "[utils]") {
    
    SECTION("Encode bytes to hex string") {
        // Empty bytes
        Bytes empty;
        REQUIRE(Hex::encode(empty) == "");
        
        // Single byte
        Bytes single = {0x00};
        REQUIRE(Hex::encode(single) == "00");
        
        // Multiple bytes
        Bytes data = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
        REQUIRE(Hex::encode(data) == "0123456789abcdef");
        
        // With uppercase
        REQUIRE(Hex::encode(data, true) == "0123456789ABCDEF");
        
        // All possible byte values in first 16
        Bytes allHex = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
        REQUIRE(Hex::encode(allHex) == "000102030405060708090a0b0c0d0e0f");
        
        // Max byte value
        Bytes maxByte = {0xFF};
        REQUIRE(Hex::encode(maxByte) == "ff");
        REQUIRE(Hex::encode(maxByte, true) == "FF");
    }
    
    SECTION("Decode hex string to bytes") {
        // Empty string
        REQUIRE(Hex::decode("") == Bytes{});
        
        // Single byte
        REQUIRE(Hex::decode("00") == Bytes{0x00});
        REQUIRE(Hex::decode("ff") == Bytes{0xFF});
        REQUIRE(Hex::decode("FF") == Bytes{0xFF});
        
        // Multiple bytes
        REQUIRE(Hex::decode("0123456789abcdef") == 
                Bytes{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF});
        
        // Uppercase and lowercase mixed
        REQUIRE(Hex::decode("0123456789AbCdEf") == 
                Bytes{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF});
        
        // With 0x prefix
        REQUIRE(Hex::decode("0x1234") == Bytes{0x12, 0x34});
        REQUIRE(Hex::decode("0X1234") == Bytes{0x12, 0x34});
        
        // Long hex string (32 bytes - common for hashes)
        std::string hash256Hex = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
        Bytes hash256Bytes = Hex::decode(hash256Hex);
        REQUIRE(hash256Bytes.size() == 32);
        REQUIRE(Hex::encode(hash256Bytes) == hash256Hex);
    }
    
    SECTION("Validate hex strings") {
        // Valid hex strings
        REQUIRE(Hex::isValid(""));
        REQUIRE(Hex::isValid("00"));
        REQUIRE(Hex::isValid("FF"));
        REQUIRE(Hex::isValid("0123456789abcdef"));
        REQUIRE(Hex::isValid("0123456789ABCDEF"));
        REQUIRE(Hex::isValid("0x1234"));
        REQUIRE(Hex::isValid("0X1234"));
        
        // Invalid hex strings
        REQUIRE_FALSE(Hex::isValid("0"));          // Odd length
        REQUIRE_FALSE(Hex::isValid("123"));        // Odd length
        REQUIRE_FALSE(Hex::isValid("gg"));         // Invalid character
        REQUIRE_FALSE(Hex::isValid("0x"));         // Prefix only
        REQUIRE_FALSE(Hex::isValid("0x1"));        // Odd length after prefix
        REQUIRE_FALSE(Hex::isValid("  1234  "));   // Spaces
        REQUIRE_FALSE(Hex::isValid("12 34"));      // Space in middle
        REQUIRE_FALSE(Hex::isValid("0x0x1234"));   // Double prefix
    }
    
    SECTION("Handle hex prefix") {
        // Add prefix
        REQUIRE(Hex::withPrefix("") == "0x");
        REQUIRE(Hex::withPrefix("1234") == "0x1234");
        REQUIRE(Hex::withPrefix("0x1234") == "0x1234");  // Already has prefix
        REQUIRE(Hex::withPrefix("0X1234") == "0X1234");  // Already has prefix
        
        // Remove prefix
        REQUIRE(Hex::withoutPrefix("") == "");
        REQUIRE(Hex::withoutPrefix("1234") == "1234");      // No prefix
        REQUIRE(Hex::withoutPrefix("0x1234") == "1234");    // Remove lowercase prefix
        REQUIRE(Hex::withoutPrefix("0X1234") == "1234");    // Remove uppercase prefix
        REQUIRE(Hex::withoutPrefix("0x") == "");            // Just prefix
    }
    
    SECTION("Round-trip encoding and decoding") {
        // Test various data sizes
        std::vector<Bytes> testData = {
            {},                          // Empty
            {0x00},                      // Single zero
            {0xFF},                      // Single max
            {0x01, 0x02, 0x03},         // Small data
            Bytes(20, 0x42),            // 20 bytes (Hash160 size)
            Bytes(32, 0xAB),            // 32 bytes (Hash256 size)
            Bytes(64, 0xCD),            // 64 bytes
        };
        
        for (const auto& data : testData) {
            std::string encoded = Hex::encode(data);
            Bytes decoded = Hex::decode(encoded);
            REQUIRE(decoded == data);
            
            // Also test with uppercase
            std::string encodedUpper = Hex::encode(data, true);
            Bytes decodedUpper = Hex::decode(encodedUpper);
            REQUIRE(decodedUpper == data);
            
            // Test with prefix
            std::string withPrefix = Hex::withPrefix(encoded);
            Bytes decodedWithPrefix = Hex::decode(withPrefix);
            REQUIRE(decodedWithPrefix == data);
        }
    }
    
    SECTION("Edge cases") {
        // Decode invalid returns empty
        REQUIRE(Hex::decode("invalid").empty());
        REQUIRE(Hex::decode("12gg34").empty());
        
        // Very long hex string
        Bytes longData(256, 0x55);
        std::string longHex = Hex::encode(longData);
        REQUIRE(longHex.length() == 512);
        REQUIRE(Hex::decode(longHex) == longData);
    }
}