#include <catch2/catch_test_macros.hpp>
#include "neocpp/wallet/account.hpp"
#include "neocpp/crypto/ec_key_pair.hpp"
#include "neocpp/crypto/nep2.hpp"
#include "neocpp/utils/hex.hpp"
#include "neocpp/exceptions.hpp"

using namespace neocpp;

TEST_CASE("Account Tests", "[wallet]") {
    
    SECTION("Create account from key pair") {
        ECKeyPair keyPair = ECKeyPair::generate();
        auto account = std::make_shared<Account>(std::make_shared<ECKeyPair>(keyPair), "TestAccount");
        
        REQUIRE(account->getLabel() == "TestAccount");
        REQUIRE(!account->getAddress().empty());
        REQUIRE(account->getAddress()[0] == 'N'); // Neo N3 address prefix
        REQUIRE(account->getKeyPair() != nullptr);
        REQUIRE(!account->isLocked());
        REQUIRE(!account->isMultiSig());
    }
    
    SECTION("Create account from WIF") {
        const std::string wif = "L3yRvfEowWQx7VvH5n45T54rkmbwKjSP728m913EdKQVUNMebQNv";
        auto account = Account::fromWIF(wif, "WIFAccount");
        
        REQUIRE(account->getLabel() == "WIFAccount");
        REQUIRE(!account->getAddress().empty());
        REQUIRE(account->exportWIF() == wif);
    }
    
    SECTION("Create account from NEP-2") {
        const std::string privateKeyHex = "1dd37fba80fec4e6a6f13fd708d8dcb3b29def768017052f6c930fa1c5d90bbb";
        ECKeyPair keyPair(Hex::decode(privateKeyHex));
        const std::string password = "TestPassword";
        
        // Create NEP-2 encrypted key
        std::string nep2 = NEP2::encrypt(keyPair, password);
        
        // Create account from NEP-2
        auto account = Account::fromNEP2(nep2, password, "NEP2Account");
        
        REQUIRE(account->getLabel() == "NEP2Account");
        REQUIRE(!account->getAddress().empty());
        REQUIRE(account->isLocked()); // Should be locked by default
        REQUIRE(!account->getEncryptedPrivateKey().empty());
    }
    
    SECTION("Create new random account") {
        auto account = Account::create("RandomAccount");
        
        REQUIRE(account->getLabel() == "RandomAccount");
        REQUIRE(!account->getAddress().empty());
        REQUIRE(account->getKeyPair() != nullptr);
        REQUIRE(!account->isLocked());
        
        // Each account should be unique
        auto account2 = Account::create("RandomAccount2");
        REQUIRE(account->getAddress() != account2->getAddress());
    }
    
    SECTION("Lock and unlock account") {
        auto account = Account::create("LockableAccount");
        const std::string password = "SecurePassword123";
        
        // Initially unlocked
        REQUIRE(!account->isLocked());
        REQUIRE(account->getKeyPair() != nullptr);
        
        // Lock the account
        account->lock(password);
        REQUIRE(account->isLocked());
        REQUIRE(account->getKeyPair() == nullptr);
        REQUIRE(!account->getEncryptedPrivateKey().empty());
        
        // Try to unlock with wrong password
        REQUIRE_FALSE(account->unlock("WrongPassword"));
        REQUIRE(account->isLocked());
        
        // Unlock with correct password
        REQUIRE(account->unlock(password));
        REQUIRE(!account->isLocked());
        REQUIRE(account->getKeyPair() != nullptr);
    }
    
    SECTION("Sign and verify with account") {
        auto account = Account::create("SigningAccount");
        
        Bytes message = {0x01, 0x02, 0x03, 0x04, 0x05};
        Bytes signature = account->sign(message);
        
        REQUIRE(!signature.empty());
        REQUIRE(account->verify(message, signature));
        
        // Wrong message should fail verification
        Bytes wrongMessage = {0x01, 0x02, 0x03, 0x04, 0x06};
        REQUIRE_FALSE(account->verify(wrongMessage, signature));
    }
    
    SECTION("Sign with locked account throws exception") {
        auto account = Account::create("LockedSigningAccount");
        account->lock("password");
        
        Bytes message = {0x01, 0x02, 0x03, 0x04, 0x05};
        REQUIRE_THROWS_AS(account->sign(message), WalletException);
    }
    
    SECTION("Export account as WIF") {
        const std::string privateKeyHex = "1dd37fba80fec4e6a6f13fd708d8dcb3b29def768017052f6c930fa1c5d90bbb";
        ECKeyPair keyPair(Hex::decode(privateKeyHex));
        auto account = std::make_shared<Account>(std::make_shared<ECKeyPair>(keyPair), "ExportAccount");
        
        std::string wif = account->exportWIF();
        REQUIRE(!wif.empty());
        
        // Should be able to recreate account from WIF
        auto recreated = Account::fromWIF(wif);
        REQUIRE(recreated->getAddress() == account->getAddress());
    }
    
    SECTION("Export locked account as WIF returns empty") {
        auto account = Account::create("LockedExportAccount");
        account->lock("password");
        
        std::string wif = account->exportWIF();
        REQUIRE(wif.empty());
    }
    
    SECTION("Export account as NEP-2") {
        auto account = Account::create("NEP2ExportAccount");
        const std::string password = "ExportPassword";
        
        std::string nep2 = account->exportNEP2(password);
        REQUIRE(!nep2.empty());
        REQUIRE(NEP2::isValid(nep2));
        
        // Should be able to recreate account from NEP-2
        auto recreated = Account::fromNEP2(nep2, password);
        REQUIRE(recreated->getAddress() == account->getAddress());
    }
    
    SECTION("Multi-signature account") {
        // Create 3 key pairs
        std::vector<SharedPtr<ECPublicKey>> publicKeys;
        for (int i = 0; i < 3; i++) {
            ECKeyPair keyPair = ECKeyPair::generate();
            publicKeys.push_back(keyPair.getPublicKey());
        }
        
        // Create 2-of-3 multi-sig account
        auto multiSigAccount = std::make_shared<Account>(publicKeys, 2, "MultiSigAccount");
        
        REQUIRE(multiSigAccount->getLabel() == "MultiSigAccount");
        REQUIRE(!multiSigAccount->getAddress().empty());
        REQUIRE(multiSigAccount->isMultiSig());
        REQUIRE(multiSigAccount->getKeyPair() == nullptr);
        
        // Cannot lock multi-sig account
        REQUIRE_THROWS_AS(multiSigAccount->lock("password"), WalletException);
        
        // Cannot sign with multi-sig account
        Bytes message = {0x01, 0x02, 0x03};
        REQUIRE_THROWS_AS(multiSigAccount->sign(message), WalletException);
    }
    
    SECTION("Get verification script") {
        auto account = Account::create("VerificationAccount");
        
        Bytes verificationScript = account->getVerificationScript();
        REQUIRE(!verificationScript.empty());
        
        // Verification script should be consistent
        Bytes verificationScript2 = account->getVerificationScript();
        REQUIRE(verificationScript == verificationScript2);
    }
    
    SECTION("Account default flag") {
        auto account = Account::create("DefaultTestAccount");
        
        // Initially not default
        REQUIRE(!account->getIsDefault());
        
        // Set as default
        account->setIsDefault(true);
        REQUIRE(account->getIsDefault());
        
        // Unset default
        account->setIsDefault(false);
        REQUIRE(!account->getIsDefault());
    }
}