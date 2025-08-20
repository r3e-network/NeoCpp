#include <catch2/catch_test_macros.hpp>
#include "neocpp/script/script_builder.hpp"
#include "neocpp/script/op_code.hpp"
#include "neocpp/utils/hex.hpp"
#include "neocpp/types/hash160.hpp"
#include "neocpp/types/hash256.hpp"
#include "neocpp/crypto/ec_key_pair.hpp"

using namespace neocpp;

TEST_CASE("ScriptBuilder Tests", "[script]") {
    
    SECTION("Push integer values") {
        ScriptBuilder builder;
        
        // Push -1
        builder.pushInteger(-1);
        REQUIRE(builder.toArray() == Bytes{static_cast<uint8_t>(OpCode::PUSHM1)});
        
        // Push 0
        builder.clear();
        builder.pushInteger(0);
        REQUIRE(builder.toArray() == Bytes{static_cast<uint8_t>(OpCode::PUSH0)});
        
        // Push 1-16
        for (int i = 1; i <= 16; i++) {
            builder.clear();
            builder.pushInteger(i);
            Bytes expected = {static_cast<uint8_t>(static_cast<uint8_t>(OpCode::PUSH1) + i - 1)};
            REQUIRE(builder.toArray() == expected);
        }
        
        // Push larger values
        builder.clear();
        builder.pushInteger(127);
        Bytes expected = {static_cast<uint8_t>(OpCode::PUSHINT8), 0x7F};
        REQUIRE(builder.toArray() == expected);
        
        builder.clear();
        builder.pushInteger(32767);
        expected = {static_cast<uint8_t>(OpCode::PUSHINT16), 0xFF, 0x7F};
        REQUIRE(builder.toArray() == expected);
    }
    
    SECTION("Push boolean values") {
        ScriptBuilder builder;
        
        builder.pushBool(true);
        REQUIRE(builder.toArray() == Bytes{static_cast<uint8_t>(OpCode::PUSH1)});
        
        builder.clear();
        builder.pushBool(false);
        REQUIRE(builder.toArray() == Bytes{static_cast<uint8_t>(OpCode::PUSH0)});
    }
    
    SECTION("Push data") {
        ScriptBuilder builder;
        
        // Small data (< 75 bytes)
        Bytes smallData = {0x01, 0x02, 0x03};
        builder.pushData(smallData);
        Bytes expected = {0x03, 0x01, 0x02, 0x03}; // length + data
        REQUIRE(builder.toArray() == expected);
        
        // Medium data (75-255 bytes)
        builder.clear();
        Bytes mediumData(100, 0xAB);
        builder.pushData(mediumData);
        REQUIRE(builder.toArray()[0] == static_cast<uint8_t>(OpCode::PUSHDATA1));
        REQUIRE(builder.toArray()[1] == 100);
        
        // Large data (256-65535 bytes)
        builder.clear();
        Bytes largeData(1000, 0xCD);
        builder.pushData(largeData);
        REQUIRE(builder.toArray()[0] == static_cast<uint8_t>(OpCode::PUSHDATA2));
    }
    
    SECTION("Push string") {
        ScriptBuilder builder;
        
        std::string text = "Hello Neo";
        builder.pushString(text);
        
        Bytes script = builder.toArray();
        REQUIRE(script[0] == text.length());
        
        // Check the string content
        std::string extracted(script.begin() + 1, script.begin() + 1 + text.length());
        REQUIRE(extracted == text);
    }
    
    SECTION("Push Hash160") {
        ScriptBuilder builder;
        
        Hash160 hash("23ba2703c53263e8d6e522dc32203339dcd8eee9");
        // Push hash as data
        builder.pushData(hash.toLittleEndianArray());
        
        Bytes script = builder.toArray();
        REQUIRE(script[0] == 0x14); // 20 bytes
        
        // Hash should be in little-endian
        Bytes hashBytes = hash.toLittleEndianArray();
        for (size_t i = 0; i < 20; i++) {
            REQUIRE(script[i + 1] == hashBytes[i]);
        }
    }
    
    SECTION("Push Hash256") {
        ScriptBuilder builder;
        
        Hash256 hash("e8c2a6a6453097f1acf66e0d40f06a856a99f9b9e58e970f1377add726d0a632");
        // Push hash as data
        builder.pushData(hash.toLittleEndianArray());
        
        Bytes script = builder.toArray();
        REQUIRE(script[0] == 0x20); // 32 bytes
        
        // Hash should be in little-endian
        Bytes hashBytes = hash.toLittleEndianArray();
        for (size_t i = 0; i < 32; i++) {
            REQUIRE(script[i + 1] == hashBytes[i]);
        }
    }
    
    SECTION("Push public key") {
        ScriptBuilder builder;
        
        ECKeyPair keyPair = ECKeyPair::generate();
        builder.pushPublicKey(keyPair.getPublicKey());
        
        Bytes script = builder.toArray();
        REQUIRE(script[0] == 0x21); // 33 bytes for compressed public key
        
        Bytes pubKeyBytes = keyPair.getPublicKey()->getEncoded();
        for (size_t i = 0; i < 33; i++) {
            REQUIRE(script[i + 1] == pubKeyBytes[i]);
        }
    }
    
    SECTION("Emit opcodes") {
        ScriptBuilder builder;
        
        builder.emit(OpCode::NOP);
        builder.emit(OpCode::DUP);
        builder.emit(OpCode::DROP);
        
        Bytes expected = {
            static_cast<uint8_t>(OpCode::NOP),
            static_cast<uint8_t>(OpCode::DUP),
            static_cast<uint8_t>(OpCode::DROP)
        };
        REQUIRE(builder.toArray() == expected);
    }
    
    SECTION("Emit syscall") {
        ScriptBuilder builder;
        
        std::string syscall = "System.Runtime.CheckWitness";
        builder.emitSysCall(syscall);
        
        Bytes script = builder.toArray();
        REQUIRE(script[0] == static_cast<uint8_t>(OpCode::SYSCALL));
        
        // The syscall should be followed by a 4-byte hash
        REQUIRE(script.size() >= 5);
    }
    
    SECTION("Build verification script") {
        ECKeyPair keyPair = ECKeyPair::generate();
        
        Bytes verificationScript = ScriptBuilder::buildVerificationScript(keyPair.getPublicKey());
        
        // Should contain: PUSH21 + public key (33 bytes) + SYSCALL + hash
        REQUIRE(verificationScript.size() > 34);
        REQUIRE(verificationScript[0] == 0x21); // PUSH21
        
        // Check public key
        Bytes pubKey = keyPair.getPublicKey()->getEncoded();
        for (size_t i = 0; i < 33; i++) {
            REQUIRE(verificationScript[i + 1] == pubKey[i]);
        }
    }
    
    SECTION("Build multi-sig verification script") {
        std::vector<SharedPtr<ECPublicKey>> publicKeys;
        
        // Create 3 public keys
        for (int i = 0; i < 3; i++) {
            ECKeyPair keyPair = ECKeyPair::generate();
            publicKeys.push_back(keyPair.getPublicKey());
        }
        
        // Build 2-of-3 multi-sig script using ScriptBuilder
        ScriptBuilder builder;
        builder.pushInteger(2); // threshold
        for (const auto& pubKey : publicKeys) {
            builder.pushPublicKey(pubKey);
        }
        builder.pushInteger(3); // key count
        builder.emitSysCall("System.Crypto.CheckMultiSig");
        
        Bytes multiSigScript = builder.toArray();
        
        REQUIRE(!multiSigScript.empty());
        
        // Should start with PUSH2 (threshold)
        REQUIRE(multiSigScript[0] == static_cast<uint8_t>(OpCode::PUSH2));
    }
    
    SECTION("Clear builder") {
        ScriptBuilder builder;
        
        builder.pushInteger(42);
        builder.emit(OpCode::NOP);
        REQUIRE(!builder.toArray().empty());
        
        builder.clear();
        REQUIRE(builder.toArray().empty());
    }
    
    SECTION("Get script size") {
        ScriptBuilder builder;
        
        REQUIRE(builder.size() == 0);
        
        builder.pushInteger(42);
        REQUIRE(builder.size() > 0);
        
        size_t size1 = builder.size();
        builder.emit(OpCode::NOP);
        REQUIRE(builder.size() == size1 + 1);
    }
}