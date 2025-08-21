#include <catch2/catch_test_macros.hpp>
#include "neocpp/script/script_builder.hpp"
#include "neocpp/script/op_code.hpp"
#include "neocpp/utils/hex.hpp"
#include <vector>
#include <cstring>

using namespace neocpp;

TEST_CASE("Script Reading/Parsing Tests", "[script]") {
    
    SECTION("Read simple push operations") {
        // Create a script with simple push operations
        ScriptBuilder builder;
        builder.pushInteger(42);
        builder.pushString("hello");
        builder.pushBool(true);
        
        Bytes script = builder.toArray();
        
        // Parse the script manually
        size_t offset = 0;
        
        // First operation: PUSHINT8 42
        REQUIRE(script[offset] == static_cast<uint8_t>(OpCode::PUSHINT8));
        offset++;
        REQUIRE(script[offset] == 42);
        offset++;
        
        // Second operation: push "hello" (5 bytes)
        REQUIRE(script[offset] == 5); // Length byte
        offset++;
        std::string hello(script.begin() + offset, script.begin() + offset + 5);
        REQUIRE(hello == "hello");
        offset += 5;
        
        // Third operation: PUSH1 (true)
        REQUIRE(script[offset] == static_cast<uint8_t>(OpCode::PUSH1));
    }
    
    SECTION("Read opcodes without operands") {
        ScriptBuilder builder;
        builder.emit(OpCode::NOP);
        builder.emit(OpCode::DUP);
        builder.emit(OpCode::DROP);
        builder.emit(OpCode::SWAP);
        builder.emit(OpCode::ADD);
        
        Bytes script = builder.toArray();
        
        REQUIRE(script.size() == 5);
        REQUIRE(script[0] == static_cast<uint8_t>(OpCode::NOP));
        REQUIRE(script[1] == static_cast<uint8_t>(OpCode::DUP));
        REQUIRE(script[2] == static_cast<uint8_t>(OpCode::DROP));
        REQUIRE(script[3] == static_cast<uint8_t>(OpCode::SWAP));
        REQUIRE(script[4] == static_cast<uint8_t>(OpCode::ADD));
    }
    
    SECTION("Read variable length data") {
        ScriptBuilder builder;
        
        // Small data (< 75 bytes)
        Bytes smallData(50, 0xAA);
        builder.pushData(smallData);
        
        // Medium data (75-255 bytes)
        Bytes mediumData(100, 0xBB);
        builder.pushData(mediumData);
        
        // Large data (256+ bytes)
        Bytes largeData(300, 0xCC);
        builder.pushData(largeData);
        
        Bytes script = builder.toArray();
        size_t offset = 0;
        
        // Read small data
        uint8_t smallLen = script[offset];
        REQUIRE(smallLen == 50);
        offset++;
        for (size_t i = 0; i < 50; i++) {
            REQUIRE(script[offset + i] == 0xAA);
        }
        offset += 50;
        
        // Read medium data (PUSHDATA1)
        REQUIRE(script[offset] == static_cast<uint8_t>(OpCode::PUSHDATA1));
        offset++;
        uint8_t mediumLen = script[offset];
        REQUIRE(mediumLen == 100);
        offset++;
        for (size_t i = 0; i < 100; i++) {
            REQUIRE(script[offset + i] == 0xBB);
        }
        offset += 100;
        
        // Read large data (PUSHDATA2)
        REQUIRE(script[offset] == static_cast<uint8_t>(OpCode::PUSHDATA2));
        offset++;
        uint16_t largeLen = script[offset] | (script[offset + 1] << 8);
        REQUIRE(largeLen == 300);
        offset += 2;
        for (size_t i = 0; i < 300; i++) {
            REQUIRE(script[offset + i] == 0xCC);
        }
    }
    
    SECTION("Read SYSCALL with operand") {
        ScriptBuilder builder;
        builder.emitSysCall("System.Runtime.CheckWitness");
        
        Bytes script = builder.toArray();
        
        REQUIRE(script[0] == static_cast<uint8_t>(OpCode::SYSCALL));
        // SYSCALL is followed by a 4-byte hash of the method name
        REQUIRE(script.size() >= 5);
        
        // The hash should be non-zero
        uint32_t hash = script[1] | (script[2] << 8) | (script[3] << 16) | (script[4] << 24);
        REQUIRE(hash != 0);
    }
    
    SECTION("Read integer push variants") {
        ScriptBuilder builder;
        
        // Test different integer push opcodes
        builder.pushInteger(-1);  // PUSHM1
        builder.pushInteger(0);   // PUSH0
        builder.pushInteger(5);   // PUSH5
        builder.pushInteger(16);  // PUSH16
        builder.pushInteger(17);  // PUSHINT8
        builder.pushInteger(256); // PUSHINT16
        
        Bytes script = builder.toArray();
        size_t offset = 0;
        
        // -1 -> PUSHM1
        REQUIRE(script[offset] == static_cast<uint8_t>(OpCode::PUSHM1));
        offset++;
        
        // 0 -> PUSH0
        REQUIRE(script[offset] == static_cast<uint8_t>(OpCode::PUSH0));
        offset++;
        
        // 5 -> PUSH5
        REQUIRE(script[offset] == static_cast<uint8_t>(OpCode::PUSH5));
        offset++;
        
        // 16 -> PUSH16
        REQUIRE(script[offset] == static_cast<uint8_t>(OpCode::PUSH16));
        offset++;
        
        // 17 -> PUSHINT8
        REQUIRE(script[offset] == static_cast<uint8_t>(OpCode::PUSHINT8));
        offset++;
        REQUIRE(script[offset] == 17);
        offset++;
        
        // 256 -> PUSHINT16
        REQUIRE(script[offset] == static_cast<uint8_t>(OpCode::PUSHINT16));
        offset++;
        uint16_t value = script[offset] | (script[offset + 1] << 8);
        REQUIRE(value == 256);
    }
    
    SECTION("Read complex script") {
        // Create a more complex script simulating a simple calculation
        ScriptBuilder builder;
        
        // Calculate: (5 + 3) * 2
        builder.pushInteger(5);
        builder.pushInteger(3);
        builder.emit(OpCode::ADD);
        builder.pushInteger(2);
        builder.emit(OpCode::MUL);
        
        Bytes script = builder.toArray();
        
        // Parse and verify
        REQUIRE(script[0] == static_cast<uint8_t>(OpCode::PUSH5));
        REQUIRE(script[1] == static_cast<uint8_t>(OpCode::PUSH3));
        REQUIRE(script[2] == static_cast<uint8_t>(OpCode::ADD));
        REQUIRE(script[3] == static_cast<uint8_t>(OpCode::PUSH2));
        REQUIRE(script[4] == static_cast<uint8_t>(OpCode::MUL));
    }
    
    SECTION("Read script with jump operations") {
        ScriptBuilder builder;
        
        // Create a script with conditional jump
        builder.pushBool(true);
        builder.emitJump(OpCode::JMPIF, 5); // Jump forward 5 bytes if true
        builder.emit(OpCode::NOP);
        builder.emit(OpCode::NOP);
        builder.emit(OpCode::NOP);
        builder.pushInteger(42);
        
        Bytes script = builder.toArray();
        size_t offset = 0;
        
        // PUSH1 (true)
        REQUIRE(script[offset] == static_cast<uint8_t>(OpCode::PUSH1));
        offset++;
        
        // JMPIF with offset
        REQUIRE(script[offset] == static_cast<uint8_t>(OpCode::JMPIF));
        offset++;
        int8_t jumpOffset = static_cast<int8_t>(script[offset]);
        REQUIRE(jumpOffset == 5);
    }
    
    SECTION("Read script from hex string") {
        // Create a script and convert to hex
        ScriptBuilder builder;
        builder.pushInteger(123);
        builder.pushString("test");
        builder.emit(OpCode::DUP);
        
        Bytes originalScript = builder.toArray();
        std::string hexScript = Hex::encode(originalScript);
        
        // Parse back from hex
        Bytes parsedScript = Hex::decode(hexScript);
        
        REQUIRE(parsedScript == originalScript);
        
        // Verify content
        size_t offset = 0;
        
        // PUSHINT8 123
        REQUIRE(parsedScript[offset] == static_cast<uint8_t>(OpCode::PUSHINT8));
        offset++;
        REQUIRE(parsedScript[offset] == 123);
        offset++;
        
        // Push "test"
        REQUIRE(parsedScript[offset] == 4); // Length
        offset++;
        std::string test(parsedScript.begin() + offset, parsedScript.begin() + offset + 4);
        REQUIRE(test == "test");
        offset += 4;
        
        // DUP
        REQUIRE(parsedScript[offset] == static_cast<uint8_t>(OpCode::DUP));
    }
    
    SECTION("Identify script type by pattern") {
        // Test identifying common script patterns
        
        // Single signature verification script pattern
        ScriptBuilder sigBuilder;
        Bytes pubKey(33, 0xAB); // Mock public key
        sigBuilder.pushData(pubKey);
        sigBuilder.emitSysCall("System.Crypto.CheckSig");
        
        Bytes sigScript = sigBuilder.toArray();
        
        // Check if it matches single-sig pattern
        bool isSingleSig = (sigScript[0] == 0x21 && // PUSH 33 bytes
                           sigScript[34] == static_cast<uint8_t>(OpCode::SYSCALL));
        REQUIRE(isSingleSig);
        
        // Multi-sig verification script pattern
        ScriptBuilder multiSigBuilder;
        multiSigBuilder.pushInteger(2); // threshold
        for (int i = 0; i < 3; i++) {
            Bytes key(33, static_cast<uint8_t>(0xA0 + i));
            multiSigBuilder.pushData(key);
        }
        multiSigBuilder.pushInteger(3); // key count
        multiSigBuilder.emitSysCall("System.Crypto.CheckMultiSig");
        
        Bytes multiSigScript = multiSigBuilder.toArray();
        
        // Check if it starts with threshold push
        bool isMultiSig = (multiSigScript[0] == static_cast<uint8_t>(OpCode::PUSH2));
        REQUIRE(isMultiSig);
    }
    
    SECTION("Calculate script size") {
        ScriptBuilder builder;
        
        size_t expectedSize = 0;
        
        // Add operations and track expected size
        builder.emit(OpCode::NOP);
        expectedSize += 1;
        
        builder.pushInteger(42);
        expectedSize += 2; // PUSHINT8 + value
        
        builder.pushString("hello");
        expectedSize += 1 + 5; // length byte + string
        
        Bytes data(100, 0xFF);
        builder.pushData(data);
        expectedSize += 1 + 1 + 100; // PUSHDATA1 + length + data
        
        Bytes script = builder.toArray();
        REQUIRE(script.size() == expectedSize);
    }
}