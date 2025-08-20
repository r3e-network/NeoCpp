#include <catch2/catch_test_macros.hpp>
#include "neocpp/utils/base58.hpp"
#include "neocpp/utils/hex.hpp"
#include <string>
#include <vector>

using namespace neocpp;

TEST_CASE("Base58 Tests", "[crypto]") {
    
    // Test vectors of strings and their Base58 encodings
    struct TestVector {
        std::string decoded;
        std::string encoded;
    };
    
    std::vector<TestVector> validVectors = {
        {"", ""},
        {" ", "Z"},
        {"-", "n"},
        {"0", "q"},
        {"1", "r"},
        {"-1", "4SU"},
        {"11", "4k8"},
        {"abc", "ZiCa"},
        {"1234598760", "3mJr7AoUXx2Wqd"},
        {"abcdefghijklmnopqrstuvwxyz", "3yxU3u1igY8WkgtjK92fbJQCd4BZiiT1v25f"},
        {"00000000000000000000000000000000000000000000000000000000000000",
         "3sN2THZeE9Eh9eYrwkvZqNstbHGvrxSAM7gXUXvyFQP8XvQLUqNCS27icwUeDT7ckHm4FUHM2mTVh1vbLmk7y"}
    };
    
    SECTION("Base58 encoding for valid strings") {
        for (const auto& tv : validVectors) {
            Bytes bytes(tv.decoded.begin(), tv.decoded.end());
            std::string result = Base58::encode(bytes);
            REQUIRE(result == tv.encoded);
        }
    }
    
    SECTION("Base58 decoding for valid strings") {
        for (const auto& tv : validVectors) {
            Bytes bytes = Base58::decode(tv.encoded);
            std::string result(bytes.begin(), bytes.end());
            REQUIRE(result == tv.decoded);
        }
    }
    
    SECTION("Base58 decoding for invalid strings") {
        std::vector<std::string> invalidStrings = {
            "0",    // Contains invalid character '0'
            "O",    // Contains invalid character 'O'
            "I",    // Contains invalid character 'I'
            "l",    // Contains invalid character 'l'
            "3mJr0", // Contains '0'
            "O3yxU", // Contains 'O'
            "3sNI",  // Contains 'I'
            "4kl8",  // Contains 'l'
            "0OIl",  // Multiple invalid characters
            "!@#$%^&*()-_=+~`" // Special characters
        };
        
        for (const auto& invalid : invalidStrings) {
            Bytes result = Base58::decode(invalid);
            REQUIRE(result.empty()); // Invalid strings should return empty bytes
        }
    }
    
    SECTION("Base58Check encoding") {
        Bytes inputData = {
            6, 161, 159, 136, 34, 110, 33, 238, 14, 79, 14, 218, 133, 13, 
            109, 40, 194, 236, 153, 44, 61, 157, 254
        };
        std::string expectedOutput = "tz1Y3qqTg9HdrzZGbEjiCPmwuZ7fWVxpPtRw";
        std::string actualOutput = Base58::encodeCheck(inputData);
        REQUIRE(actualOutput == expectedOutput);
    }
    
    SECTION("Base58Check decoding") {
        std::string inputString = "tz1Y3qqTg9HdrzZGbEjiCPmwuZ7fWVxpPtRw";
        Bytes expectedOutputData = {
            6, 161, 159, 136, 34, 110, 33, 238, 14, 79, 14, 218, 133, 13, 
            109, 40, 194, 236, 153, 44, 61, 157, 254
        };
        
        Bytes actualOutput = Base58::decodeCheck(inputString);
        REQUIRE(actualOutput == expectedOutputData);
    }
    
    SECTION("Base58Check decoding with invalid characters") {
        Bytes result = Base58::decodeCheck("0oO1lL");
        REQUIRE(result.empty()); // Should return empty for invalid characters
    }
    
    SECTION("Base58Check decoding with invalid checksum") {
        Bytes result = Base58::decodeCheck("tz1Y3qqTg9HdrzZGbEjiCPmwuZ7fWVxpPtrW");
        REQUIRE(result.empty()); // Should return empty for invalid checksum
    }
    
    SECTION("Base58 round-trip encoding and decoding") {
        // Test various data sizes
        std::vector<Bytes> testData = {
            {0x00}, // Single zero byte
            {0x01, 0x02, 0x03}, // Small data
            {0xFF, 0xFE, 0xFD, 0xFC, 0xFB}, // High value bytes
            Bytes(32, 0xAB), // 32 bytes (common for hashes)
            Bytes(64, 0xCD), // 64 bytes
        };
        
        for (const auto& data : testData) {
            std::string encoded = Base58::encode(data);
            Bytes decoded = Base58::decode(encoded);
            REQUIRE(decoded == data);
        }
    }
    
    SECTION("Base58Check round-trip encoding and decoding") {
        std::vector<Bytes> testData = {
            {0x00}, // Single zero byte
            {0x01, 0x02, 0x03}, // Small data
            {0xFF, 0xFE, 0xFD, 0xFC, 0xFB}, // High value bytes
            Bytes(20, 0x42), // 20 bytes (Hash160 size)
            Bytes(32, 0xAB), // 32 bytes (Hash256 size)
        };
        
        for (const auto& data : testData) {
            std::string encoded = Base58::encodeCheck(data);
            Bytes decoded = Base58::decodeCheck(encoded);
            REQUIRE(decoded == data);
        }
    }
    
    SECTION("Base58 encoding preserves leading zeros") {
        // In Base58, leading zero bytes are encoded as '1'
        Bytes data1 = {0x00, 0x01, 0x02};
        std::string encoded1 = Base58::encode(data1);
        REQUIRE(encoded1[0] == '1'); // Leading zero becomes '1'
        
        Bytes data2 = {0x00, 0x00, 0x01, 0x02};
        std::string encoded2 = Base58::encode(data2);
        REQUIRE(encoded2[0] == '1');
        REQUIRE(encoded2[1] == '1'); // Two leading zeros become "11"
    }
}