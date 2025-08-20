#include <catch2/catch_test_macros.hpp>
#include "neocpp/utils/base64.hpp"
#include "neocpp/utils/hex.hpp"
#include <string>
#include <vector>

using namespace neocpp;

TEST_CASE("Base64 Tests", "[crypto]") {
    
    // Test vectors from Swift tests
    const std::string inputHex = "150c14242dbf5e2f6ac2568b59b7822278d571b75f17be0c14242dbf5e2f6ac2568b59b7822278d571b75f17be13c00c087472616e736665720c14897720d8cd76f4f00abfa37c0edd889c208fde9b41627d5b5238";
    const std::string expectedBase64 = "FQwUJC2/Xi9qwlaLWbeCInjVcbdfF74MFCQtv14vasJWi1m3giJ41XG3Xxe+E8AMCHRyYW5zZmVyDBSJdyDYzXb08Aq/o3wO3YicII/em0FifVtSOA==";
    
    SECTION("Base64 encode from bytes") {
        Bytes inputBytes = Hex::decode(inputHex);
        std::string encoded = Base64::encode(inputBytes);
        REQUIRE(encoded == expectedBase64);
    }
    
    SECTION("Base64 decode to bytes") {
        Bytes decoded = Base64::decode(expectedBase64);
        std::string decodedHex = Hex::encode(decoded);
        REQUIRE(decodedHex == inputHex);
    }
    
    SECTION("Round-trip encoding and decoding") {
        // Test various data sizes
        std::vector<Bytes> testData = {
            {},                              // Empty
            {0x00},                          // Single zero
            {0xFF},                          // Single max
            {0x01, 0x02, 0x03},             // Small data
            {0x00, 0x01, 0x02, 0x03, 0x04}, // 5 bytes
            Bytes(20, 0x42),                // 20 bytes (Hash160 size)
            Bytes(32, 0xAB),                // 32 bytes (Hash256 size)
            Bytes(64, 0xCD),                // 64 bytes
            Bytes(100, 0xEF),               // Non-standard size
        };
        
        for (const auto& data : testData) {
            std::string encoded = Base64::encode(data);
            Bytes decoded = Base64::decode(encoded);
            REQUIRE(decoded == data);
        }
    }
    
    SECTION("Common Base64 test vectors") {
        struct TestVector {
            std::string plain;
            std::string encoded;
        };
        
        std::vector<TestVector> vectors = {
            {"", ""},
            {"f", "Zg=="},
            {"fo", "Zm8="},
            {"foo", "Zm9v"},
            {"foob", "Zm9vYg=="},
            {"fooba", "Zm9vYmE="},
            {"foobar", "Zm9vYmFy"},
            {"Man", "TWFu"},
            {"Ma", "TWE="},
            {"M", "TQ=="},
            {"pleasure.", "cGxlYXN1cmUu"},
            {"leasure.", "bGVhc3VyZS4="},
            {"easure.", "ZWFzdXJlLg=="},
            {"asure.", "YXN1cmUu"},
            {"sure.", "c3VyZS4="},
        };
        
        for (const auto& tv : vectors) {
            Bytes plainBytes(tv.plain.begin(), tv.plain.end());
            
            // Test encoding
            std::string encoded = Base64::encode(plainBytes);
            REQUIRE(encoded == tv.encoded);
            
            // Test decoding
            Bytes decoded = Base64::decode(tv.encoded);
            std::string decodedStr(decoded.begin(), decoded.end());
            REQUIRE(decodedStr == tv.plain);
        }
    }
    
    SECTION("Validate Base64 strings") {
        // Valid Base64 strings
        REQUIRE(Base64::isValid(""));
        REQUIRE(Base64::isValid("Zg=="));
        REQUIRE(Base64::isValid("Zm8="));
        REQUIRE(Base64::isValid("Zm9v"));
        REQUIRE(Base64::isValid("Zm9vYmFy"));
        REQUIRE(Base64::isValid("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"));
        REQUIRE(Base64::isValid("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_")); // URL safe
        
        // Invalid Base64 strings
        REQUIRE_FALSE(Base64::isValid("Zg="));      // Wrong padding
        REQUIRE_FALSE(Base64::isValid("Zg==="));    // Too much padding
        REQUIRE_FALSE(Base64::isValid("!@#$"));     // Invalid characters
        REQUIRE_FALSE(Base64::isValid("Zm9 v"));    // Space in middle
        REQUIRE_FALSE(Base64::isValid("===="));     // Only padding
    }
    
    SECTION("Edge cases") {
        // Very long data
        Bytes longData(1024, 0x55);
        std::string longEncoded = Base64::encode(longData);
        Bytes longDecoded = Base64::decode(longEncoded);
        REQUIRE(longDecoded == longData);
        
        // All possible byte values
        Bytes allBytes;
        for (int i = 0; i < 256; i++) {
            allBytes.push_back(static_cast<uint8_t>(i));
        }
        std::string allEncoded = Base64::encode(allBytes);
        Bytes allDecoded = Base64::decode(allEncoded);
        REQUIRE(allDecoded == allBytes);
        
        // Decode invalid returns empty
        REQUIRE(Base64::decode("!@#$%^&*").empty());
        REQUIRE(Base64::decode("Zg===").empty());
    }
    
    SECTION("Padding edge cases") {
        // Test correct padding for different input sizes
        Bytes oneByteInput = {0x41};  // 'A'
        std::string oneByteEncoded = Base64::encode(oneByteInput);
        REQUIRE(oneByteEncoded == "QQ==");
        REQUIRE(Base64::decode(oneByteEncoded) == oneByteInput);
        
        Bytes twoByteInput = {0x41, 0x42};  // 'AB'
        std::string twoByteEncoded = Base64::encode(twoByteInput);
        REQUIRE(twoByteEncoded == "QUI=");
        REQUIRE(Base64::decode(twoByteEncoded) == twoByteInput);
        
        Bytes threeByteInput = {0x41, 0x42, 0x43};  // 'ABC'
        std::string threeByteEncoded = Base64::encode(threeByteInput);
        REQUIRE(threeByteEncoded == "QUJD");
        REQUIRE(Base64::decode(threeByteEncoded) == threeByteInput);
    }
}