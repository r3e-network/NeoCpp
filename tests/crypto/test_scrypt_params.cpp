#include <catch2/catch_test_macros.hpp>
#include "neocpp/crypto/scrypt_params.hpp"

using namespace neocpp;

TEST_CASE("ScryptParams Tests", "[crypto]") {
    
    SECTION("Create ScryptParams with values") {
        ScryptParams params(8, 8, 9);  // N must be power of 2
        REQUIRE(params.getN() == 8);
        REQUIRE(params.getR() == 8);
        REQUIRE(params.getP() == 9);
        REQUIRE(params.getDkLen() == 64);  // Default dkLen
    }
    
    SECTION("Create ScryptParams with custom dkLen") {
        ScryptParams params(1024, 8, 1, 32);
        REQUIRE(params.getN() == 1024);
        REQUIRE(params.getR() == 8);
        REQUIRE(params.getP() == 1);
        REQUIRE(params.getDkLen() == 32);
    }
    
    SECTION("Default ScryptParams values") {
        ScryptParams params = ScryptParams::getDefault();
        // Default values based on NEP-2 standard
        REQUIRE(params.getN() == 16384);
        REQUIRE(params.getR() == 8);
        REQUIRE(params.getP() == 8);
        REQUIRE(params.getDkLen() == 64);
    }
    
    SECTION("Light ScryptParams values") {
        ScryptParams params = ScryptParams::getLight();
        // Light values for testing
        REQUIRE(params.getN() == 256);
        REQUIRE(params.getR() == 1);
        REQUIRE(params.getP() == 1);
        REQUIRE(params.getDkLen() == 64);
    }
    
    SECTION("Validate parameter constraints") {
        // Valid parameters
        ScryptParams valid1(16384, 8, 8);
        REQUIRE(valid1.isValid());
        
        ScryptParams valid2(1024, 8, 8);
        REQUIRE(valid2.isValid());
        
        ScryptParams valid3(2048, 8, 8);
        REQUIRE(valid3.isValid());
        
        // Edge cases
        ScryptParams minValid(2, 1, 1);  // Minimum valid values
        REQUIRE(minValid.isValid());
        
        // Test with different dkLen values
        ScryptParams customDkLen(16384, 8, 8, 32);
        REQUIRE(customDkLen.isValid());
        REQUIRE(customDkLen.getDkLen() == 32);
    }
    
    SECTION("Compare different parameter sets") {
        ScryptParams params1(16384, 8, 8);
        ScryptParams params2(16384, 8, 8);
        ScryptParams params3(1024, 8, 8);
        ScryptParams params4(16384, 4, 8);
        ScryptParams params5(16384, 8, 4);
        ScryptParams params6(16384, 8, 8, 32);  // Different dkLen
        
        // Check that all parameters are accessible
        REQUIRE(params1.getN() == params2.getN());
        REQUIRE(params1.getR() == params2.getR());
        REQUIRE(params1.getP() == params2.getP());
        REQUIRE(params1.getDkLen() == params2.getDkLen());
        
        REQUIRE(params1.getN() != params3.getN());
        REQUIRE(params1.getR() != params4.getR());
        REQUIRE(params1.getP() != params5.getP());
        REQUIRE(params1.getDkLen() != params6.getDkLen());
    }
    
    SECTION("Multiple parameter combinations") {
        // Test various combinations that should be valid
        struct TestCase {
            int n;
            int r;
            int p;
            int dkLen;
            bool shouldBeValid;
        };
        
        std::vector<TestCase> testCases = {
            {16384, 8, 8, 64, true},   // NEP-2 default
            {256, 1, 1, 64, true},      // Light params
            {1024, 8, 1, 32, true},     // Custom params
            {2048, 4, 2, 64, true},     // Another valid combination
            {4096, 16, 1, 128, true},   // High r value
            {8192, 8, 2, 64, true},     // Medium params
        };
        
        for (const auto& tc : testCases) {
            ScryptParams params(tc.n, tc.r, tc.p, tc.dkLen);
            REQUIRE(params.getN() == tc.n);
            REQUIRE(params.getR() == tc.r);
            REQUIRE(params.getP() == tc.p);
            REQUIRE(params.getDkLen() == tc.dkLen);
            if (tc.shouldBeValid) {
                REQUIRE(params.isValid());
            }
        }
    }
}