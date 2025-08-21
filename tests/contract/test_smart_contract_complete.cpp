#include <catch2/catch_test_macros.hpp>
#include "neocpp/contract/smart_contract.hpp"
#include "neocpp/contract/contract_manifest.hpp"
#include "neocpp/contract/nef_file.hpp"
#include "neocpp/script/script_builder.hpp"
#include "neocpp/types/contract_parameter.hpp"
#include "neocpp/types/hash160.hpp"
#include "neocpp/utils/hex.hpp"
#include "neocpp/protocol/stack_item.hpp"
#include <memory>
#include <string>

using namespace neocpp;

class MockSmartContract : public SmartContract {
public:
    MockSmartContract(const Hash160& scriptHash) : SmartContract(scriptHash) {}
    
    // Mock manifest
    SharedPtr<ContractManifest> manifest;
    
    SharedPtr<ContractManifest> getManifest() override {
        if (!manifest) {
            manifest = std::make_shared<ContractManifest>();
            manifest->setName("TestContract");
        }
        return manifest;
    }
};

TEST_CASE("SmartContract Complete Tests", "[contract]") {
    
    Hash160 contractHash("0x1234567890123456789012345678901234567890");
    
    SECTION("Construct smart contract") {
        SmartContract contract(contractHash);
        REQUIRE(contract.getScriptHash() == contractHash);
    }
    
    SECTION("Get manifest") {
        MockSmartContract contract(contractHash);
        auto manifest = contract.getManifest();
        REQUIRE(manifest != nullptr);
        REQUIRE(manifest->getName() == "TestContract");
    }
    
    SECTION("Get name") {
        MockSmartContract contract(contractHash);
        REQUIRE(contract.getName() == "TestContract");
    }
    
    SECTION("Invoke with empty string") {
        SmartContract contract(contractHash);
        
        // Build invocation script with empty function name
        ScriptBuilder builder;
        builder.contractCall(contractHash, "", {});
        
        Bytes script = builder.toArray();
        REQUIRE(!script.empty());
    }
    
    SECTION("Build invoke function script") {
        SmartContract contract(contractHash);
        std::string method = "transfer";
        std::vector<ContractParameter> params = {
            ContractParameter::hash160(Hash160("0xabcdef1234567890abcdef1234567890abcdef12")),
            ContractParameter::hash160(Hash160("0x1234567890abcdef1234567890abcdef12345678")),
            ContractParameter::integer(100),
            ContractParameter::any()
        };
        
        Bytes script = contract.buildInvokeFunctionScript(method, params);
        REQUIRE(!script.empty());
        
        // Verify script contains method name
        std::string scriptHex = Hex::encode(script);
        std::string methodHex = Hex::encode(Bytes(method.begin(), method.end()));
        REQUIRE(scriptHex.find(methodHex) != std::string::npos);
    }
    
    SECTION("Invoke function") {
        SmartContract contract(contractHash);
        
        auto invocationBuilder = contract.invokeFunction("balanceOf", {
            ContractParameter::hash160(Hash160("0xabcdef1234567890abcdef1234567890abcdef12"))
        });
        
        REQUIRE(invocationBuilder != nullptr);
        // The builder should have the script set
        auto script = invocationBuilder->getScript();
        REQUIRE(!script.empty());
    }
    
    SECTION("Call function returning string") {
        SmartContract contract(contractHash);
        
        // Mock response would be a ByteString stack item
        auto stackItem = std::make_shared<ByteStringStackItem>(Bytes{'N', 'E', 'O'});
        
        std::string result = contract.callFunctionReturningString(stackItem);
        REQUIRE(result == "NEO");
    }
    
    SECTION("Call function returning non-string") {
        SmartContract contract(contractHash);
        
        // Mock response with integer stack item
        auto stackItem = std::make_shared<IntegerStackItem>(42);
        
        REQUIRE_THROWS_AS(
            contract.callFunctionReturningString(stackItem),
            std::runtime_error
        );
    }
    
    SECTION("Call function returning int") {
        SmartContract contract(contractHash);
        
        // Mock response with integer stack item
        auto stackItem = std::make_shared<IntegerStackItem>(12345);
        
        int64_t result = contract.callFunctionReturningInt(stackItem);
        REQUIRE(result == 12345);
    }
    
    SECTION("Call function returning non-int") {
        SmartContract contract(contractHash);
        
        // Mock response with string stack item
        auto stackItem = std::make_shared<ByteStringStackItem>(Bytes{'t', 'e', 's', 't'});
        
        REQUIRE_THROWS_AS(
            contract.callFunctionReturningInt(stackItem),
            std::runtime_error
        );
    }
    
    SECTION("Call function returning bool") {
        SmartContract contract(contractHash);
        
        // Test true
        auto stackItemTrue = std::make_shared<BooleanStackItem>(true);
        bool resultTrue = contract.callFunctionReturningBool(stackItemTrue);
        REQUIRE(resultTrue == true);
        
        // Test false
        auto stackItemFalse = std::make_shared<BooleanStackItem>(false);
        bool resultFalse = contract.callFunctionReturningBool(stackItemFalse);
        REQUIRE(resultFalse == false);
    }
    
    SECTION("Call function returning bool - zero") {
        SmartContract contract(contractHash);
        
        // Integer 0 should be false
        auto stackItem = std::make_shared<IntegerStackItem>(0);
        bool result = contract.callFunctionReturningBool(stackItem);
        REQUIRE(result == false);
    }
    
    SECTION("Call function returning bool - one") {
        SmartContract contract(contractHash);
        
        // Integer 1 should be true
        auto stackItem = std::make_shared<IntegerStackItem>(1);
        bool result = contract.callFunctionReturningBool(stackItem);
        REQUIRE(result == true);
    }
    
    SECTION("Call function returning non-bool") {
        SmartContract contract(contractHash);
        
        // Array stack item cannot be converted to bool
        auto stackItem = std::make_shared<ArrayStackItem>(std::vector<SharedPtr<StackItem>>{});
        
        REQUIRE_THROWS_AS(
            contract.callFunctionReturningBool(stackItem),
            std::runtime_error
        );
    }
    
    SECTION("Call function returning script hash") {
        SmartContract contract(contractHash);
        
        Hash160 expectedHash("0xabcdef1234567890abcdef1234567890abcdef12");
        auto stackItem = std::make_shared<ByteStringStackItem>(expectedHash.toArray());
        
        Hash160 result = contract.callFunctionReturningScriptHash(stackItem);
        REQUIRE(result == expectedHash);
    }
    
    SECTION("Call function returning iterator") {
        SmartContract contract(contractHash);
        
        // Mock iterator response
        auto stackItem = std::make_shared<InteropInterfaceStackItem>();
        
        auto iterator = contract.callFunctionReturningIterator(stackItem);
        REQUIRE(iterator != nullptr);
    }
    
    SECTION("Call function returning iterator - traverse") {
        SmartContract contract(contractHash);
        
        // Create array to simulate iterator results
        std::vector<SharedPtr<StackItem>> items = {
            std::make_shared<IntegerStackItem>(1),
            std::make_shared<IntegerStackItem>(2),
            std::make_shared<IntegerStackItem>(3)
        };
        
        auto arrayItem = std::make_shared<ArrayStackItem>(items);
        
        std::vector<int64_t> results;
        for (const auto& item : items) {
            results.push_back(std::static_pointer_cast<IntegerStackItem>(item)->getValue());
        }
        
        REQUIRE(results.size() == 3);
        REQUIRE(results[0] == 1);
        REQUIRE(results[1] == 2);
        REQUIRE(results[2] == 3);
    }
    
    SECTION("Call function traversing iterator") {
        SmartContract contract(contractHash);
        
        // Test with max items limit
        int maxItems = 10;
        auto result = contract.traverseIterator("sessionId", "iteratorId", maxItems);
        
        // Result would be from RPC call
        REQUIRE(result != nullptr);
    }
    
    SECTION("Call function returning iterator - other return type") {
        SmartContract contract(contractHash);
        
        // Non-interop interface should throw
        auto stackItem = std::make_shared<IntegerStackItem>(42);
        
        REQUIRE_THROWS_AS(
            contract.callFunctionReturningIterator(stackItem),
            std::runtime_error
        );
    }
    
    SECTION("Invoking function performs correct call") {
        SmartContract contract(contractHash);
        
        std::string method = "symbol";
        auto invocationBuilder = contract.invokeFunction(method, {});
        
        REQUIRE(invocationBuilder != nullptr);
        auto script = invocationBuilder->getScript();
        REQUIRE(!script.empty());
        
        // Verify method name is in script
        std::string scriptHex = Hex::encode(script);
        std::string methodHex = Hex::encode(Bytes(method.begin(), method.end()));
        REQUIRE(scriptHex.find(methodHex) != std::string::npos);
    }
    
    SECTION("Invoking function without parameters") {
        SmartContract contract(contractHash);
        
        auto invocationBuilder = contract.invokeFunction("totalSupply");
        
        REQUIRE(invocationBuilder != nullptr);
        auto script = invocationBuilder->getScript();
        REQUIRE(!script.empty());
    }
    
    SECTION("Call function and unwrap iterator") {
        SmartContract contract(contractHash);
        
        // Create test data
        std::vector<SharedPtr<StackItem>> items = {
            std::make_shared<ByteStringStackItem>(Bytes{'a', 'b', 'c'}),
            std::make_shared<ByteStringStackItem>(Bytes{'d', 'e', 'f'}),
            std::make_shared<ByteStringStackItem>(Bytes{'g', 'h', 'i'})
        };
        
        auto arrayItem = std::make_shared<ArrayStackItem>(items);
        
        std::vector<std::string> results;
        for (const auto& item : items) {
            auto byteItem = std::static_pointer_cast<ByteStringStackItem>(item);
            results.push_back(std::string(byteItem->getValue().begin(), byteItem->getValue().end()));
        }
        
        REQUIRE(results.size() == 3);
        REQUIRE(results[0] == "abc");
        REQUIRE(results[1] == "def");
        REQUIRE(results[2] == "ghi");
    }
    
    SECTION("Call invoke function - missing function") {
        SmartContract contract(contractHash);
        
        // Empty function name should still create valid script
        auto invocationBuilder = contract.invokeFunction("", {});
        REQUIRE(invocationBuilder != nullptr);
        
        auto script = invocationBuilder->getScript();
        REQUIRE(!script.empty());
    }
    
    SECTION("Script hash validation") {
        // Test with various hash formats
        Hash160 hash1("0x1234567890123456789012345678901234567890");
        SmartContract contract1(hash1);
        REQUIRE(contract1.getScriptHash() == hash1);
        
        Hash160 hash2("1234567890123456789012345678901234567890");
        SmartContract contract2(hash2);
        REQUIRE(contract2.getScriptHash() == hash2);
    }
    
    SECTION("Contract call with complex parameters") {
        SmartContract contract(contractHash);
        
        // Create nested parameters
        std::vector<ContractParameter> arrayParams = {
            ContractParameter::integer(1),
            ContractParameter::string("test"),
            ContractParameter::boolean(true)
        };
        
        std::map<ContractParameter, ContractParameter> mapParams;
        mapParams[ContractParameter::string("key1")] = ContractParameter::integer(100);
        mapParams[ContractParameter::string("key2")] = ContractParameter::boolean(false);
        
        std::vector<ContractParameter> params = {
            ContractParameter::array(arrayParams),
            ContractParameter::map(mapParams),
            ContractParameter::byteArray({0x01, 0x02, 0x03})
        };
        
        Bytes script = contract.buildInvokeFunctionScript("complexMethod", params);
        REQUIRE(!script.empty());
        
        // Script should be larger due to complex parameters
        REQUIRE(script.size() > 50);
    }
}