#include <catch2/catch_test_macros.hpp>
#include "neocpp/serialization/binary_reader.hpp"
#include "neocpp/exceptions.hpp"
#include "neocpp/utils/hex.hpp"

using namespace neocpp;

TEST_CASE("BinaryReader Tests", "[serialization]") {
    
    SECTION("Constructor from bytes vector") {
        Bytes data = {0x01, 0x02, 0x03, 0x04, 0x05};
        BinaryReader reader(data);
        
        REQUIRE(reader.position() == 0);
        REQUIRE(reader.hasMore());
        REQUIRE(reader.remaining() == 5);
    }
    
    SECTION("Constructor from raw data") {
        uint8_t data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
        BinaryReader reader(data, 5);
        
        REQUIRE(reader.position() == 0);
        REQUIRE(reader.hasMore());
        REQUIRE(reader.remaining() == 5);
    }
    
    SECTION("Read single byte") {
        Bytes data = {0x42, 0xFF, 0x00};
        BinaryReader reader(data);
        
        REQUIRE(reader.readByte() == 0x42);
        REQUIRE(reader.readByte() == 0xFF);
        REQUIRE(reader.readByte() == 0x00);
        REQUIRE_FALSE(reader.hasMore());
        REQUIRE(reader.remaining() == 0);
    }
    
    SECTION("Read boolean") {
        Bytes data = {0x01, 0x00, 0xFF};
        BinaryReader reader(data);
        
        REQUIRE(reader.readBool() == true);
        REQUIRE(reader.readBool() == false);
        REQUIRE(reader.readBool() == true); // Non-zero is true
    }
    
    SECTION("Read bytes as vector") {
        Bytes data = {0x01, 0x02, 0x03, 0x04, 0x05};
        BinaryReader reader(data);
        
        Bytes result = reader.readBytes(3);
        REQUIRE(result.size() == 3);
        REQUIRE(result[0] == 0x01);
        REQUIRE(result[1] == 0x02);
        REQUIRE(result[2] == 0x03);
        REQUIRE(reader.remaining() == 2);
    }
    
    SECTION("Read bytes into buffer") {
        Bytes data = {0x01, 0x02, 0x03, 0x04, 0x05};
        BinaryReader reader(data);
        
        uint8_t buffer[3];
        reader.readBytes(buffer, 3);
        
        REQUIRE(buffer[0] == 0x01);
        REQUIRE(buffer[1] == 0x02);
        REQUIRE(buffer[2] == 0x03);
        REQUIRE(reader.remaining() == 2);
    }
    
    SECTION("Read signed integers") {
        // Create test data for signed integers
        Bytes data = {
            // int8: -128
            0x80,
            // int16: -32768 (little-endian)
            0x00, 0x80,
            // int32: -2147483648 (little-endian)  
            0x00, 0x00, 0x00, 0x80,
            // int64: -9223372036854775808 (little-endian)
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80
        };
        
        BinaryReader reader(data);
        
        REQUIRE(reader.readInt8() == -128);
        REQUIRE(reader.readInt16() == -32768);
        REQUIRE(reader.readInt32() == -2147483648);
        REQUIRE(reader.readInt64() == -9223372036854775808LL);
    }
    
    SECTION("Read unsigned integers") {
        // Create test data for unsigned integers
        Bytes data = {
            // uint8: 255
            0xFF,
            // uint16: 65535 (little-endian)
            0xFF, 0xFF,
            // uint32: 4294967295 (little-endian)
            0xFF, 0xFF, 0xFF, 0xFF,
            // uint64: 18446744073709551615 (little-endian)
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
        };
        
        BinaryReader reader(data);
        
        REQUIRE(reader.readUInt8() == 255);
        REQUIRE(reader.readUInt16() == 65535);
        REQUIRE(reader.readUInt32() == 4294967295U);
        REQUIRE(reader.readUInt64() == 18446744073709551615ULL);
    }
    
    SECTION("Read small values as little-endian integers") {
        // Test specific little-endian encoding
        Bytes data = {
            // uint16: 0x0102 = 258 in little-endian should be 0x0201
            0x01, 0x02,
            // uint32: 0x01020304 in little-endian should be 0x04030201
            0x01, 0x02, 0x03, 0x04
        };
        
        BinaryReader reader(data);
        
        REQUIRE(reader.readUInt16() == 0x0201);
        REQUIRE(reader.readUInt32() == 0x04030201);
    }
    
    SECTION("Read variable length integer") {
        BinaryReader reader1(Bytes{0x64}); // 100 < 0xFD
        REQUIRE(reader1.readVarInt() == 100);
        
        BinaryReader reader2(Bytes{0xFD, 0xE8, 0x03}); // 0xFD + uint16(1000)
        REQUIRE(reader2.readVarInt() == 1000);
        
        BinaryReader reader3(Bytes{0xFE, 0xA0, 0x86, 0x01, 0x00}); // 0xFE + uint32(100000)
        REQUIRE(reader3.readVarInt() == 100000);
        
        BinaryReader reader4(Bytes{0xFF, 0x00, 0xE4, 0x0B, 0x54, 0x02, 0x00, 0x00, 0x00}); // 0xFF + uint64
        REQUIRE(reader4.readVarInt() == 10000000000ULL);
    }
    
    SECTION("Read variable length bytes") {
        // Create data with var length prefix
        Bytes data = {0x05, 0x01, 0x02, 0x03, 0x04, 0x05}; // Length 5, then 5 bytes
        BinaryReader reader(data);
        
        Bytes result = reader.readVarBytes();
        REQUIRE(result.size() == 5);
        REQUIRE(result == Bytes({0x01, 0x02, 0x03, 0x04, 0x05}));
    }
    
    SECTION("Read variable length string") {
        // Create string data with var length prefix
        std::string text = "Hello Neo";
        Bytes data = {0x09}; // Length 9
        data.insert(data.end(), text.begin(), text.end());
        
        BinaryReader reader(data);
        std::string result = reader.readVarString();
        REQUIRE(result == text);
    }
    
    SECTION("Read fixed length string") {
        // String with null terminator
        Bytes data = {'H', 'e', 'l', 'l', 'o', 0x00, 0x00, 0x00};
        BinaryReader reader(data);
        
        std::string result = reader.readFixedString(8);
        REQUIRE(result == "Hello");
    }
    
    SECTION("Read fixed length string without null terminator") {
        Bytes data = {'H', 'e', 'l', 'l', 'o', 'N', 'e', 'o'};
        BinaryReader reader(data);
        
        std::string result = reader.readFixedString(8);
        REQUIRE(result == "HelloNeo");
    }
    
    SECTION("Skip bytes") {
        Bytes data = {0x01, 0x02, 0x03, 0x04, 0x05};
        BinaryReader reader(data);
        
        reader.skip(2);
        REQUIRE(reader.position() == 2);
        REQUIRE(reader.readByte() == 0x03);
    }
    
    SECTION("Seek to position") {
        Bytes data = {0x01, 0x02, 0x03, 0x04, 0x05};
        BinaryReader reader(data);
        
        reader.seek(3);
        REQUIRE(reader.position() == 3);
        REQUIRE(reader.readByte() == 0x04);
        
        reader.seek(1);
        REQUIRE(reader.position() == 1);
        REQUIRE(reader.readByte() == 0x02);
    }
    
    SECTION("Reset position") {
        Bytes data = {0x01, 0x02, 0x03, 0x04, 0x05};
        BinaryReader reader(data);
        
        reader.readBytes(3);
        REQUIRE(reader.position() == 3);
        
        reader.reset();
        REQUIRE(reader.position() == 0);
        REQUIRE(reader.readByte() == 0x01);
    }
    
    SECTION("Exception when reading beyond end") {
        Bytes data = {0x01, 0x02};
        BinaryReader reader(data);
        
        reader.readByte();
        reader.readByte();
        
        REQUIRE_THROWS_AS(reader.readByte(), DeserializationException);
        REQUIRE_THROWS_AS(reader.readBytes(1), DeserializationException);
        REQUIRE_THROWS_AS(reader.readInt32(), DeserializationException);
    }
    
    SECTION("Exception when skipping beyond end") {
        Bytes data = {0x01, 0x02};
        BinaryReader reader(data);
        
        REQUIRE_THROWS_AS(reader.skip(3), DeserializationException);
    }
    
    SECTION("Exception when seeking beyond end") {
        Bytes data = {0x01, 0x02};
        BinaryReader reader(data);
        
        REQUIRE_THROWS_AS(reader.seek(3), DeserializationException);
    }
    
    SECTION("Empty data handling") {
        Bytes data;
        BinaryReader reader(data);
        
        REQUIRE_FALSE(reader.hasMore());
        REQUIRE(reader.remaining() == 0);
        REQUIRE(reader.position() == 0);
        REQUIRE_THROWS_AS(reader.readByte(), DeserializationException);
    }
}