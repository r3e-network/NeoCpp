#include <catch2/catch_test_macros.hpp>
#include "neocpp/types/contract_parameter.hpp"
#include "neocpp/types/hash160.hpp"
#include "neocpp/types/hash256.hpp"
#include "neocpp/crypto/ec_key_pair.hpp"
#include "neocpp/utils/hex.hpp"
#include <string>
#include <vector>

using namespace neocpp;

TEST_CASE("ContractParameter Tests", "[types]") {
    
    SECTION("Boolean parameter") {
        ContractParameter paramTrue(true);
        ContractParameter paramFalse(false);
        
        REQUIRE(paramTrue.getType() == ContractParameterType::BOOLEAN);
        REQUIRE(paramTrue.getBoolean() == true);
        
        REQUIRE(paramFalse.getType() == ContractParameterType::BOOLEAN);
        REQUIRE(paramFalse.getBoolean() == false);
    }
    
    SECTION("Integer parameter") {
        ContractParameter param1(int64_t(42));
        ContractParameter param2(int64_t(-100));
        ContractParameter param3(int64_t(0));
        
        REQUIRE(param1.getType() == ContractParameterType::INTEGER);
        REQUIRE(param1.getInteger() == 42);
        
        REQUIRE(param2.getType() == ContractParameterType::INTEGER);
        REQUIRE(param2.getInteger() == -100);
        
        REQUIRE(param3.getType() == ContractParameterType::INTEGER);
        REQUIRE(param3.getInteger() == 0);
    }
    
    SECTION("String parameter") {
        ContractParameter param1("Hello World");
        ContractParameter param2("");
        ContractParameter param3("Neo Smart Contract");
        
        REQUIRE(param1.getType() == ContractParameterType::STRING);
        REQUIRE(param1.getString() == "Hello World");
        
        REQUIRE(param2.getType() == ContractParameterType::STRING);
        REQUIRE(param2.getString() == "");
        
        REQUIRE(param3.getType() == ContractParameterType::STRING);
        REQUIRE(param3.getString() == "Neo Smart Contract");
    }
    
    SECTION("ByteArray parameter") {
        Bytes bytes1 = {0x01, 0x02, 0x03, 0x04};
        Bytes bytes2 = {};
        Bytes bytes3(100, 0xFF);
        
        ContractParameter param1(bytes1);
        ContractParameter param2(bytes2);
        ContractParameter param3(bytes3);
        
        REQUIRE(param1.getType() == ContractParameterType::BYTE_ARRAY);
        REQUIRE(param1.getByteArray() == bytes1);
        
        REQUIRE(param2.getType() == ContractParameterType::BYTE_ARRAY);
        REQUIRE(param2.getByteArray().empty());
        
        REQUIRE(param3.getType() == ContractParameterType::BYTE_ARRAY);
        REQUIRE(param3.getByteArray().size() == 100);
    }
    
    SECTION("Hash160 parameter") {
        Hash160 hash1("0x0102030405060708090a0b0c0d0e0f1011121314");
        Hash160 hash2("0xabcdef1234567890abcdef1234567890abcdef12");
        
        ContractParameter param1(hash1);
        ContractParameter param2(hash2);
        
        REQUIRE(param1.getType() == ContractParameterType::HASH160);
        REQUIRE(param1.getHash160() == hash1);
        
        REQUIRE(param2.getType() == ContractParameterType::HASH160);
        REQUIRE(param2.getHash160() == hash2);
    }
    
    SECTION("Hash256 parameter") {
        Hash256 hash1("0x0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20");
        Hash256 hash2("0xabcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890");
        
        ContractParameter param1(hash1);
        ContractParameter param2(hash2);
        
        REQUIRE(param1.getType() == ContractParameterType::HASH256);
        REQUIRE(param1.getHash256() == hash1);
        
        REQUIRE(param2.getType() == ContractParameterType::HASH256);
        REQUIRE(param2.getHash256() == hash2);
    }
    
    SECTION("PublicKey parameter") {
        ECKeyPair keyPair1 = ECKeyPair::generate();
        ECKeyPair keyPair2 = ECKeyPair::generate();
        
        ContractParameter param1(keyPair1.getPublicKey());
        ContractParameter param2(keyPair2.getPublicKey());
        
        REQUIRE(param1.getType() == ContractParameterType::PUBLIC_KEY);
        REQUIRE(param1.getPublicKey() == keyPair1.getPublicKey());
        
        REQUIRE(param2.getType() == ContractParameterType::PUBLIC_KEY);
        REQUIRE(param2.getPublicKey() == keyPair2.getPublicKey());
    }
    
    SECTION("Array parameter") {
        std::vector<ContractParameter> array1;
        array1.push_back(ContractParameter(int64_t(1)));
        array1.push_back(ContractParameter("test"));
        array1.push_back(ContractParameter(true));
        
        ContractParameter param1(array1);
        
        REQUIRE(param1.getType() == ContractParameterType::ARRAY);
        REQUIRE(param1.getArray().size() == 3);
        REQUIRE(param1.getArray()[0].getInteger() == 1);
        REQUIRE(param1.getArray()[1].getString() == "test");
        REQUIRE(param1.getArray()[2].getBoolean() == true);
        
        // Empty array
        std::vector<ContractParameter> emptyArray;
        ContractParameter param2(emptyArray);
        
        REQUIRE(param2.getType() == ContractParameterType::ARRAY);
        REQUIRE(param2.getArray().empty());
    }
    
    SECTION("Map parameter") {
        std::map<ContractParameter, ContractParameter> map1;
        map1[ContractParameter("key1")] = ContractParameter(int64_t(100));
        map1[ContractParameter("key2")] = ContractParameter(false);
        
        ContractParameter param1(map1);
        
        REQUIRE(param1.getType() == ContractParameterType::MAP);
        REQUIRE(param1.getMap().size() == 2);
        
        // Empty map
        std::map<ContractParameter, ContractParameter> emptyMap;
        ContractParameter param2(emptyMap);
        
        REQUIRE(param2.getType() == ContractParameterType::MAP);
        REQUIRE(param2.getMap().empty());
    }
    
    SECTION("Void parameter") {
        ContractParameter param = ContractParameter::voidParam();
        
        REQUIRE(param.getType() == ContractParameterType::VOID);
    }
    
    SECTION("Signature parameter") {
        Bytes signature(64, 0xAB);
        ContractParameter param = ContractParameter::signature(signature);
        
        REQUIRE(param.getType() == ContractParameterType::SIGNATURE);
        REQUIRE(param.getByteArray() == signature);
        REQUIRE(param.getByteArray().size() == 64);
    }
    
    SECTION("Type conversion and validation") {
        ContractParameter intParam(int64_t(42));
        
        // Should throw when accessing wrong type
        REQUIRE_THROWS(intParam.getString());
        REQUIRE_THROWS(intParam.getBoolean());
        REQUIRE_THROWS(intParam.getHash160());
        REQUIRE_THROWS(intParam.getHash256());
        REQUIRE_THROWS(intParam.getArray());
        REQUIRE_THROWS(intParam.getMap());
        
        // Should work for correct type
        REQUIRE_NOTHROW(intParam.getInteger());
    }
    
    SECTION("Nested arrays") {
        std::vector<ContractParameter> innerArray1;
        innerArray1.push_back(ContractParameter(int64_t(1)));
        innerArray1.push_back(ContractParameter(int64_t(2)));
        
        std::vector<ContractParameter> innerArray2;
        innerArray2.push_back(ContractParameter("a"));
        innerArray2.push_back(ContractParameter("b"));
        
        std::vector<ContractParameter> outerArray;
        outerArray.push_back(ContractParameter(innerArray1));
        outerArray.push_back(ContractParameter(innerArray2));
        
        ContractParameter param(outerArray);
        
        REQUIRE(param.getType() == ContractParameterType::ARRAY);
        REQUIRE(param.getArray().size() == 2);
        REQUIRE(param.getArray()[0].getType() == ContractParameterType::ARRAY);
        REQUIRE(param.getArray()[0].getArray().size() == 2);
        REQUIRE(param.getArray()[0].getArray()[0].getInteger() == 1);
        REQUIRE(param.getArray()[1].getArray()[1].getString() == "b");
    }
    
    SECTION("Complex map with various types") {
        std::map<ContractParameter, ContractParameter> complexMap;
        
        // Different key and value types
        complexMap[ContractParameter("string_key")] = ContractParameter(int64_t(123));
        complexMap[ContractParameter(int64_t(456))] = ContractParameter("string_value");
        complexMap[ContractParameter(true)] = ContractParameter(Bytes{0x01, 0x02});
        
        ContractParameter param(complexMap);
        
        REQUIRE(param.getType() == ContractParameterType::MAP);
        REQUIRE(param.getMap().size() == 3);
    }
    
    SECTION("Parameter equality") {
        ContractParameter param1(int64_t(42));
        ContractParameter param2(int64_t(42));
        ContractParameter param3(int64_t(43));
        
        REQUIRE(param1 == param2);
        REQUIRE(param1 != param3);
        
        ContractParameter str1("test");
        ContractParameter str2("test");
        ContractParameter str3("different");
        
        REQUIRE(str1 == str2);
        REQUIRE(str1 != str3);
        
        // Different types should not be equal
        REQUIRE(param1 != str1);
    }
}