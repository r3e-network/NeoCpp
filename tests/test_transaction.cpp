#include <catch2/catch_test_macros.hpp>
#include <neocpp/neocpp.hpp>

using namespace neocpp;

TEST_CASE("Transaction operations", "[transaction]") {
    SECTION("Create empty transaction") {
        auto tx = std::make_shared<Transaction>();
        REQUIRE(tx->getVersion() == NeoConstants::CURRENT_TX_VERSION);
        REQUIRE(tx->getSigners().empty());
        REQUIRE(tx->getWitnesses().empty());
    }
    
    SECTION("Add signer to transaction") {
        auto tx = std::make_shared<Transaction>();
        
        // Create a signer
        Hash160 account;
        auto signer = std::make_shared<Signer>(account, WitnessScope::CALLED_BY_ENTRY);
        tx->addSigner(signer);
        
        REQUIRE(tx->getSigners().size() == 1);
        REQUIRE(tx->getSigners()[0]->getScopes() == WitnessScope::CALLED_BY_ENTRY);
    }
    
    SECTION("Transaction hash calculation") {
        auto tx = std::make_shared<Transaction>();
        tx->setNonce(12345);
        tx->setSystemFee(1000000);
        tx->setNetworkFee(500000);
        tx->setValidUntilBlock(1000);
        
        Hash256 hash1 = tx->getHash();
        Hash256 hash2 = tx->getHash(); // Should be cached
        
        REQUIRE(hash1 == hash2);
    }
    
    SECTION("Witness scope operations") {
        auto scopes = WitnessScope::CALLED_BY_ENTRY | WitnessScope::CUSTOM_CONTRACTS;
        uint8_t combined = WitnessScopeHelper::combineScopes({WitnessScope::CALLED_BY_ENTRY, WitnessScope::CUSTOM_CONTRACTS});
        
        REQUIRE(combined == (0x01 | 0x10));
        
        auto extracted = WitnessScopeHelper::extractCombinedScopes(combined);
        REQUIRE(extracted.size() == 2);
    }
}