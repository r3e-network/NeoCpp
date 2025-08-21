#include <catch2/catch_test_macros.hpp>
#include "neocpp/script/script_builder.hpp"
#include "neocpp/script/op_code.hpp"
#include "neocpp/script/interop_service.hpp"
#include "neocpp/utils/hex.hpp"
#include "neocpp/types/contract_parameter.hpp"
#include "neocpp/crypto/ec_key_pair.hpp"
#include <map>

using namespace neocpp;

TEST_CASE("ScriptBuilder Complete Tests", "[script]") {
    
    SECTION("Push array empty") {
        ScriptBuilder builder;
        std::vector<ContractParameter> emptyArray;
        builder.pushArray(emptyArray);
        REQUIRE(builder.toArray() == Bytes{static_cast<uint8_t>(OpCode::NEWARRAY0)});
    }
    
    SECTION("Push param empty array") {
        ScriptBuilder builder;
        ContractParameter param = ContractParameter::array({});
        builder.pushParam(param);
        REQUIRE(builder.toArray() == Bytes{static_cast<uint8_t>(OpCode::NEWARRAY0)});
    }
    
    SECTION("Push byte array") {
        ScriptBuilder builder;
        
        // Test 1 byte
        Bytes data1(1, 0x01);
        builder.pushData(data1);
        REQUIRE(builder.toArray()[0] == 0x0c);
        REQUIRE(builder.toArray()[1] == 0x01);
        
        builder.clear();
        // Test 75 bytes
        Bytes data75(75, 0x01);
        builder.pushData(data75);
        REQUIRE(builder.toArray()[0] == 0x0c);
        REQUIRE(builder.toArray()[1] == 0x4b);
        
        builder.clear();
        // Test 256 bytes
        Bytes data256(256, 0x01);
        builder.pushData(data256);
        REQUIRE(builder.toArray()[0] == 0x0d);
        REQUIRE(builder.toArray()[1] == 0x00);
        REQUIRE(builder.toArray()[2] == 0x01);
        
        builder.clear();
        // Test 65536 bytes
        Bytes data65536(65536, 0x01);
        builder.pushData(data65536);
        REQUIRE(builder.toArray()[0] == 0x0e);
        REQUIRE(builder.toArray()[1] == 0x00);
        REQUIRE(builder.toArray()[2] == 0x00);
        REQUIRE(builder.toArray()[3] == 0x01);
        REQUIRE(builder.toArray()[4] == 0x00);
    }
    
    SECTION("Push string") {
        ScriptBuilder builder;
        
        // Empty string
        builder.pushString("");
        REQUIRE(builder.toArray() == Bytes{0x0c, 0x00});
        
        builder.clear();
        // Single char
        builder.pushString("a");
        REQUIRE(builder.toArray() == Bytes{0x0c, 0x01, 0x61});
        
        builder.clear();
        // Long string (10000 chars)
        std::string longString(10000, 'a');
        builder.pushString(longString);
        REQUIRE(builder.toArray()[0] == 0x0d);
        REQUIRE(builder.toArray()[1] == 0x10);
        REQUIRE(builder.toArray()[2] == 0x27);
    }
    
    SECTION("Push integer") {
        ScriptBuilder builder;
        
        // Push 0
        builder.pushInteger(0);
        REQUIRE(builder.toArray() == Bytes{static_cast<uint8_t>(OpCode::PUSH0)});
        
        builder.clear();
        // Push 1
        builder.pushInteger(1);
        REQUIRE(builder.toArray() == Bytes{static_cast<uint8_t>(OpCode::PUSH1)});
        
        builder.clear();
        // Push 16
        builder.pushInteger(16);
        REQUIRE(builder.toArray() == Bytes{static_cast<uint8_t>(OpCode::PUSH16)});
        
        builder.clear();
        // Push 17
        builder.pushInteger(17);
        REQUIRE(builder.toArray() == Bytes{0x00, 0x11});
        
        builder.clear();
        // Push -800000
        builder.pushInteger(-800000);
        Bytes expected = builder.toArray();
        REQUIRE(expected.size() == 5);
        
        builder.clear();
        // Push large positive
        builder.pushInteger(100000000000LL);
        expected = builder.toArray();
        REQUIRE(expected.size() == 9);
        
        builder.clear();
        // Push large negative  
        builder.pushInteger(-100000000000LL);
        expected = builder.toArray();
        REQUIRE(expected.size() == 9);
    }
    
    SECTION("Verification script from public keys") {
        std::string key1 = "035fdb1d1f06759547020891ae97c729327853aeb1256b6fe0473bc2e9fa42ff50";
        std::string key2 = "03eda286d19f7ee0b472afd1163d803d620a961e1581a8f2704b52c0285f6e022d";
        std::string key3 = "03ac81ec17f2f15fd6d193182f927c5971559c2a32b9408a06fec9e711fb7ca02e";
        
        std::vector<SharedPtr<ECPublicKey>> keys;
        keys.push_back(std::make_shared<ECPublicKey>(Hex::decode(key1)));
        keys.push_back(std::make_shared<ECPublicKey>(Hex::decode(key2)));
        keys.push_back(std::make_shared<ECPublicKey>(Hex::decode(key3)));
        
        Bytes script = ScriptBuilder::buildVerificationScript(keys, 2);
        
        // Verify script structure
        size_t offset = 0;
        REQUIRE(script[offset++] == static_cast<uint8_t>(OpCode::PUSH2));
        REQUIRE(script[offset++] == static_cast<uint8_t>(OpCode::PUSHDATA1));
        REQUIRE(script[offset++] == 0x21);
        // Skip first key (33 bytes)
        offset += 33;
        REQUIRE(script[offset++] == static_cast<uint8_t>(OpCode::PUSHDATA1));
        REQUIRE(script[offset++] == 0x21);
        // Skip second key (33 bytes)
        offset += 33;
        REQUIRE(script[offset++] == static_cast<uint8_t>(OpCode::PUSHDATA1));
        REQUIRE(script[offset++] == 0x21);
        // Skip third key (33 bytes)
        offset += 33;
        REQUIRE(script[offset++] == static_cast<uint8_t>(OpCode::PUSH3));
        REQUIRE(script[offset++] == static_cast<uint8_t>(OpCode::SYSCALL));
    }
    
    SECTION("Verification script from public key") {
        std::string key = "035fdb1d1f06759547020891ae97c729327853aeb1256b6fe0473bc2e9fa42ff50";
        Bytes keyBytes = Hex::decode(key);
        Bytes script = ScriptBuilder::buildVerificationScript(keyBytes);
        
        // Verify script structure
        size_t offset = 0;
        REQUIRE(script[offset++] == static_cast<uint8_t>(OpCode::PUSHDATA1));
        REQUIRE(script[offset++] == 0x21);
        // Skip key (33 bytes)
        for (size_t i = 0; i < 33; i++) {
            REQUIRE(script[offset++] == keyBytes[i]);
        }
        REQUIRE(script[offset++] == static_cast<uint8_t>(OpCode::SYSCALL));
    }
    
    SECTION("Map") {
        ScriptBuilder builder;
        std::map<ContractParameter, ContractParameter> map;
        map[ContractParameter::integer(1)] = ContractParameter::string("first");
        map[ContractParameter::byteArray(Hex::decode("7365636f6e64"))] = ContractParameter::boolean(true);
        
        builder.pushMap(map);
        Bytes script = builder.toArray();
        
        // Map should contain the values and end with PACKMAP
        REQUIRE(script.back() == static_cast<uint8_t>(OpCode::PACKMAP));
        // Should have pushed 2 for map size
        REQUIRE(std::find(script.begin(), script.end(), static_cast<uint8_t>(OpCode::PUSH2)) != script.end());
    }
    
    SECTION("Map nested") {
        ScriptBuilder builder;
        
        std::map<ContractParameter, ContractParameter> nestedMap;
        nestedMap[ContractParameter::integer(10)] = ContractParameter::string("nestedFirst");
        
        std::map<ContractParameter, ContractParameter> map;
        map[ContractParameter::integer(1)] = ContractParameter::string("first");
        map[ContractParameter::byteArray(Hex::decode("6e6573746564"))] = ContractParameter::map(nestedMap);
        
        builder.pushMap(map);
        Bytes script = builder.toArray();
        
        // Should have multiple PACKMAP opcodes for nested structure
        int packMapCount = std::count(script.begin(), script.end(), static_cast<uint8_t>(OpCode::PACKMAP));
        REQUIRE(packMapCount == 2);
    }
}
