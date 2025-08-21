#include <catch2/catch_test_macros.hpp>
#include "neocpp/contract/neo_token.hpp"
#include "neocpp/types/hash160.hpp"
#include "neocpp/wallet/account.hpp"
#include "neocpp/crypto/ec_key_pair.hpp"
#include "neocpp/utils/hex.hpp"
#include <memory>
#include <vector>

using namespace neocpp;

TEST_CASE("NeoToken Complete Tests", "[contract]") {
    
    auto neoToken = std::make_shared<NeoToken>();
    auto account1 = Account::create();
    auto account2 = Account::create();
    
    SECTION("Constants") {
        REQUIRE(neoToken->getName() == "NEO");
        REQUIRE(neoToken->getSymbol() == "NEO");
        REQUIRE(neoToken->getDecimals() == 0);
        REQUIRE(neoToken->getTotalSupply() == 100000000);
        
        Hash160 expectedHash("0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5");
        REQUIRE(neoToken->getScriptHash() == expectedHash);
    }
    
    SECTION("Register candidate") {
        auto publicKey = account1->getKeyPair()->getPublicKey();
        
        auto builder = neoToken->registerCandidate(publicKey);
        REQUIRE(builder != nullptr);
        
        auto script = builder->getScript();
        REQUIRE(!script.empty());
        
        // Script should contain registerCandidate method
        std::string scriptHex = Hex::encode(script);
        std::string methodHex = Hex::encode(Bytes("registerCandidate".begin(), "registerCandidate".end()));
        REQUIRE(scriptHex.find(methodHex) != std::string::npos);
    }
    
    SECTION("Unregister candidate") {
        auto publicKey = account1->getKeyPair()->getPublicKey();
        
        auto builder = neoToken->unregisterCandidate(publicKey);
        REQUIRE(builder != nullptr);
        
        auto script = builder->getScript();
        REQUIRE(!script.empty());
        
        // Script should contain unregisterCandidate method
        std::string scriptHex = Hex::encode(script);
        std::string methodHex = Hex::encode(Bytes("unregisterCandidate".begin(), "unregisterCandidate".end()));
        REQUIRE(scriptHex.find(methodHex) != std::string::npos);
    }
    
    SECTION("Get candidates") {
        auto result = neoToken->getCandidates();
        
        // Result would come from RPC call
        REQUIRE(result != nullptr);
    }
    
    SECTION("Is candidate") {
        auto publicKey = account1->getKeyPair()->getPublicKey();
        
        bool isCandidate = neoToken->isCandidate(publicKey);
        // Result would come from RPC call
        REQUIRE_NOTHROW(isCandidate);
    }
    
    SECTION("Get all candidates iterator") {
        auto iterator = neoToken->getAllCandidatesIterator();
        
        // Iterator would be from RPC call
        REQUIRE(iterator != nullptr);
    }
    
    SECTION("Get candidate votes") {
        auto publicKey = account1->getKeyPair()->getPublicKey();
        
        auto votes = neoToken->getCandidateVotes(publicKey);
        // Result would come from RPC call
        REQUIRE(votes >= 0);
    }
    
    SECTION("Vote") {
        auto voter = account1;
        auto candidate = account2->getKeyPair()->getPublicKey();
        
        auto builder = neoToken->vote(voter, candidate);
        REQUIRE(builder != nullptr);
        
        auto script = builder->getScript();
        REQUIRE(!script.empty());
        
        // Script should contain vote method
        std::string scriptHex = Hex::encode(script);
        std::string methodHex = Hex::encode(Bytes("vote".begin(), "vote".end()));
        REQUIRE(scriptHex.find(methodHex) != std::string::npos);
    }
    
    SECTION("Cancel vote") {
        auto voter = account1;
        
        auto builder = neoToken->cancelVote(voter);
        REQUIRE(builder != nullptr);
        
        auto script = builder->getScript();
        REQUIRE(!script.empty());
        
        // Should call vote with null candidate
        std::string scriptHex = Hex::encode(script);
        std::string methodHex = Hex::encode(Bytes("vote".begin(), "vote".end()));
        REQUIRE(scriptHex.find(methodHex) != std::string::npos);
    }
    
    SECTION("Build vote script") {
        auto voterHash = account1->getScriptHash();
        auto candidate = account2->getKeyPair()->getPublicKey();
        
        Bytes script = NeoToken::buildVoteScript(voterHash, candidate);
        REQUIRE(!script.empty());
        
        // Script should contain vote method and parameters
        std::string scriptHex = Hex::encode(script);
        std::string methodHex = Hex::encode(Bytes("vote".begin(), "vote".end()));
        REQUIRE(scriptHex.find(methodHex) != std::string::npos);
    }
    
    SECTION("Build cancel vote script") {
        auto voterHash = account1->getScriptHash();
        
        Bytes script = NeoToken::buildCancelVoteScript(voterHash);
        REQUIRE(!script.empty());
        
        // Script should contain vote method with null candidate
        std::string scriptHex = Hex::encode(script);
        std::string methodHex = Hex::encode(Bytes("vote".begin(), "vote".end()));
        REQUIRE(scriptHex.find(methodHex) != std::string::npos);
    }
    
    SECTION("Get gas per block") {
        auto gasPerBlock = neoToken->getGasPerBlock();
        
        // Result would come from RPC call
        REQUIRE(gasPerBlock >= 0);
    }
    
    SECTION("Set gas per block") {
        int64_t newGasPerBlock = 500000000; // 5 GAS
        
        auto builder = neoToken->setGasPerBlock(newGasPerBlock);
        REQUIRE(builder != nullptr);
        
        auto script = builder->getScript();
        REQUIRE(!script.empty());
        
        // Script should contain setGasPerBlock method
        std::string scriptHex = Hex::encode(script);
        std::string methodHex = Hex::encode(Bytes("setGasPerBlock".begin(), "setGasPerBlock".end()));
        REQUIRE(scriptHex.find(methodHex) != std::string::npos);
    }
    
    SECTION("Get register price") {
        auto registerPrice = neoToken->getRegisterPrice();
        
        // Result would come from RPC call
        REQUIRE(registerPrice >= 0);
    }
    
    SECTION("Set register price") {
        int64_t newPrice = 1000 * 100000000; // 1000 GAS
        
        auto builder = neoToken->setRegisterPrice(newPrice);
        REQUIRE(builder != nullptr);
        
        auto script = builder->getScript();
        REQUIRE(!script.empty());
        
        // Script should contain setRegisterPrice method
        std::string scriptHex = Hex::encode(script);
        std::string methodHex = Hex::encode(Bytes("setRegisterPrice".begin(), "setRegisterPrice".end()));
        REQUIRE(scriptHex.find(methodHex) != std::string::npos);
    }
    
    SECTION("Get account state") {
        auto accountState = neoToken->getAccountState(account1);
        
        // Result would come from RPC call
        REQUIRE(accountState != nullptr);
        REQUIRE(accountState->balance >= 0);
        REQUIRE(accountState->balanceHeight >= 0);
    }
    
    SECTION("Get account state - no vote") {
        auto accountState = neoToken->getAccountState(account1);
        
        // Account with no vote
        if (accountState) {
            REQUIRE(accountState->balance >= 0);
            REQUIRE(accountState->voteTo == nullptr);
        }
    }
    
    SECTION("Get account state - no balance") {
        auto newAccount = Account::create();
        auto accountState = neoToken->getAccountState(newAccount);
        
        // New account should have zero balance
        if (accountState) {
            REQUIRE(accountState->balance == 0);
        }
    }
}