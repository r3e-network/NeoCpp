#include <catch2/catch_test_macros.hpp>
#include <neocpp/neocpp.hpp>

using namespace neocpp;

TEST_CASE("Wallet operations", "[wallet]") {
    SECTION("Create new wallet") {
        auto wallet = std::make_shared<Wallet>("Test Wallet");
        REQUIRE(wallet->getName() == "Test Wallet");
        REQUIRE(wallet->isEmpty());
    }
    
    SECTION("Create account") {
        auto wallet = std::make_shared<Wallet>("Test Wallet");
        auto account = wallet->createAccount("Test Account");
        
        REQUIRE(account != nullptr);
        REQUIRE(account->getLabel() == "Test Account");
        REQUIRE(!account->getAddress().empty());
        REQUIRE(wallet->size() == 1);
    }
    
    SECTION("Import account from WIF") {
        auto wallet = std::make_shared<Wallet>("Test Wallet");
        
        // Generate a key pair and export as WIF
        auto keyPair = ECKeyPair::generate();
        std::string wif = keyPair.exportAsWIF();
        
        // Import into wallet
        auto account = wallet->importFromWIF(wif, "Imported Account");
        REQUIRE(account != nullptr);
        REQUIRE(account->getLabel() == "Imported Account");
        REQUIRE(wallet->containsAccount(account->getAddress()));
    }
    
    SECTION("Get account by address") {
        auto wallet = std::make_shared<Wallet>("Test Wallet");
        auto account = wallet->createAccount("Test Account");
        std::string address = account->getAddress();
        
        auto retrieved = wallet->getAccount(address);
        REQUIRE(retrieved != nullptr);
        REQUIRE(retrieved->getAddress() == address);
    }
}