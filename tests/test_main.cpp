#include <catch2/catch_test_macros.hpp>
#include <neocpp/neocpp.hpp>

using namespace neocpp;

TEST_CASE("Basic SDK functionality", "[sdk]") {
    SECTION("Constants are defined") {
        REQUIRE(NeoConstants::HASH160_SIZE == 20);
        REQUIRE(NeoConstants::HASH256_SIZE == 32);
        REQUIRE(NeoConstants::PRIVATE_KEY_SIZE == 32);
        REQUIRE(NeoConstants::PUBLIC_KEY_SIZE_COMPRESSED == 33);
    }
    
    SECTION("Can create Hash160") {
        Hash160 hash;
        REQUIRE(hash.toArray().size() == 20);
    }
    
    SECTION("Can create Hash256") {
        Hash256 hash;
        REQUIRE(hash.toArray().size() == 32);
    }
}