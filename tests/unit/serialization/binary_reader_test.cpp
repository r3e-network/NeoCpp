#include <catch2/catch_test_macros.hpp>
#include "neocpp/serialization/binary_reader.hpp"
#include "neocpp/utils/hex.hpp"
#include <sstream>

using namespace neocpp;

TEST_CASE("binary_reader tests", "[binary_reader]") {
    SECTION("Read basic types") {
        Bytes data = {
            0xFF,                               // uint8
            0x34, 0x12,                        // uint16 (little-endian)
            0xEF, 0xBE, 0xAD, 0xDE,           // uint32 (little-endian)
            0xF0, 0xDE, 0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12  // uint64 (little-endian)
        };
        
        std::istringstream stream(std::string(data.begin(), data.end()));
        BinaryReader reader(stream);
        
        REQUIRE(reader.read<uint8_t>() == 0xFF);
        REQUIRE(reader.read<uint16_t>() == 0x1234);
        REQUIRE(reader.read<uint32_t>() == 0xDEADBEEF);
        REQUIRE(reader.read<uint64_t>() == 0x123456789ABCDEF0);
    }
    
    SECTION("Read variable length integer") {
        Bytes data = {
            0xFC,                               // 252 as single byte
            0xFD, 0xFD, 0x00,                  // 253 as 3 bytes
            0xFD, 0xFF, 0xFF,                  // 65535 as 3 bytes
            0xFE, 0x00, 0x00, 0x01, 0x00,     // 65536 as 5 bytes
        };
        
        std::istringstream stream(std::string(data.begin(), data.end()));
        BinaryReader reader(stream);
        
        REQUIRE(reader.readVarInt() == 252);
        REQUIRE(reader.readVarInt() == 253);
        REQUIRE(reader.readVarInt() == 65535);
        REQUIRE(reader.readVarInt() == 65536);
    }
    
    SECTION("Read string") {
        std::string testStr = "Hello Neo!";
        Bytes data;
        data.push_back(testStr.length());
        data.insert(data.end(), testStr.begin(), testStr.end());
        
        std::istringstream stream(std::string(data.begin(), data.end()));
        BinaryReader reader(stream);
        
        REQUIRE(reader.readVarString() == testStr);
    }
}
