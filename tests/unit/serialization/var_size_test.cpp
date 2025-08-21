#include <catch2/catch_test_macros.hpp>
#include "neocpp/serialization/binary_writer.hpp"
#include "neocpp/serialization/binary_reader.hpp"

using namespace neocpp;

TEST_CASE("var_size tests", "[var_size]") {
    SECTION("Variable integer encoding sizes") {
        // Test size calculation for variable integers
        REQUIRE(BinaryWriter::getVarSize(0) == 1);
        REQUIRE(BinaryWriter::getVarSize(252) == 1);
        REQUIRE(BinaryWriter::getVarSize(253) == 3);
        REQUIRE(BinaryWriter::getVarSize(65535) == 3);
        REQUIRE(BinaryWriter::getVarSize(65536) == 5);
        REQUIRE(BinaryWriter::getVarSize(0xFFFFFFFF) == 5);
        REQUIRE(BinaryWriter::getVarSize(0x100000000) == 9);
    }
    
    SECTION("String size calculation") {
        std::string shortStr(10, 'A');
        std::string mediumStr(252, 'B');
        std::string longStr(300, 'C');
        
        // Size = length encoding + string data
        REQUIRE(BinaryWriter::getVarSize(shortStr) == 1 + 10);
        REQUIRE(BinaryWriter::getVarSize(mediumStr) == 1 + 252);
        REQUIRE(BinaryWriter::getVarSize(longStr) == 3 + 300);
    }
}
