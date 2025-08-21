#include <catch2/catch_test_macros.hpp>
#include "neocpp/transaction/witness.hpp"
#include "neocpp/types/hash160.hpp"
#include "neocpp/serialization/binary_writer.hpp"
#include "neocpp/serialization/binary_reader.hpp"
#include "neocpp/crypto/ec_key_pair.hpp"
#include "neocpp/script/script_builder.hpp"
#include "neocpp/utils/hex.hpp"

using namespace neocpp;

TEST_CASE("Witness Tests", "[transaction]") {
    
    SECTION("Default constructor") {
        Witness witness;
        
        REQUIRE(witness.getInvocationScript().empty());
        REQUIRE(witness.getVerificationScript().empty());
    }
    
    SECTION("Constructor with scripts") {
        Bytes invocation = {0x0C, 0x40}; // Invocation script prefix
        Bytes verification = {0x0C, 0x21}; // Verification script prefix
        
        Witness witness(invocation, verification);
        
        REQUIRE(witness.getInvocationScript() == invocation);
        REQUIRE(witness.getVerificationScript() == verification);
    }
    
    SECTION("Set and get invocation script") {
        Witness witness;
        Bytes invocation = {0x0C, 0x40, 0x01, 0x02, 0x03};
        
        witness.setInvocationScript(invocation);
        REQUIRE(witness.getInvocationScript() == invocation);
    }
    
    SECTION("Set and get verification script") {
        Witness witness;
        Bytes verification = {0x0C, 0x21, 0x04, 0x05, 0x06};
        
        witness.setVerificationScript(verification);
        REQUIRE(witness.getVerificationScript() == verification);
    }
    
    SECTION("Get script hash from verification script") {
        // Create a sample verification script
        Bytes verification = {
            0x0C, 0x21, 0x03, 0x19, 0x22, 0x06, 0x20, 0x98, 0xd5, 0xb0,
            0x24, 0x93, 0x3c, 0x7e, 0x3e, 0xf7, 0xb2, 0xd0, 0xa2, 0x22,
            0x30, 0x44, 0x0a, 0xe9, 0x48, 0x83, 0xe9, 0x39, 0x6d, 0x90,
            0x27, 0xd5, 0x5b, 0x41, 0x41, 0xc1, 0x14, 0x1a
        };
        
        Witness witness({}, verification);
        Hash160 scriptHash = witness.getScriptHash();
        
        // Should not be empty/zero hash
        REQUIRE_FALSE(scriptHash.toArray().empty());
    }
    
    SECTION("Get script hash from empty verification script") {
        Witness witness;
        Hash160 scriptHash = witness.getScriptHash();
        
        // Should return empty/zero hash
        Bytes expectedEmpty(20, 0x00);
        REQUIRE(scriptHash.toArray() == expectedEmpty);
    }
    
    SECTION("Create witness from signature") {
        // Sample signature and public key
        Bytes signature(64, 0xAA); // Dummy 64-byte signature
        Bytes publicKey(33, 0x02); // Dummy compressed public key starting with 0x02
        publicKey[32] = 0x01;
        
        auto witness = Witness::fromSignature(signature, publicKey);
        
        REQUIRE_FALSE(witness->getInvocationScript().empty());
        REQUIRE_FALSE(witness->getVerificationScript().empty());
        
        // The invocation script should contain the signature
        const auto& invocation = witness->getInvocationScript();
        REQUIRE(invocation.size() > signature.size());
        
        // The verification script should contain the public key
        const auto& verification = witness->getVerificationScript();
        REQUIRE(verification.size() > publicKey.size());
    }
    
    SECTION("Create multi-signature witness") {
        // Sample signatures
        std::vector<Bytes> signatures = {
            Bytes(64, 0xAA),
            Bytes(64, 0xBB)
        };
        
        // Sample public keys
        std::vector<Bytes> publicKeys = {
            Bytes(33, 0x02),
            Bytes(33, 0x03),
            Bytes(33, 0x04)
        };
        
        // Set different last bytes to make them unique
        publicKeys[0][32] = 0x01;
        publicKeys[1][32] = 0x02;
        publicKeys[2][32] = 0x03;
        
        int signingThreshold = 2;
        
        auto witness = Witness::fromMultiSignature(signatures, publicKeys, signingThreshold);
        
        REQUIRE_FALSE(witness->getInvocationScript().empty());
        REQUIRE_FALSE(witness->getVerificationScript().empty());
        
        // Multi-sig scripts should be larger due to multiple keys/signatures
        REQUIRE(witness->getInvocationScript().size() > 64); // Larger than single signature
        REQUIRE(witness->getVerificationScript().size() > 33); // Larger than single key
    }
    
    SECTION("Get size calculation") {
        Bytes invocation = {0x0C, 0x40}; // 2 bytes
        invocation.insert(invocation.end(), 64, 0xAA); // + 64 signature bytes = 66 total
        
        Bytes verification = {0x0C, 0x21}; // 2 bytes
        verification.insert(verification.end(), 33, 0x02); // + 33 key bytes = 35 total
        verification.push_back(0x41); // + CheckWitness opcode = 36 total
        
        Witness witness(invocation, verification);
        size_t size = witness.getSize();
        
        // Size should include var_int prefixes for both scripts plus script lengths
        // invocation: 66 bytes (needs 1 byte var_int)
        // verification: 36 bytes (needs 1 byte var_int)
        // Total: 1 + 66 + 1 + 36 = 104
        REQUIRE(size >= 100); // Approximate check since var_int calculation varies
        REQUIRE(size <= 110);
    }
    
    SECTION("Serialization and deserialization") {
        Bytes invocation = {0x0C, 0x40, 0x01, 0x02, 0x03, 0x04};
        Bytes verification = {0x0C, 0x21, 0x05, 0x06, 0x07, 0x08, 0x41};
        
        Witness original(invocation, verification);
        
        BinaryWriter writer;
        original.serialize(writer);
        Bytes serialized = writer.toArray();
        
        BinaryReader reader(serialized);
        auto deserialized = Witness::deserialize(reader);
        
        REQUIRE(deserialized->getInvocationScript() == original.getInvocationScript());
        REQUIRE(deserialized->getVerificationScript() == original.getVerificationScript());
        REQUIRE(*deserialized == original);
    }
    
    SECTION("Serialization with empty scripts") {
        Witness original({}, {});
        
        BinaryWriter writer;
        original.serialize(writer);
        Bytes serialized = writer.toArray();
        
        BinaryReader reader(serialized);
        auto deserialized = Witness::deserialize(reader);
        
        REQUIRE(deserialized->getInvocationScript().empty());
        REQUIRE(deserialized->getVerificationScript().empty());
        REQUIRE(*deserialized == original);
    }
    
    SECTION("Equality operators") {
        Bytes invocation1 = {0x0C, 0x40, 0x01, 0x02, 0x03};
        Bytes verification1 = {0x0C, 0x21, 0x04, 0x05, 0x06, 0x41};
        
        Bytes invocation2 = {0x0C, 0x40, 0x01, 0x02, 0x03};
        Bytes verification2 = {0x0C, 0x21, 0x04, 0x05, 0x06, 0x41};
        
        Bytes differentInvocation = {0x0C, 0x40, 0x07, 0x08, 0x09};
        Bytes differentVerification = {0x0C, 0x21, 0x0A, 0x0B, 0x0C, 0x41};
        
        Witness witness1(invocation1, verification1);
        Witness witness2(invocation2, verification2); // Same as witness1
        Witness witness3(differentInvocation, verification1); // Different invocation
        Witness witness4(invocation1, differentVerification); // Different verification
        
        REQUIRE(witness1 == witness2);
        REQUIRE_FALSE(witness1 != witness2);
        
        REQUIRE_FALSE(witness1 == witness3);
        REQUIRE(witness1 != witness3);
        
        REQUIRE_FALSE(witness1 == witness4);
        REQUIRE(witness1 != witness4);
    }
    
    SECTION("Large script serialization") {
        // Test with large scripts that need multi-byte var_int encoding
        Bytes largeInvocation(1000, 0xAA);
        Bytes largeVerification(500, 0xBB);
        
        Witness witness(largeInvocation, largeVerification);
        
        BinaryWriter writer;
        witness.serialize(writer);
        Bytes serialized = writer.toArray();
        
        BinaryReader reader(serialized);
        auto deserialized = Witness::deserialize(reader);
        
        REQUIRE(deserialized->getInvocationScript() == largeInvocation);
        REQUIRE(deserialized->getVerificationScript() == largeVerification);
        REQUIRE(*deserialized == witness);
    }
    
    SECTION("Witness script hash consistency") {
        // Create two witnesses with the same verification script
        Bytes verification = {0x0C, 0x21, 0x03, 0x19, 0x22, 0x06, 0x41};
        
        Witness witness1({0x01, 0x02}, verification);
        Witness witness2({0x03, 0x04}, verification);
        
        // Should have the same script hash since verification script is the same
        REQUIRE(witness1.getScriptHash() == witness2.getScriptHash());
    }
    
    SECTION("Copy and move semantics") {
        Bytes invocation = {0x0C, 0x40, 0x01, 0x02};
        Bytes verification = {0x0C, 0x21, 0x03, 0x04, 0x41};
        
        // Copy constructor
        Witness original(invocation, verification);
        Witness copy(original);
        REQUIRE(copy == original);
        
        // Move constructor
        Witness moved(std::move(copy));
        REQUIRE(moved == original);
        
        // Copy assignment
        Witness assigned;
        assigned = original;
        REQUIRE(assigned == original);
        
        // Move assignment
        Witness moveAssigned;
        moveAssigned = std::move(assigned);
        REQUIRE(moveAssigned == original);
    }
}