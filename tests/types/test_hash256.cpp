#include <catch2/catch_test_macros.hpp>
#include <unordered_map>
#include "neocpp/types/hash256.hpp"
#include "neocpp/serialization/binary_writer.hpp"
#include "neocpp/serialization/binary_reader.hpp"
#include "neocpp/utils/hex.hpp"
#include "neocpp/exceptions.hpp"

using namespace neocpp;

TEST_CASE("Hash256 Tests", "[types]") {
    
    SECTION("Create Hash256 from bytes") {
        Bytes hashBytes(32, 0xAB);
        Hash256 hash(hashBytes);
        
        REQUIRE(hash.toArray().size() == 32);
        REQUIRE(hash.toArray() == hashBytes);
    }
    
    SECTION("Create Hash256 from hex string") {
        const std::string hashHex = "e8c2a6a6453097f1acf66e0d40f06a856a99f9b9e58e970f1377add726d0a632";
        Hash256 hash(hashHex);
        
        REQUIRE(hash.toString() == hashHex);
        REQUIRE(hash.toArray() == Hex::decode(hashHex));
    }
    
    SECTION("Create Hash256 from hex string with 0x prefix") {
        const std::string hashHex = "0xe8c2a6a6453097f1acf66e0d40f06a856a99f9b9e58e970f1377add726d0a632";
        Hash256 hash(hashHex);
        
        REQUIRE(hash.toString() == hashHex.substr(2));
    }
    
    SECTION("Invalid Hash256 size throws exception") {
        Bytes tooShort(31, 0xAB);
        REQUIRE_THROWS_AS(Hash256(tooShort), IllegalArgumentException);
        
        Bytes tooLong(33, 0xAB);
        REQUIRE_THROWS_AS(Hash256(tooLong), IllegalArgumentException);
    }
    
    SECTION("Zero Hash256") {
        Hash256 zero = Hash256::ZERO;
        
        REQUIRE(zero.toString() == "0000000000000000000000000000000000000000000000000000000000000000");
        REQUIRE(zero.toArray() == Bytes(32, 0x00));
    }
    
    SECTION("Hash256 equality") {
        const std::string hashHex = "e8c2a6a6453097f1acf66e0d40f06a856a99f9b9e58e970f1377add726d0a632";
        Hash256 hash1(hashHex);
        Hash256 hash2(hashHex);
        
        REQUIRE(hash1 == hash2);
        
        Hash256 hash3("0000000000000000000000000000000000000000000000000000000000000000");
        REQUIRE_FALSE(hash1 == hash3);
    }
    
    SECTION("Hash256 comparison") {
        Hash256 hash1("0000000000000000000000000000000000000000000000000000000000000001");
        Hash256 hash2("0000000000000000000000000000000000000000000000000000000000000002");
        
        REQUIRE(hash1 < hash2);
        REQUIRE(hash2 > hash1);
        REQUIRE(hash1 <= hash2);
        REQUIRE(hash2 >= hash1);
    }
    
    SECTION("Hash256 serialization") {
        const std::string hashHex = "e8c2a6a6453097f1acf66e0d40f06a856a99f9b9e58e970f1377add726d0a632";
        Hash256 hash(hashHex);
        
        // Test size
        REQUIRE(hash.getSize() == 32);
        
        // Test little-endian conversion
        Bytes littleEndian = hash.toLittleEndianArray();
        REQUIRE(littleEndian.size() == 32);
        
        // First byte should be last byte of big-endian
        REQUIRE(littleEndian[0] == 0x32);
        REQUIRE(littleEndian[31] == 0xe8);
    }
    
    SECTION("Hash256 copy and move semantics") {
        const std::string hashHex = "e8c2a6a6453097f1acf66e0d40f06a856a99f9b9e58e970f1377add726d0a632";
        
        // Copy constructor
        Hash256 hash1(hashHex);
        Hash256 hash2(hash1);
        REQUIRE(hash1 == hash2);
        
        // Move constructor
        Hash256 hash3(std::move(hash2));
        REQUIRE(hash3.toString() == hashHex);
        
        // Copy assignment
        Hash256 hash4 = Hash256::ZERO;
        hash4 = hash1;
        REQUIRE(hash4 == hash1);
        
        // Move assignment
        Hash256 hash5 = Hash256::ZERO;
        hash5 = std::move(hash4);
        REQUIRE(hash5.toString() == hashHex);
    }
    
    SECTION("Hash256 as map key") {
        std::unordered_map<Hash256, std::string, Hash256::Hasher> hashMap;
        
        Hash256 hash1("e8c2a6a6453097f1acf66e0d40f06a856a99f9b9e58e970f1377add726d0a632");
        Hash256 hash2("0000000000000000000000000000000000000000000000000000000000000000");
        
        hashMap[hash1] = "first";
        hashMap[hash2] = "second";
        
        REQUIRE(hashMap[hash1] == "first");
        REQUIRE(hashMap[hash2] == "second");
        REQUIRE(hashMap.size() == 2);
        
        // Same hash should map to same value
        Hash256 hash3("e8c2a6a6453097f1acf66e0d40f06a856a99f9b9e58e970f1377add726d0a632");
        REQUIRE(hashMap[hash3] == "first");
    }
    
    SECTION("Hash256 binary reader/writer compatibility") {
        const std::string hashHex = "e8c2a6a6453097f1acf66e0d40f06a856a99f9b9e58e970f1377add726d0a632";
        Hash256 original(hashHex);
        
        // Write to buffer
        BinaryWriter writer;
        original.serialize(writer);
        Bytes buffer = writer.toArray();
        
        // Read back
        BinaryReader reader(buffer);
        Hash256 deserialized = Hash256::deserialize(reader);
        
        REQUIRE(original == deserialized);
    }
}