#include <catch2/catch_test_macros.hpp>
#include "neocpp/serialization/binary_writer.hpp"
#include "neocpp/serialization/binary_reader.hpp"

using namespace neocpp;

namespace {
    // Helper function to calculate the size needed for a VarInt
    size_t getVarIntSize(uint64_t value) {
        if (value < 0xFD) {
            return 1;
        } else if (value <= 0xFFFF) {
            return 3; // 0xFD + 2 bytes
        } else if (value <= 0xFFFFFFFF) {
            return 5; // 0xFE + 4 bytes  
        } else {
            return 9; // 0xFF + 8 bytes
        }
    }
    
    // Helper function to calculate size needed for VarBytes
    size_t getVarBytesSize(const Bytes& bytes) {
        return getVarIntSize(bytes.size()) + bytes.size();
    }
    
    // Helper function to calculate size needed for VarString
    size_t getVarStringSize(const std::string& str) {
        return getVarIntSize(str.size()) + str.size();
    }
}

TEST_CASE("Variable Size Encoding Tests", "[serialization]") {
    
    SECTION("VarInt size calculation") {
        // Test size calculation for various VarInt values
        REQUIRE(getVarIntSize(0) == 1);
        REQUIRE(getVarIntSize(100) == 1);
        REQUIRE(getVarIntSize(252) == 1);
        REQUIRE(getVarIntSize(253) == 3); // 0xFD
        REQUIRE(getVarIntSize(1000) == 3);
        REQUIRE(getVarIntSize(65535) == 3);
        REQUIRE(getVarIntSize(65536) == 5); // 0xFE
        REQUIRE(getVarIntSize(100000) == 5);
        REQUIRE(getVarIntSize(4294967295U) == 5);
        REQUIRE(getVarIntSize(4294967296ULL) == 9); // 0xFF
        REQUIRE(getVarIntSize(10000000000ULL) == 9);
    }
    
    SECTION("VarInt encoding and size validation") {
        std::vector<uint64_t> testValues = {
            0, 1, 100, 252, 253, 1000, 65535, 65536, 100000,
            4294967295U, 4294967296ULL, 10000000000ULL
        };
        
        for (uint64_t value : testValues) {
            BinaryWriter writer;
            writer.writeVarInt(value);
            
            Bytes encoded = writer.toArray();
            size_t expectedSize = getVarIntSize(value);
            
            REQUIRE(encoded.size() == expectedSize);
            
            // Verify we can read it back
            BinaryReader reader(encoded);
            REQUIRE(reader.readVarInt() == value);
        }
    }
    
    SECTION("VarBytes size calculation") {
        Bytes empty;
        Bytes small = {0x01, 0x02, 0x03};
        Bytes medium(1000, 0xAB);
        Bytes large(100000, 0xCD);
        
        REQUIRE(getVarBytesSize(empty) == 1); // Just the size prefix
        REQUIRE(getVarBytesSize(small) == 4); // 1 + 3
        REQUIRE(getVarBytesSize(medium) == 1003); // 3 + 1000
        REQUIRE(getVarBytesSize(large) == 100005); // 5 + 100000
    }
    
    SECTION("VarBytes encoding and size validation") {
        std::vector<Bytes> testData = {
            {},
            {0x01},
            {0x01, 0x02, 0x03, 0x04, 0x05},
            Bytes(252, 0xAA),
            Bytes(253, 0xBB),
            Bytes(1000, 0xCC),
            Bytes(65536, 0xDD)
        };
        
        for (const auto& data : testData) {
            BinaryWriter writer;
            writer.writeVarBytes(data);
            
            Bytes encoded = writer.toArray();
            size_t expectedSize = getVarBytesSize(data);
            
            REQUIRE(encoded.size() == expectedSize);
            
            // Verify we can read it back
            BinaryReader reader(encoded);
            Bytes decoded = reader.readVarBytes();
            REQUIRE(decoded == data);
        }
    }
    
    SECTION("VarString size calculation") {
        std::string empty = "";
        std::string short_str = "Neo";
        std::string medium_str(1000, 'A');
        std::string long_str(100000, 'B');
        
        REQUIRE(getVarStringSize(empty) == 1);
        REQUIRE(getVarStringSize(short_str) == 4); // 1 + 3
        REQUIRE(getVarStringSize(medium_str) == 1003); // 3 + 1000  
        REQUIRE(getVarStringSize(long_str) == 100005); // 5 + 100000
    }
    
    SECTION("VarString encoding and size validation") {
        std::vector<std::string> testStrings = {
            "",
            "A",
            "Hello Neo",
            std::string(252, 'X'),
            std::string(253, 'Y'),
            std::string(1000, 'Z'),
            std::string(10000, 'W')
        };
        
        for (const auto& str : testStrings) {
            BinaryWriter writer;
            writer.writeVarString(str);
            
            Bytes encoded = writer.toArray();
            size_t expectedSize = getVarStringSize(str);
            
            REQUIRE(encoded.size() == expectedSize);
            
            // Verify we can read it back
            BinaryReader reader(encoded);
            std::string decoded = reader.readVarString();
            REQUIRE(decoded == str);
        }
    }
    
    SECTION("VarInt boundary values") {
        // Test exact boundary values
        struct TestCase {
            uint64_t value;
            size_t expectedSize;
            uint8_t firstByte;
        };
        
        std::vector<TestCase> boundaries = {
            {252, 1, 252},      // Last single byte value
            {253, 3, 0xFD},     // First 3-byte value
            {65535, 3, 0xFD},   // Last 3-byte value
            {65536, 5, 0xFE},   // First 5-byte value
            {4294967295U, 5, 0xFE}, // Last 5-byte value
            {4294967296ULL, 9, 0xFF} // First 9-byte value
        };
        
        for (const auto& test : boundaries) {
            BinaryWriter writer;
            writer.writeVarInt(test.value);
            
            Bytes encoded = writer.toArray();
            REQUIRE(encoded.size() == test.expectedSize);
            REQUIRE(encoded[0] == test.firstByte);
            
            // Verify decoding
            BinaryReader reader(encoded);
            REQUIRE(reader.readVarInt() == test.value);
        }
    }
    
    SECTION("Large data size calculations") {
        // Test with very large hypothetical data sizes
        uint64_t maxUint32 = 4294967295U;
        uint64_t maxUint64 = 18446744073709551615ULL;
        
        REQUIRE(getVarIntSize(maxUint32) == 5);
        REQUIRE(getVarIntSize(maxUint64) == 9);
        
        // Calculate sizes for very large arrays
        size_t largeArrayVarIntSize = getVarIntSize(maxUint32);
        REQUIRE(largeArrayVarIntSize == 5);
        
        size_t hugeArrayVarIntSize = getVarIntSize(maxUint64);  
        REQUIRE(hugeArrayVarIntSize == 9);
    }
    
    SECTION("Mixed variable size data") {
        BinaryWriter writer;
        
        // Write various variable-size items
        writer.writeVarInt(100);              // 1 byte
        writer.writeVarString("Hello");       // 1 + 5 = 6 bytes
        writer.writeVarBytes({0x01, 0x02});   // 1 + 2 = 3 bytes
        writer.writeVarInt(1000);             // 3 bytes
        writer.writeVarString("Neo");         // 1 + 3 = 4 bytes
        
        Bytes encoded = writer.toArray();
        size_t expectedTotal = 1 + 6 + 3 + 3 + 4; // 17 bytes
        REQUIRE(encoded.size() == expectedTotal);
        
        // Verify we can read everything back
        BinaryReader reader(encoded);
        REQUIRE(reader.readVarInt() == 100);
        REQUIRE(reader.readVarString() == "Hello");
        REQUIRE(reader.readVarBytes() == Bytes({0x01, 0x02}));
        REQUIRE(reader.readVarInt() == 1000);
        REQUIRE(reader.readVarString() == "Neo");
        REQUIRE_FALSE(reader.hasMore());
    }
}