#include <catch2/catch_test_macros.hpp>
#include "neocpp/crypto/ecdsa_signature.hpp"
#include "neocpp/crypto/ec_key_pair.hpp"
#include "neocpp/utils/hex.hpp"
#include <vector>

using namespace neocpp;

TEST_CASE("ECDSASignature Tests", "[crypto]") {
    
    SECTION("Constructor from bytes") {
        Bytes sigBytes(64, 0xAB);
        ECDSASignature signature(sigBytes);
        
        REQUIRE(signature.getBytes() == sigBytes);
        REQUIRE(signature.getBytes().size() == 64);
    }
    
    SECTION("Constructor from R and S components") {
        Bytes r(32, 0x11);
        Bytes s(32, 0x22);
        
        ECDSASignature signature(r, s);
        
        Bytes combined = signature.getBytes();
        REQUIRE(combined.size() == 64);
        
        // First 32 bytes should be R
        for (size_t i = 0; i < 32; i++) {
            REQUIRE(combined[i] == 0x11);
        }
        
        // Last 32 bytes should be S
        for (size_t i = 32; i < 64; i++) {
            REQUIRE(combined[i] == 0x22);
        }
    }
    
    SECTION("Get R and S components") {
        Bytes r(32, 0xAA);
        Bytes s(32, 0xBB);
        
        ECDSASignature signature(r, s);
        
        REQUIRE(signature.getR() == r);
        REQUIRE(signature.getS() == s);
        REQUIRE(signature.getR().size() == 32);
        REQUIRE(signature.getS().size() == 32);
    }
    
    SECTION("Convert to DER format") {
        // Create a signature with known R and S values
        Bytes r = Hex::decode("0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20");
        Bytes s = Hex::decode("2122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f40");
        
        ECDSASignature signature(r, s);
        
        Bytes der = signature.toDER();
        
        // DER format should start with 0x30 (SEQUENCE)
        REQUIRE(der[0] == 0x30);
        
        // Should contain the signature data
        REQUIRE(der.size() > 64); // DER encoding adds overhead
    }
    
    SECTION("Create from DER format") {
        // First create a signature and convert to DER
        Bytes r(32, 0xCC);
        Bytes s(32, 0xDD);
        ECDSASignature original(r, s);
        
        Bytes der = original.toDER();
        
        // Parse back from DER
        ECDSASignature parsed = ECDSASignature::fromDER(der);
        
        // Should match original
        REQUIRE(parsed.getR() == original.getR());
        REQUIRE(parsed.getS() == original.getS());
        REQUIRE(parsed.getBytes() == original.getBytes());
    }
    
    SECTION("Sign and verify with key pair") {
        ECKeyPair keyPair = ECKeyPair::generate();
        
        Bytes message = Hex::decode("48656c6c6f20576f726c64"); // "Hello World"
        
        // Sign the message
        auto signature = keyPair.sign(message);
        
        REQUIRE(signature != nullptr);
        REQUIRE(signature->getBytes().size() == 64);
        
        // Verify the signature
        bool valid = keyPair.getPublicKey()->verify(message, signature);
        REQUIRE(valid == true);
        
        // Verify with wrong message should fail
        Bytes wrongMessage = Hex::decode("48656c6c6f20576f726c6421"); // "Hello World!"
        valid = keyPair.getPublicKey()->verify(wrongMessage, signature);
        REQUIRE(valid == false);
    }
    
    SECTION("Signature uniqueness") {
        ECKeyPair keyPair = ECKeyPair::generate();
        Bytes message = Hex::decode("0102030405060708");
        
        // Sign the same message twice
        auto sig1 = keyPair.sign(message);
        auto sig2 = keyPair.sign(message);
        
        REQUIRE(sig1 != nullptr);
        REQUIRE(sig2 != nullptr);
        
        // Signatures may be different due to randomness in ECDSA
        // But both should be valid
        REQUIRE(keyPair.getPublicKey()->verify(message, sig1) == true);
        REQUIRE(keyPair.getPublicKey()->verify(message, sig2) == true);
    }
    
    SECTION("Empty signature handling") {
        Bytes empty;
        
        // Creating signature with empty bytes should work but produce invalid signature
        ECDSASignature emptySignature(empty);
        REQUIRE(emptySignature.getBytes().empty());
    }
    
    SECTION("Large signature components") {
        // Test with maximum size components
        Bytes largeR(32, 0xFF);
        Bytes largeS(32, 0xFF);
        
        ECDSASignature signature(largeR, largeS);
        
        REQUIRE(signature.getR() == largeR);
        REQUIRE(signature.getS() == largeS);
        
        // Should be convertible to DER
        Bytes der = signature.toDER();
        REQUIRE(!der.empty());
        
        // Should be parseable from DER
        ECDSASignature parsed = ECDSASignature::fromDER(der);
        REQUIRE(parsed.getR() == largeR);
        REQUIRE(parsed.getS() == largeS);
    }
    
    SECTION("Signature serialization consistency") {
        Bytes r = Hex::decode("123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef0");
        Bytes s = Hex::decode("fedcba9876543210fedcba9876543210fedcba9876543210fedcba9876543210");
        
        ECDSASignature sig1(r, s);
        ECDSASignature sig2(r, s);
        
        // Same R and S should produce same signature bytes
        REQUIRE(sig1.getBytes() == sig2.getBytes());
        
        // DER encoding should also be consistent
        REQUIRE(sig1.toDER() == sig2.toDER());
    }
    
    SECTION("Verify with different public key") {
        ECKeyPair keyPair1 = ECKeyPair::generate();
        ECKeyPair keyPair2 = ECKeyPair::generate();
        
        Bytes message = Hex::decode("0123456789abcdef");
        
        // Sign with first key
        auto signature = keyPair1.sign(message);
        
        // Verify with first key should succeed
        REQUIRE(keyPair1.getPublicKey()->verify(message, signature) == true);
        
        // Verify with second key should fail
        REQUIRE(keyPair2.getPublicKey()->verify(message, signature) == false);
    }
}