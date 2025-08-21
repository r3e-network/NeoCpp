#include <catch2/catch_test_macros.hpp>
#include "neocpp/contract/smart_contract.hpp"
#include "neocpp/types/hash160.hpp"
#include "neocpp/types/contract_parameter.hpp"
#include <nlohmann/json.hpp>
#include <memory>
#include <vector>

using namespace neocpp;
using json = nlohmann::json;

TEST_CASE("SmartContract Tests", "[contract]") {
    
    SECTION("Constructor with script hash") {
        Hash160 scriptHash("0x0102030405060708090a0b0c0d0e0f1011121314");
        SharedPtr<NeoRpcClient> client = nullptr; // Mock client
        
        SmartContract contract(scriptHash, client);
        
        REQUIRE(contract.getScriptHash() == scriptHash);
        REQUIRE(contract.getClient() == client);
    }
    
    SECTION("Set and get RPC client") {
        Hash160 scriptHash("0x1234567890abcdef1234567890abcdef12345678");
        SharedPtr<NeoRpcClient> client1 = nullptr;
        SharedPtr<NeoRpcClient> client2 = nullptr;
        
        SmartContract contract(scriptHash, client1);
        REQUIRE(contract.getClient() == client1);
        
        contract.setClient(client2);
        REQUIRE(contract.getClient() == client2);
    }
    
    SECTION("Get script hash") {
        Hash160 hash1("0xabcdef1234567890abcdef1234567890abcdef12");
        Hash160 hash2("0x1234567890abcdef1234567890abcdef12345678");
        
        SmartContract contract1(hash1, nullptr);
        SmartContract contract2(hash2, nullptr);
        
        REQUIRE(contract1.getScriptHash() == hash1);
        REQUIRE(contract2.getScriptHash() == hash2);
        REQUIRE(contract1.getScriptHash() != hash2);
        REQUIRE(contract2.getScriptHash() != hash1);
    }
    
    SECTION("Script hash immutability") {
        Hash160 originalHash("0xfedcba0987654321fedcba0987654321fedcba09");
        SmartContract contract(originalHash, nullptr);
        
        // Script hash should remain the same
        REQUIRE(contract.getScriptHash() == originalHash);
        
        // Changing client shouldn't affect script hash
        contract.setClient(nullptr);
        REQUIRE(contract.getScriptHash() == originalHash);
    }
    
    SECTION("Multiple contracts with different hashes") {
        std::vector<SharedPtr<SmartContract>> contracts;
        
        for (int i = 0; i < 5; i++) {
            std::string hashStr = "0x";
            for (int j = 0; j < 20; j++) {
                hashStr += std::to_string(i) + std::to_string(j % 10);
            }
            Hash160 hash(hashStr.substr(0, 42)); // Take first 42 chars (0x + 40 hex)
            
            auto contract = std::make_shared<SmartContract>(hash, nullptr);
            contracts.push_back(contract);
        }
        
        // Each contract should have unique hash
        for (size_t i = 0; i < contracts.size(); i++) {
            for (size_t j = i + 1; j < contracts.size(); j++) {
                REQUIRE(contracts[i]->getScriptHash() != contracts[j]->getScriptHash());
            }
        }
    }
    
    // Note: Most SmartContract methods require RPC client interaction
    // which would need mocking or integration testing.
    // These tests cover the basic contract construction and property management.
}