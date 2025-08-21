#include <catch2/catch_test_macros.hpp>
#include "neocpp/contract/nef_file.hpp"
#include "neocpp/serialization/binary_writer.hpp"
#include "neocpp/serialization/binary_reader.hpp"
#include "neocpp/utils/hex.hpp"
#include "neocpp/crypto/base64.hpp"
#include <string>
#include <vector>

using namespace neocpp;

TEST_CASE("NefFile Tests", "[contract]") {
    
    SECTION("Default constructor") {
        NefFile nef;
        
        REQUIRE(nef.getMagic() == "NEF3");
        REQUIRE(nef.getCompiler().empty());
        REQUIRE(nef.getVersion().empty());
        REQUIRE(nef.getScript().empty());
        REQUIRE(nef.getChecksum().empty());
    }
    
    SECTION("Constructor with script") {
        Bytes script = {0x01, 0x02, 0x03, 0x04, 0x05};
        NefFile nef(script);
        
        REQUIRE(nef.getMagic() == "NEF3");
        REQUIRE(nef.getCompiler() == "NeoCpp");
        REQUIRE(nef.getVersion() == "1.0.0");
        REQUIRE(nef.getScript() == script);
        REQUIRE(!nef.getChecksum().empty()); // Should be calculated
    }
    
    SECTION("Constructor with custom compiler and version") {
        Bytes script = {0x10, 0x20, 0x30};
        NefFile nef(script, "CustomCompiler", "2.0.0");
        
        REQUIRE(nef.getCompiler() == "CustomCompiler");
        REQUIRE(nef.getVersion() == "2.0.0");
        REQUIRE(nef.getScript() == script);
    }
    
    SECTION("Set and get compiler") {
        NefFile nef;
        
        nef.setCompiler("TestCompiler");
        REQUIRE(nef.getCompiler() == "TestCompiler");
        
        nef.setCompiler("AnotherCompiler");
        REQUIRE(nef.getCompiler() == "AnotherCompiler");
    }
    
    SECTION("Set and get version") {
        NefFile nef;
        
        nef.setVersion("1.0.0");
        REQUIRE(nef.getVersion() == "1.0.0");
        
        nef.setVersion("2.5.3");
        REQUIRE(nef.getVersion() == "2.5.3");
    }
    
    SECTION("Set script and auto-update checksum") {
        NefFile nef;
        Bytes script = {0xAA, 0xBB, 0xCC, 0xDD};
        
        nef.setScript(script);
        
        REQUIRE(nef.getScript() == script);
        REQUIRE(!nef.getChecksum().empty()); // Checksum should be calculated
    }
    
    SECTION("Update checksum") {
        NefFile nef;
        Bytes script = {0x01, 0x02, 0x03};
        
        nef.setScript(script);
        Bytes checksum1 = nef.getChecksum();
        
        // Change script
        script = {0x04, 0x05, 0x06};
        nef.setScript(script);
        Bytes checksum2 = nef.getChecksum();
        
        // Checksums should be different
        REQUIRE(checksum1 != checksum2);
    }
    
    SECTION("Verify checksum") {
        Bytes script = {0x11, 0x22, 0x33, 0x44};
        NefFile nef(script);
        
        // Checksum should be valid after construction
        REQUIRE(nef.verifyChecksum() == true);
        
        // Manually update checksum
        nef.updateChecksum();
        REQUIRE(nef.verifyChecksum() == true);
    }
    
    SECTION("Serialize to bytes") {
        Bytes script = {0x01, 0x02, 0x03};
        NefFile nef(script, "TestCompiler", "1.0.0");
        
        Bytes serialized = nef.toBytes();
        
        REQUIRE(!serialized.empty());
        
        // Should contain magic header
        std::string magic(serialized.begin(), serialized.begin() + 4);
        REQUIRE(magic == "NEF3");
    }
    
    SECTION("Serialize and deserialize") {
        Bytes script = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};
        NefFile original(script, "MyCompiler", "3.0.0");
        
        // Serialize
        BinaryWriter writer;
        original.serialize(writer);
        Bytes serialized = writer.toArray();
        
        // Deserialize
        BinaryReader reader(serialized);
        NefFile deserialized = NefFile::deserialize(reader);
        
        // Verify all fields match
        REQUIRE(deserialized.getMagic() == original.getMagic());
        REQUIRE(deserialized.getCompiler() == original.getCompiler());
        REQUIRE(deserialized.getVersion() == original.getVersion());
        REQUIRE(deserialized.getScript() == original.getScript());
        REQUIRE(deserialized.getChecksum() == original.getChecksum());
    }
    
    SECTION("Convert to Base64") {
        Bytes script = {0x01, 0x02, 0x03, 0x04};
        NefFile nef(script);
        
        std::string base64 = nef.toBase64();
        
        REQUIRE(!base64.empty());
        // Base64 should be valid (no invalid characters)
        for (char c : base64) {
            bool valid = (c >= 'A' && c <= 'Z') || 
                        (c >= 'a' && c <= 'z') || 
                        (c >= '0' && c <= '9') || 
                        c == '+' || c == '/' || c == '=';
            REQUIRE(valid);
        }
    }
    
    SECTION("Parse from Base64") {
        Bytes script = {0x10, 0x20, 0x30, 0x40, 0x50};
        NefFile original(script, "Base64Compiler", "1.2.3");
        
        // Convert to Base64
        std::string base64 = original.toBase64();
        
        // Parse back
        NefFile parsed = NefFile::fromBase64(base64);
        
        // Verify all fields match
        REQUIRE(parsed.getMagic() == original.getMagic());
        REQUIRE(parsed.getCompiler() == original.getCompiler());
        REQUIRE(parsed.getVersion() == original.getVersion());
        REQUIRE(parsed.getScript() == original.getScript());
        REQUIRE(parsed.getChecksum() == original.getChecksum());
    }
    
    SECTION("Get size") {
        NefFile nef;
        size_t emptySize = nef.getSize();
        REQUIRE(emptySize > 0); // At least magic + empty fields
        
        Bytes script(100, 0xFF);
        nef.setScript(script);
        size_t withScriptSize = nef.getSize();
        
        REQUIRE(withScriptSize > emptySize);
        REQUIRE(withScriptSize >= emptySize + 100); // At least script size more
    }
    
    SECTION("Empty script") {
        NefFile nef;
        
        // Can have empty script
        REQUIRE(nef.getScript().empty());
        
        // Should still be serializable
        Bytes serialized = nef.toBytes();
        REQUIRE(!serialized.empty());
        
        // Should be deserializable
        BinaryReader reader(serialized);
        NefFile deserialized = NefFile::deserialize(reader);
        REQUIRE(deserialized.getScript().empty());
    }
    
    SECTION("Large script") {
        // Test with a larger script
        Bytes largeScript(1000, 0xAB);
        NefFile nef(largeScript);
        
        REQUIRE(nef.getScript().size() == 1000);
        
        // Should be serializable
        Bytes serialized = nef.toBytes();
        REQUIRE(serialized.size() > 1000); // Larger due to metadata
        
        // Round-trip through Base64
        std::string base64 = nef.toBase64();
        NefFile parsed = NefFile::fromBase64(base64);
        REQUIRE(parsed.getScript().size() == 1000);
        REQUIRE(parsed.getScript() == largeScript);
    }
    
    SECTION("Compiler name limits") {
        NefFile nef;
        
        // Test with long compiler name
        std::string longCompiler(255, 'X');
        nef.setCompiler(longCompiler);
        REQUIRE(nef.getCompiler() == longCompiler);
        
        // Should be serializable
        Bytes serialized = nef.toBytes();
        BinaryReader reader(serialized);
        NefFile deserialized = NefFile::deserialize(reader);
        REQUIRE(deserialized.getCompiler() == longCompiler);
    }
    
    SECTION("Checksum consistency") {
        Bytes script = {0x12, 0x34, 0x56, 0x78};
        NefFile nef1(script);
        NefFile nef2(script);
        
        // Same script should produce same checksum
        REQUIRE(nef1.getChecksum() == nef2.getChecksum());
        
        // Different scripts should produce different checksums
        Bytes differentScript = {0x87, 0x65, 0x43, 0x21};
        NefFile nef3(differentScript);
        REQUIRE(nef1.getChecksum() != nef3.getChecksum());
    }
    
    SECTION("Magic header validation") {
        NefFile nef;
        
        // Magic should always be "NEF3"
        REQUIRE(nef.getMagic() == "NEF3");
        REQUIRE(nef.getMagic().length() == 4);
        
        // After serialization
        Bytes serialized = nef.toBytes();
        REQUIRE(serialized.size() >= 4);
        REQUIRE(serialized[0] == 'N');
        REQUIRE(serialized[1] == 'E');
        REQUIRE(serialized[2] == 'F');
        REQUIRE(serialized[3] == '3');
    }
}