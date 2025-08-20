#include <catch2/catch_test_macros.hpp>
#include <neocpp/neocpp.hpp>

using namespace neocpp;

TEST_CASE("Hash functionality", "[crypto]") {
    SECTION("SHA256 hashing") {
        Bytes data = {0x01, 0x02, 0x03};
        Bytes hash = HashUtils::sha256(data);
        REQUIRE(hash.size() == 32);
    }
    
    SECTION("Double SHA256") {
        Bytes data = {0x01, 0x02, 0x03};
        Bytes hash = HashUtils::doubleSha256(data);
        REQUIRE(hash.size() == 32);
    }
    
    SECTION("RIPEMD160") {
        Bytes data = {0x01, 0x02, 0x03};
        Bytes hash = HashUtils::ripemd160(data);
        REQUIRE(hash.size() == 20);
    }
    
    SECTION("SHA256 then RIPEMD160") {
        Bytes data = {0x01, 0x02, 0x03};
        Bytes hash = HashUtils::sha256ThenRipemd160(data);
        REQUIRE(hash.size() == 20);
    }
}