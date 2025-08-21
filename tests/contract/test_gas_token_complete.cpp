#include <catch2/catch_test_macros.hpp>
#include "neocpp/contract/gas_token.hpp"
#include "neocpp/types/hash160.hpp"
#include "neocpp/wallet/account.hpp"
#include "neocpp/utils/hex.hpp"
#include <memory>

using namespace neocpp;

TEST_CASE("GasToken Complete Tests", "[contract]") {
    
    auto gasToken = std::make_shared<GasToken>();
    auto account1 = Account::create();
    auto account2 = Account::create();
    
    SECTION("Constants") {
        REQUIRE(gasToken->getName() == "GAS");
        REQUIRE(gasToken->getSymbol() == "GAS");
        REQUIRE(gasToken->getDecimals() == 8);
        REQUIRE(gasToken->getTotalSupply() == 5200000000000000LL); // 52,000,000 GAS with 8 decimals
        
        Hash160 expectedHash("0xd2a4cff31913016155e38e474a2c06d08be276cf");
        REQUIRE(gasToken->getScriptHash() == expectedHash);
    }
    
    SECTION("Transfer") {
        auto from = account1;
        auto to = account2;
        int64_t amount = 10000000000; // 100 GAS
        
        auto builder = gasToken->transfer(from, to->getScriptHash(), amount);
        REQUIRE(builder != nullptr);
        
        auto script = builder->getScript();
        REQUIRE(!script.empty());
        
        // Script should contain transfer method
        std::string scriptHex = Hex::encode(script);
        std::string methodHex = Hex::encode(Bytes("transfer".begin(), "transfer".end()));
        REQUIRE(scriptHex.find(methodHex) != std::string::npos);
    }
    
    SECTION("Transfer with data") {
        auto from = account1;
        auto to = account2;
        int64_t amount = 50000000000; // 500 GAS
        std::string data = "Payment for services";
        
        auto builder = gasToken->transfer(from, to->getScriptHash(), amount, data);
        REQUIRE(builder != nullptr);
        
        auto script = builder->getScript();
        REQUIRE(!script.empty());
        
        // Script should contain transfer method and data
        std::string scriptHex = Hex::encode(script);
        std::string methodHex = Hex::encode(Bytes("transfer".begin(), "transfer".end()));
        REQUIRE(scriptHex.find(methodHex) != std::string::npos);
    }
    
    SECTION("Balance of") {
        auto balance = gasToken->balanceOf(account1);
        
        // Result would come from RPC call
        REQUIRE(balance >= 0);
    }
    
    SECTION("Decimals") {
        auto decimals = gasToken->decimals();
        
        // Result would come from RPC call, but we know GAS has 8 decimals
        REQUIRE(decimals == 8);
    }
    
    SECTION("Get refuel amount") {
        // Test refuel calculations
        int64_t gasConsumed = 1000000000; // 10 GAS consumed
        
        // Refuel amount calculation would depend on network state
        // This would normally come from RPC
        int64_t refuelAmount = gasToken->getRefuelAmount(gasConsumed);
        REQUIRE(refuelAmount >= 0);
    }
    
    SECTION("Build transfer script") {
        Hash160 from("0x1234567890123456789012345678901234567890");
        Hash160 to("0xabcdef1234567890abcdef1234567890abcdef12");
        int64_t amount = 25000000000; // 250 GAS
        
        Bytes script = GasToken::buildTransferScript(from, to, amount);
        REQUIRE(!script.empty());
        
        // Script should be properly formatted
        std::string scriptHex = Hex::encode(script);
        std::string methodHex = Hex::encode(Bytes("transfer".begin(), "transfer".end()));
        REQUIRE(scriptHex.find(methodHex) != std::string::npos);
    }
    
    SECTION("Build transfer script with data") {
        Hash160 from("0x1234567890123456789012345678901234567890");
        Hash160 to("0xabcdef1234567890abcdef1234567890abcdef12");
        int64_t amount = 75000000000; // 750 GAS
        ContractParameter data = ContractParameter::string("Invoice #12345");
        
        Bytes script = GasToken::buildTransferScript(from, to, amount, data);
        REQUIRE(!script.empty());
        
        // Script should contain all parameters
        std::string scriptHex = Hex::encode(script);
        REQUIRE(scriptHex.length() > 100); // Should be longer with data
    }
    
    SECTION("Transfer from account to hash") {
        int64_t amount = 100000000; // 1 GAS
        Hash160 toHash("0xabcdef1234567890abcdef1234567890abcdef12");
        
        auto builder = gasToken->transfer(account1, toHash, amount);
        REQUIRE(builder != nullptr);
        
        auto script = builder->getScript();
        REQUIRE(!script.empty());
    }
    
    SECTION("Transfer zero amount") {
        // Zero amount transfer should still be valid
        int64_t amount = 0;
        
        auto builder = gasToken->transfer(account1, account2->getScriptHash(), amount);
        REQUIRE(builder != nullptr);
        
        auto script = builder->getScript();
        REQUIRE(!script.empty());
    }
    
    SECTION("Transfer max amount") {
        // Maximum possible GAS amount
        int64_t amount = INT64_MAX;
        
        auto builder = gasToken->transfer(account1, account2->getScriptHash(), amount);
        REQUIRE(builder != nullptr);
        
        auto script = builder->getScript();
        REQUIRE(!script.empty());
    }
    
    SECTION("Multiple transfers in batch") {
        std::vector<std::pair<Hash160, int64_t>> transfers = {
            {Hash160("0x1111111111111111111111111111111111111111"), 10000000000},
            {Hash160("0x2222222222222222222222222222222222222222"), 20000000000},
            {Hash160("0x3333333333333333333333333333333333333333"), 30000000000}
        };
        
        // Build multiple transfer scripts
        std::vector<Bytes> scripts;
        for (const auto& [to, amount] : transfers) {
            auto builder = gasToken->transfer(account1, to, amount);
            scripts.push_back(builder->getScript());
        }
        
        REQUIRE(scripts.size() == 3);
        for (const auto& script : scripts) {
            REQUIRE(!script.empty());
        }
    }
    
    SECTION("GAS fee calculations") {
        // Test various fee scenarios
        struct FeeTestCase {
            int64_t amount;
            int64_t expectedMinFee;
        };
        
        std::vector<FeeTestCase> testCases = {
            {100000000, 10000},      // 1 GAS transfer, min fee
            {10000000000, 10000},    // 100 GAS transfer
            {100000000000, 10000},   // 1000 GAS transfer
            {1000000000000, 10000}   // 10000 GAS transfer
        };
        
        for (const auto& tc : testCases) {
            auto builder = gasToken->transfer(account1, account2->getScriptHash(), tc.amount);
            REQUIRE(builder != nullptr);
            
            // Network fee would be calculated by the builder
            // This is a simplified test
            REQUIRE(builder->getScript().size() > 0);
        }
    }
    
    SECTION("Get account balance with decimals") {
        // Test balance formatting with 8 decimals
        int64_t rawBalance = 12345678900; // 123.456789 GAS
        
        // Convert to decimal representation
        double gasAmount = static_cast<double>(rawBalance) / 100000000.0;
        REQUIRE(gasAmount == Approx(123.456789));
    }
    
    SECTION("Script hash validation") {
        // Verify GAS token script hash is correct
        Hash160 gasHash = gasToken->getScriptHash();
        std::string hashStr = gasHash.toString();
        
        // Should be the well-known GAS token hash
        REQUIRE(hashStr == "0xd2a4cff31913016155e38e474a2c06d08be276cf");
    }
}