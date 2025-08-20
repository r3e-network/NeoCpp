#include <catch2/catch_test_macros.hpp>
#include "neocpp/serialization/binary_writer.hpp"
#include "neocpp/utils/hex.hpp"

using namespace neocpp;

TEST_CASE("BinaryWriter Tests", "[serialization]") {
    
    SECTION("Write single bytes") {
        BinaryWriter writer;
        
        writer.writeByte(0x00);
        writer.writeByte(0xFF);
        writer.writeByte(0x42);
        
        Bytes result = writer.toArray();
        REQUIRE(result.size() == 3);
        REQUIRE(result[0] == 0x00);
        REQUIRE(result[1] == 0xFF);
        REQUIRE(result[2] == 0x42);
    }
    
    SECTION("Write integers (little-endian)") {
        BinaryWriter writer;
        
        // Write int8
        writer.writeInt8(-128);
        writer.writeUInt8(255);
        
        // Write int16
        writer.writeInt16(-32768);
        writer.writeUInt16(65535);
        
        // Write int32
        writer.writeInt32(-2147483648);
        writer.writeUInt32(4294967295);
        
        // Write int64
        writer.writeInt64(-9223372036854775808LL);
        writer.writeUInt64(18446744073709551615ULL);
        
        Bytes result = writer.toArray();
        REQUIRE(result.size() == 30); // 1+1+2+2+4+4+8+8
    }
    
    SECTION("Write boolean") {
        BinaryWriter writer;
        
        writer.writeBool(true);
        writer.writeBool(false);
        
        Bytes result = writer.toArray();
        REQUIRE(result.size() == 2);
        REQUIRE(result[0] == 0x01);
        REQUIRE(result[1] == 0x00);
    }
    
    SECTION("Write bytes") {
        BinaryWriter writer;
        
        Bytes data = {0x01, 0x02, 0x03, 0x04, 0x05};
        writer.writeBytes(data);
        
        Bytes result = writer.toArray();
        REQUIRE(result == data);
    }
    
    SECTION("Write variable length integer") {
        BinaryWriter writer;
        
        // Value < 0xFD (253)
        writer.writeVarInt(100);
        Bytes result = writer.toArray();
        REQUIRE(result.size() == 1);
        REQUIRE(result[0] == 100);
        
        // Value 0xFD-0xFFFF
        writer.clear();
        writer.writeVarInt(1000);
        result = writer.toArray();
        REQUIRE(result.size() == 3);
        REQUIRE(result[0] == 0xFD);
        
        // Value 0x10000-0xFFFFFFFF
        writer.clear();
        writer.writeVarInt(100000);
        result = writer.toArray();
        REQUIRE(result.size() == 5);
        REQUIRE(result[0] == 0xFE);
        
        // Value > 0xFFFFFFFF
        writer.clear();
        writer.writeVarInt(10000000000ULL);
        result = writer.toArray();
        REQUIRE(result.size() == 9);
        REQUIRE(result[0] == 0xFF);
    }
    
    SECTION("Write variable length bytes") {
        BinaryWriter writer;
        
        Bytes data = {0x01, 0x02, 0x03, 0x04, 0x05};
        writer.writeVarBytes(data);
        
        Bytes result = writer.toArray();
        REQUIRE(result[0] == 5); // Length prefix
        REQUIRE(result.size() == 6); // Length + data
        
        for (size_t i = 0; i < 5; i++) {
            REQUIRE(result[i + 1] == data[i]);
        }
    }
    
    SECTION("Write variable length string") {
        BinaryWriter writer;
        
        std::string text = "Hello Neo";
        writer.writeVarString(text);
        
        Bytes result = writer.toArray();
        REQUIRE(result[0] == 9); // Length of string
        
        std::string extracted(result.begin() + 1, result.end());
        REQUIRE(extracted == text);
    }
    
    SECTION("Write fixed string") {
        BinaryWriter writer;
        
        std::string text = "Neo";
        writer.writeFixedString(text, 10);
        
        Bytes result = writer.toArray();
        REQUIRE(result.size() == 10);
        
        // Should be padded with zeros
        REQUIRE(result[0] == 'N');
        REQUIRE(result[1] == 'e');
        REQUIRE(result[2] == 'o');
        for (size_t i = 3; i < 10; i++) {
            REQUIRE(result[i] == 0);
        }
    }
    
    SECTION("Clear writer") {
        BinaryWriter writer;
        
        writer.writeUInt32(12345);
        REQUIRE(writer.toArray().size() == 4);
        
        writer.clear();
        REQUIRE(writer.toArray().empty());
    }
    
    SECTION("Write multiple data types") {
        BinaryWriter writer;
        
        // Write various data types
        writer.writeUInt8(1);
        writer.writeUInt32(12345);
        writer.writeBytes(Bytes(10, 0));
        
        Bytes result = writer.toArray();
        REQUIRE(result.size() == 15); // 1 + 4 + 10
        REQUIRE(result[0] == 1);
        // Check uint32 in little-endian
        REQUIRE(result[1] == 0x39); // 12345 = 0x3039
        REQUIRE(result[2] == 0x30);
        REQUIRE(result[3] == 0x00);
        REQUIRE(result[4] == 0x00);
    }
}