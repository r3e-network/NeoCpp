#include <catch2/catch_test_macros.hpp>
#include <neocpp/neocpp.hpp>

using namespace neocpp;

TEST_CASE("Type conversions", "[types]") {
    SECTION("Hex encoding/decoding") {
        Bytes original = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
        std::string hex = Hex::encode(original);
        REQUIRE(hex == "0123456789abcdef");
        
        Bytes decoded = Hex::decode(hex);
        REQUIRE(decoded == original);
    }
    
    SECTION("Hex with prefix") {
        std::string hex = "abcdef";
        std::string withPrefix = Hex::withPrefix(hex);
        REQUIRE(withPrefix == "0xabcdef");
        
        std::string withoutPrefix = Hex::withoutPrefix(withPrefix);
        REQUIRE(withoutPrefix == "abcdef");
    }
    
    SECTION("Base64 encoding/decoding") {
        Bytes original = {0x01, 0x02, 0x03, 0x04};
        std::string encoded = Base64::encode(original);
        REQUIRE(!encoded.empty());
        
        Bytes decoded = Base64::decode(encoded);
        REQUIRE(decoded == original);
    }
}