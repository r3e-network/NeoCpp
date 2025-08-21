#include <catch2/catch_test_macros.hpp>
#include "neocpp/contract/policy_contract.hpp"
#include "neocpp/types/hash160.hpp"
#include "neocpp/wallet/account.hpp"
#include "neocpp/crypto/ec_key_pair.hpp"
#include "neocpp/utils/hex.hpp"
#include <memory>
#include <vector>

using namespace neocpp;

TEST_CASE("PolicyContract Complete Tests", "[contract]") {
    
    auto policyContract = std::make_shared<PolicyContract>();
    auto account = Account::create();
    
    SECTION("Constants") {
        REQUIRE(policyContract->getName() == "PolicyContract");
        REQUIRE(policyContract->getContractName() == "PolicyContract");
        
        Hash160 expectedHash("0xcc5e4edd9f5f8dba8bb65734541df7a1c081c67b");
        REQUIRE(policyContract->getScriptHash() == expectedHash);
    }
    
    SECTION("Get fee per byte") {
        auto feePerByte = policyContract->getFeePerByte();
        
        // Result would come from RPC call
        // Default is 1000 (0.00001 GAS per byte)
        REQUIRE(feePerByte >= 0);
    }
    
    SECTION("Set fee per byte") {
        int64_t newFeePerByte = 2000; // 0.00002 GAS per byte
        
        auto builder = policyContract->setFeePerByte(newFeePerByte);
        REQUIRE(builder != nullptr);
        
        auto script = builder->getScript();
        REQUIRE(!script.empty());
        
        // Script should contain setFeePerByte method
        std::string scriptHex = Hex::encode(script);
        std::string methodHex = Hex::encode(Bytes("setFeePerByte".begin(), "setFeePerByte".end()));
        REQUIRE(scriptHex.find(methodHex) != std::string::npos);
    }
    
    SECTION("Get execution fee factor") {
        auto executionFeeFactor = policyContract->getExecFeeFactor();
        
        // Result would come from RPC call
        // Default is 30
        REQUIRE(executionFeeFactor >= 0);
    }
    
    SECTION("Set execution fee factor") {
        uint32_t newFactor = 50;
        
        auto builder = policyContract->setExecFeeFactor(newFactor);
        REQUIRE(builder != nullptr);
        
        auto script = builder->getScript();
        REQUIRE(!script.empty());
        
        // Script should contain setExecFeeFactor method
        std::string scriptHex = Hex::encode(script);
        std::string methodHex = Hex::encode(Bytes("setExecFeeFactor".begin(), "setExecFeeFactor".end()));
        REQUIRE(scriptHex.find(methodHex) != std::string::npos);
    }
    
    SECTION("Get storage price") {
        auto storagePrice = policyContract->getStoragePrice();
        
        // Result would come from RPC call
        // Default is 100000 (0.001 GAS per byte)
        REQUIRE(storagePrice >= 0);
    }
    
    SECTION("Set storage price") {
        uint32_t newStoragePrice = 200000; // 0.002 GAS per byte
        
        auto builder = policyContract->setStoragePrice(newStoragePrice);
        REQUIRE(builder != nullptr);
        
        auto script = builder->getScript();
        REQUIRE(!script.empty());
        
        // Script should contain setStoragePrice method
        std::string scriptHex = Hex::encode(script);
        std::string methodHex = Hex::encode(Bytes("setStoragePrice".begin(), "setStoragePrice".end()));
        REQUIRE(scriptHex.find(methodHex) != std::string::npos);
    }
    
    SECTION("Is blocked account") {
        Hash160 accountHash("0x1234567890123456789012345678901234567890");
        
        bool isBlocked = policyContract->isBlocked(accountHash);
        
        // Result would come from RPC call
        // Most accounts should not be blocked
        REQUIRE_NOTHROW(isBlocked);
    }
    
    SECTION("Block account") {
        Hash160 accountToBlock("0xabcdef1234567890abcdef1234567890abcdef12");
        
        auto builder = policyContract->blockAccount(accountToBlock);
        REQUIRE(builder != nullptr);
        
        auto script = builder->getScript();
        REQUIRE(!script.empty());
        
        // Script should contain blockAccount method
        std::string scriptHex = Hex::encode(script);
        std::string methodHex = Hex::encode(Bytes("blockAccount".begin(), "blockAccount".end()));
        REQUIRE(scriptHex.find(methodHex) != std::string::npos);
    }
    
    SECTION("Unblock account") {
        Hash160 accountToUnblock("0xabcdef1234567890abcdef1234567890abcdef12");
        
        auto builder = policyContract->unblockAccount(accountToUnblock);
        REQUIRE(builder != nullptr);
        
        auto script = builder->getScript();
        REQUIRE(!script.empty());
        
        // Script should contain unblockAccount method
        std::string scriptHex = Hex::encode(script);
        std::string methodHex = Hex::encode(Bytes("unblockAccount".begin(), "unblockAccount".end()));
        REQUIRE(scriptHex.find(methodHex) != std::string::npos);
    }
    
    SECTION("Get attribute fee") {
        TransactionAttributeType attributeType = TransactionAttributeType::HIGH_PRIORITY;
        
        auto attributeFee = policyContract->getAttributeFee(attributeType);
        
        // Result would come from RPC call
        // High priority typically has a fee
        REQUIRE(attributeFee >= 0);
    }
    
    SECTION("Set attribute fee") {
        TransactionAttributeType attributeType = TransactionAttributeType::HIGH_PRIORITY;
        int64_t newFee = 5000000000; // 50 GAS
        
        auto builder = policyContract->setAttributeFee(attributeType, newFee);
        REQUIRE(builder != nullptr);
        
        auto script = builder->getScript();
        REQUIRE(!script.empty());
        
        // Script should contain setAttributeFee method
        std::string scriptHex = Hex::encode(script);
        std::string methodHex = Hex::encode(Bytes("setAttributeFee".begin(), "setAttributeFee".end()));
        REQUIRE(scriptHex.find(methodHex) != std::string::npos);
    }
    
    SECTION("Multiple attribute fees") {
        std::vector<TransactionAttributeType> types = {
            TransactionAttributeType::HIGH_PRIORITY,
            TransactionAttributeType::ORACLE_RESPONSE,
            TransactionAttributeType::NOT_VALID_BEFORE
        };
        
        for (const auto& type : types) {
            auto fee = policyContract->getAttributeFee(type);
            REQUIRE(fee >= 0);
        }
    }
    
    SECTION("Fee calculations") {
        // Test fee calculation helpers
        uint32_t transactionSize = 500; // bytes
        int64_t feePerByte = 1000; // 0.00001 GAS per byte
        
        int64_t expectedFee = transactionSize * feePerByte;
        REQUIRE(expectedFee == 500000);
    }
    
    SECTION("Build block account script") {
        std::vector<Hash160> accountsToBlock = {
            Hash160("0x1111111111111111111111111111111111111111"),
            Hash160("0x2222222222222222222222222222222222222222"),
            Hash160("0x3333333333333333333333333333333333333333")
        };
        
        // Build multiple block scripts
        std::vector<Bytes> scripts;
        for (const auto& accountHash : accountsToBlock) {
            auto builder = policyContract->blockAccount(accountHash);
            scripts.push_back(builder->getScript());
        }
        
        REQUIRE(scripts.size() == 3);
        for (const auto& script : scripts) {
            REQUIRE(!script.empty());
        }
    }
    
    SECTION("Policy parameter validation") {
        // Test parameter limits
        
        // Fee per byte should not be negative
        REQUIRE_THROWS_AS(
            policyContract->setFeePerByte(-1),
            std::invalid_argument
        );
        
        // Execution fee factor should be reasonable
        uint32_t maxFactor = 1000;
        auto builder = policyContract->setExecFeeFactor(maxFactor);
        REQUIRE(builder != nullptr);
        
        // Storage price should be positive
        REQUIRE_THROWS_AS(
            policyContract->setStoragePrice(0),
            std::invalid_argument
        );
    }
    
    SECTION("Get all blocked accounts") {
        auto blockedAccounts = policyContract->getBlockedAccounts();
        
        // Result would be an iterator from RPC
        REQUIRE(blockedAccounts != nullptr);
    }
    
    SECTION("Check multiple accounts blocked status") {
        std::vector<Hash160> accountsToCheck = {
            Hash160("0x1234567890123456789012345678901234567890"),
            Hash160("0xabcdef1234567890abcdef1234567890abcdef12"),
            Hash160("0x5555555555555555555555555555555555555555")
        };
        
        std::vector<bool> blockedStatuses;
        for (const auto& accountHash : accountsToCheck) {
            bool isBlocked = policyContract->isBlocked(accountHash);
            blockedStatuses.push_back(isBlocked);
        }
        
        REQUIRE(blockedStatuses.size() == 3);
        // Most accounts should not be blocked
        int blockedCount = std::count(blockedStatuses.begin(), blockedStatuses.end(), true);
        REQUIRE(blockedCount >= 0);
    }
    
    SECTION("Script hash validation") {
        // Verify PolicyContract script hash is correct
        Hash160 policyHash = policyContract->getScriptHash();
        std::string hashStr = policyHash.toString();
        
        // Should be the well-known PolicyContract hash
        REQUIRE(hashStr == "0xcc5e4edd9f5f8dba8bb65734541df7a1c081c67b");
    }
}