#include <catch2/catch_test_macros.hpp>
#include "neocpp/script/script_builder.hpp"
#include "neocpp/script/op_code.hpp"
#include "neocpp/crypto/ec_key_pair.hpp"
#include "neocpp/crypto/ecdsa_signature.hpp"
#include "neocpp/utils/hex.hpp"
#include "neocpp/types/hash160.hpp"
#include <algorithm>

using namespace neocpp;

TEST_CASE("VerificationScript Tests", "[script]") {
    
    SECTION("Single signature verification script") {
        // Generate a key pair
        ECKeyPair keyPair = ECKeyPair::generate();
        auto publicKey = keyPair.getPublicKey();
        
        // Build verification script for single public key
        Bytes verificationScript = ScriptBuilder::buildVerificationScript(publicKey);
        
        // Verification script format:
        // PUSH21 (0x21) + 33-byte compressed public key + SYSCALL + 4-byte hash
        REQUIRE(verificationScript.size() >= 38); // 1 + 33 + 1 + 4 minimum
        
        // First byte should be PUSH21 (33 bytes)
        REQUIRE(verificationScript[0] == 0x21);
        
        // Next 33 bytes should be the public key
        Bytes pubKeyBytes = publicKey->getEncoded();
        REQUIRE(pubKeyBytes.size() == 33); // Compressed public key
        
        for (size_t i = 0; i < 33; i++) {
            REQUIRE(verificationScript[i + 1] == pubKeyBytes[i]);
        }
        
        // After public key should be SYSCALL
        REQUIRE(verificationScript[34] == static_cast<uint8_t>(OpCode::SYSCALL));
    }
    
    SECTION("Single signature verification from raw public key bytes") {
        ECKeyPair keyPair = ECKeyPair::generate();
        Bytes pubKeyBytes = keyPair.getPublicKey()->getEncoded();
        
        // Build verification script from raw bytes
        Bytes verificationScript = ScriptBuilder::buildVerificationScript(pubKeyBytes);
        
        // Should produce the same result
        REQUIRE(verificationScript.size() >= 38);
        REQUIRE(verificationScript[0] == 0x21);
        
        for (size_t i = 0; i < 33; i++) {
            REQUIRE(verificationScript[i + 1] == pubKeyBytes[i]);
        }
        
        REQUIRE(verificationScript[34] == static_cast<uint8_t>(OpCode::SYSCALL));
    }
    
    SECTION("Multi-signature verification script (2-of-3)") {
        // Create 3 key pairs
        std::vector<SharedPtr<ECPublicKey>> publicKeys;
        for (int i = 0; i < 3; i++) {
            ECKeyPair keyPair = ECKeyPair::generate();
            publicKeys.push_back(keyPair.getPublicKey());
        }
        
        // Build 2-of-3 multi-sig verification script
        int signingThreshold = 2;
        Bytes verificationScript = ScriptBuilder::buildVerificationScript(publicKeys, signingThreshold);
        
        // Multi-sig script format:
        // PUSH2 (threshold) + [PUSH21 + pubkey] * 3 + PUSH3 (key count) + SYSCALL + hash
        REQUIRE(!verificationScript.empty());
        
        // First byte should be PUSH2 (threshold = 2)
        REQUIRE(verificationScript[0] == static_cast<uint8_t>(OpCode::PUSH2));
        
        // Should contain all three public keys (but they will be sorted)
        // Sort the keys the same way the builder does
        std::vector<SharedPtr<ECPublicKey>> sortedKeys = publicKeys;
        std::sort(sortedKeys.begin(), sortedKeys.end(), 
                  [](const SharedPtr<ECPublicKey>& a, const SharedPtr<ECPublicKey>& b) {
                      return a->getEncoded() < b->getEncoded();
                  });
        
        size_t offset = 1;
        for (const auto& pubKey : sortedKeys) {
            // Each public key should be preceded by PUSH21
            REQUIRE(verificationScript[offset] == 0x21);
            offset++;
            
            // Check the public key bytes
            Bytes keyBytes = pubKey->getEncoded();
            for (size_t i = 0; i < 33; i++) {
                REQUIRE(verificationScript[offset + i] == keyBytes[i]);
            }
            offset += 33;
        }
        
        // After all keys, should have PUSH3 (key count)
        REQUIRE(verificationScript[offset] == static_cast<uint8_t>(OpCode::PUSH3));
        offset++;
        
        // Finally SYSCALL
        REQUIRE(verificationScript[offset] == static_cast<uint8_t>(OpCode::SYSCALL));
    }
    
    SECTION("Multi-signature verification script (3-of-5)") {
        // Create 5 key pairs
        std::vector<SharedPtr<ECPublicKey>> publicKeys;
        for (int i = 0; i < 5; i++) {
            ECKeyPair keyPair = ECKeyPair::generate();
            publicKeys.push_back(keyPair.getPublicKey());
        }
        
        // Build 3-of-5 multi-sig verification script
        int signingThreshold = 3;
        Bytes verificationScript = ScriptBuilder::buildVerificationScript(publicKeys, signingThreshold);
        
        REQUIRE(!verificationScript.empty());
        
        // First byte should be PUSH3 (threshold = 3)
        REQUIRE(verificationScript[0] == static_cast<uint8_t>(OpCode::PUSH3));
        
        // Count public keys in script
        size_t offset = 1;
        int keyCount = 0;
        while (offset < verificationScript.size() && verificationScript[offset] == 0x21) {
            offset += 34; // Skip PUSH21 + 33 byte key
            keyCount++;
        }
        REQUIRE(keyCount == 5);
        
        // Should have PUSH5 (key count)
        REQUIRE(verificationScript[offset] == static_cast<uint8_t>(OpCode::PUSH5));
    }
    
    SECTION("Script hash from verification script") {
        ECKeyPair keyPair = ECKeyPair::generate();
        Bytes verificationScript = ScriptBuilder::buildVerificationScript(keyPair.getPublicKey());
        
        // The script hash is typically computed as:
        // Hash160(verificationScript)
        // This would be used as the account address
        
        REQUIRE(!verificationScript.empty());
        REQUIRE(verificationScript.size() >= 38);
        
        // We can't test the exact hash without implementing Hash160,
        // but we can verify the script structure is correct for hashing
        REQUIRE(verificationScript[0] == 0x21); // PUSH21
        REQUIRE(verificationScript[34] == static_cast<uint8_t>(OpCode::SYSCALL));
    }
    
    SECTION("Verification script for witness") {
        // In Neo, a witness consists of:
        // 1. Invocation script (contains signatures)
        // 2. Verification script (contains public keys and verification logic)
        
        ECKeyPair keyPair = ECKeyPair::generate();
        
        // Create verification script
        Bytes verificationScript = ScriptBuilder::buildVerificationScript(keyPair.getPublicKey());
        
        // Create a signature for some transaction
        Bytes txHash(32, 0xFF); // Mock transaction hash
        auto signature = keyPair.sign(txHash);
        
        // Create invocation script (contains the signature)
        ScriptBuilder invocationBuilder;
        invocationBuilder.pushData(signature->getBytes());
        Bytes invocationScript = invocationBuilder.toArray();
        
        // Both scripts should be non-empty
        REQUIRE(!verificationScript.empty());
        REQUIRE(!invocationScript.empty());
        
        // The invocation script should contain the signature
        REQUIRE((invocationScript[0] == 0x40 || invocationScript[0] == 0x41)); // 64 or 65 bytes
        
        // The verification script should contain the public key
        REQUIRE(verificationScript[0] == 0x21); // PUSH21
    }
    
    SECTION("Custom verification script") {
        // Build a custom verification script that always returns true
        ScriptBuilder builder;
        builder.pushBool(true);  // Push true
        builder.emit(OpCode::RET); // Return
        
        Bytes customScript = builder.toArray();
        
        REQUIRE(customScript.size() == 2);
        REQUIRE(customScript[0] == static_cast<uint8_t>(OpCode::PUSH1)); // true = PUSH1
        REQUIRE(customScript[1] == static_cast<uint8_t>(OpCode::RET));
    }
    
    SECTION("Verification script with contract call") {
        // Build a verification script that calls a smart contract
        ScriptBuilder builder;
        
        // Push parameters
        builder.pushString("verify");
        builder.pushInteger(1); // parameter count
        
        // Push contract hash (20 bytes)
        Hash160 contractHash("23ba2703c53263e8d6e522dc32203339dcd8eee9");
        builder.pushData(contractHash.toLittleEndianArray());
        
        // Call the contract
        builder.emitSysCall("System.Contract.Call");
        
        Bytes verificationScript = builder.toArray();
        
        REQUIRE(!verificationScript.empty());
        
        // Should contain the method name "verify"
        std::string verify = "verify";
        bool containsVerify = false;
        for (size_t i = 0; i <= verificationScript.size() - verify.length(); i++) {
            if (std::equal(verify.begin(), verify.end(), verificationScript.begin() + i)) {
                containsVerify = true;
                break;
            }
        }
        REQUIRE(containsVerify);
    }
    
    SECTION("Empty verification script") {
        // An empty verification script is invalid
        ScriptBuilder builder;
        Bytes emptyScript = builder.toArray();
        
        REQUIRE(emptyScript.empty());
        
        // This would fail verification as there's no logic
    }
}