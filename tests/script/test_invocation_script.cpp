#include <catch2/catch_test_macros.hpp>
#include "neocpp/script/script_builder.hpp"
#include "neocpp/script/op_code.hpp"
#include "neocpp/crypto/ec_key_pair.hpp"
#include "neocpp/crypto/ecdsa_signature.hpp"
#include "neocpp/utils/hex.hpp"

using namespace neocpp;

TEST_CASE("InvocationScript", "[script]") {
    
    SECTION("Create invocation script with single signature") {
        // Create a signature
        Bytes signatureBytes(64, 0xAB); // Mock signature
        ECDSASignature signature(signatureBytes);
        
        // Build invocation script (pushes signature onto stack)
        ScriptBuilder builder;
        builder.pushData(signature.getBytes());
        Bytes invocationScript = builder.toArray();
        
        // Invocation script should be: length_byte + signature
        REQUIRE(invocationScript.size() == 65); // 1 byte length + 64 byte signature
        REQUIRE(invocationScript[0] == 0x40); // 64 bytes
        for (size_t i = 0; i < 64; i++) {
            REQUIRE(invocationScript[i + 1] == 0xAB);
        }
    }
    
    SECTION("Create invocation script with multiple signatures") {
        // Create multiple signatures for multi-sig
        std::vector<Bytes> signatures;
        for (int i = 0; i < 3; i++) {
            Bytes sig(64, static_cast<uint8_t>(0xA0 + i));
            signatures.push_back(sig);
        }
        
        // Build invocation script (pushes all signatures)
        ScriptBuilder builder;
        for (const auto& sig : signatures) {
            builder.pushData(sig);
        }
        Bytes invocationScript = builder.toArray();
        
        // Should contain all three signatures
        REQUIRE(invocationScript.size() == 3 * 65); // 3 * (1 byte length + 64 byte signature)
        
        // Check first signature
        REQUIRE(invocationScript[0] == 0x40); // 64 bytes
        for (size_t i = 0; i < 64; i++) {
            REQUIRE(invocationScript[i + 1] == 0xA0);
        }
        
        // Check second signature
        REQUIRE(invocationScript[65] == 0x40); // 64 bytes
        for (size_t i = 0; i < 64; i++) {
            REQUIRE(invocationScript[66 + i] == 0xA1);
        }
    }
    
    SECTION("Create invocation script from actual signature") {
        // Create a key pair
        ECKeyPair keyPair = ECKeyPair::generate();
        
        // Sign some data
        Bytes message = Hex::decode("0102030405060708090a0b0c0d0e0f10");
        auto signature = keyPair.sign(message);
        
        // Build invocation script
        ScriptBuilder builder;
        builder.pushData(signature->getBytes());
        Bytes invocationScript = builder.toArray();
        
        // Invocation script should contain the signature
        REQUIRE(invocationScript.size() > 64); // At least signature size
        REQUIRE((invocationScript[0] == 0x40 || invocationScript[0] == 0x41)); // 64 or 65 bytes
        
        // Extract signature from script
        Bytes extractedSig(invocationScript.begin() + 1, invocationScript.begin() + 1 + signature->getBytes().size());
        REQUIRE(extractedSig == signature->getBytes());
    }
    
    SECTION("Empty invocation script") {
        ScriptBuilder builder;
        Bytes invocationScript = builder.toArray();
        
        REQUIRE(invocationScript.empty());
    }
    
    SECTION("Invocation script with contract parameters") {
        ScriptBuilder builder;
        
        // Push various parameters as you would for a contract invocation
        builder.pushInteger(42);           // Integer parameter
        builder.pushString("test");        // String parameter
        builder.pushBool(true);           // Boolean parameter
        
        // Push the method name
        builder.pushString("transfer");
        
        // Push parameter count
        builder.pushInteger(3);
        
        Bytes invocationScript = builder.toArray();
        
        REQUIRE(!invocationScript.empty());
        
        // First should be PUSH integer (42)
        // The exact encoding depends on the value
        size_t offset = 0;
        if (invocationScript[offset] == static_cast<uint8_t>(OpCode::PUSHINT8)) {
            REQUIRE(invocationScript[offset + 1] == 42);
            offset += 2;
        }
        
        // Then string "test"
        REQUIRE(invocationScript[offset] == 4); // Length of "test"
        REQUIRE(invocationScript[offset + 1] == 't');
        REQUIRE(invocationScript[offset + 2] == 'e');
        REQUIRE(invocationScript[offset + 3] == 's');
        REQUIRE(invocationScript[offset + 4] == 't');
    }
    
    SECTION("Invocation script for witness") {
        // For a witness, the invocation script contains the signature(s)
        // that satisfy the verification script
        
        ECKeyPair keyPair = ECKeyPair::generate();
        Bytes messageHash(32, 0xFF); // Mock transaction hash
        
        auto signature = keyPair.sign(messageHash);
        
        // Build invocation script for witness
        ScriptBuilder builder;
        builder.pushData(signature->getBytes());
        Bytes invocationScript = builder.toArray();
        
        // This invocation script would be paired with a verification script
        // that checks this signature against the public key
        REQUIRE(invocationScript.size() == signature->getBytes().size() + 1);
        REQUIRE((invocationScript[0] == 0x40 || invocationScript[0] == 0x41));
    }
}