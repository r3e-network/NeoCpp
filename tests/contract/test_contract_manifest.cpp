#include <catch2/catch_test_macros.hpp>
#include "neocpp/contract/contract_manifest.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using namespace neocpp;
using json = nlohmann::json;

TEST_CASE("ContractManifest Tests", "[contract]") {
    
    SECTION("Default constructor") {
        ContractManifest manifest;
        
        REQUIRE(manifest.getName().empty());
        REQUIRE(manifest.getGroups().empty());
        REQUIRE(manifest.getSupportedStandards().empty());
        REQUIRE(manifest.getAbi().is_null());
        REQUIRE(manifest.getPermissions().is_null());
        REQUIRE(manifest.getTrusts().is_null());
        REQUIRE(manifest.getExtra().is_null());
    }
    
    SECTION("Set and get name") {
        ContractManifest manifest;
        
        manifest.setName("TestContract");
        REQUIRE(manifest.getName() == "TestContract");
        
        manifest.setName("UpdatedContract");
        REQUIRE(manifest.getName() == "UpdatedContract");
    }
    
    SECTION("Add and get groups") {
        ContractManifest manifest;
        
        manifest.addGroup("group1");
        manifest.addGroup("group2");
        manifest.addGroup("group3");
        
        auto groups = manifest.getGroups();
        REQUIRE(groups.size() == 3);
        REQUIRE(groups[0] == "group1");
        REQUIRE(groups[1] == "group2");
        REQUIRE(groups[2] == "group3");
    }
    
    SECTION("Add supported standards") {
        ContractManifest manifest;
        
        manifest.addSupportedStandard("NEP-17");
        manifest.addSupportedStandard("NEP-11");
        
        auto standards = manifest.getSupportedStandards();
        REQUIRE(standards.size() == 2);
        REQUIRE(standards[0] == "NEP-17");
        REQUIRE(standards[1] == "NEP-11");
    }
    
    SECTION("Check supported standards") {
        ContractManifest manifest;
        
        manifest.addSupportedStandard("NEP-17");
        manifest.addSupportedStandard("NEP-11");
        
        REQUIRE(manifest.supportsStandard("NEP-17") == true);
        REQUIRE(manifest.supportsStandard("NEP-11") == true);
        REQUIRE(manifest.supportsStandard("NEP-5") == false);
        REQUIRE(manifest.supportsStandard("NEP-10") == false);
    }
    
    SECTION("Set and get ABI") {
        ContractManifest manifest;
        
        json abi = {
            {"methods", json::array({
                {
                    {"name", "transfer"},
                    {"parameters", json::array({
                        {{"name", "from"}, {"type", "Hash160"}},
                        {{"name", "to"}, {"type", "Hash160"}},
                        {{"name", "amount"}, {"type", "Integer"}},
                        {{"name", "data"}, {"type", "Any"}}
                    })},
                    {"returntype", "Boolean"},
                    {"offset", 0},
                    {"safe", true}
                },
                {
                    {"name", "balanceOf"},
                    {"parameters", json::array({
                        {{"name", "account"}, {"type", "Hash160"}}
                    })},
                    {"returntype", "Integer"},
                    {"offset", 100},
                    {"safe", true}
                }
            })},
            {"events", json::array({
                {
                    {"name", "Transfer"},
                    {"parameters", json::array({
                        {{"name", "from"}, {"type", "Hash160"}},
                        {{"name", "to"}, {"type", "Hash160"}},
                        {{"name", "amount"}, {"type", "Integer"}}
                    })}
                }
            })}
        };
        
        manifest.setAbi(abi);
        REQUIRE(manifest.getAbi() == abi);
        REQUIRE(manifest.getAbi()["methods"].size() == 2);
        REQUIRE(manifest.getAbi()["events"].size() == 1);
    }
    
    SECTION("Get methods from ABI") {
        ContractManifest manifest;
        
        json abi = {
            {"methods", json::array({
                {{"name", "method1"}, {"returntype", "Void"}},
                {{"name", "method2"}, {"returntype", "Boolean"}},
                {{"name", "method3"}, {"returntype", "Integer"}}
            })},
            {"events", json::array()}
        };
        
        manifest.setAbi(abi);
        auto methods = manifest.getMethods();
        
        REQUIRE(methods.size() == 3);
        REQUIRE(methods[0]["name"] == "method1");
        REQUIRE(methods[1]["name"] == "method2");
        REQUIRE(methods[2]["name"] == "method3");
    }
    
    SECTION("Get events from ABI") {
        ContractManifest manifest;
        
        json abi = {
            {"methods", json::array()},
            {"events", json::array({
                {{"name", "Event1"}, {"parameters", json::array()}},
                {{"name", "Event2"}, {"parameters", json::array()}}
            })}
        };
        
        manifest.setAbi(abi);
        auto events = manifest.getEvents();
        
        REQUIRE(events.size() == 2);
        REQUIRE(events[0]["name"] == "Event1");
        REQUIRE(events[1]["name"] == "Event2");
    }
    
    SECTION("Set and get permissions") {
        ContractManifest manifest;
        
        json permissions = json::array({
            {
                {"contract", "*"},
                {"methods", "*"}
            },
            {
                {"contract", "0x0102030405060708090a0b0c0d0e0f1011121314"},
                {"methods", json::array({"transfer", "balanceOf"})}
            }
        });
        
        manifest.setPermissions(permissions);
        REQUIRE(manifest.getPermissions() == permissions);
        REQUIRE(manifest.getPermissions().size() == 2);
    }
    
    SECTION("Set and get trusts") {
        ContractManifest manifest;
        
        json trusts = json::array({
            "*",
            "0x0102030405060708090a0b0c0d0e0f1011121314"
        });
        
        manifest.setTrusts(trusts);
        REQUIRE(manifest.getTrusts() == trusts);
        REQUIRE(manifest.getTrusts().size() == 2);
    }
    
    SECTION("Set and get extra data") {
        ContractManifest manifest;
        
        json extra = {
            {"author", "Test Author"},
            {"email", "test@example.com"},
            {"description", "Test contract"},
            {"version", "1.0.0"}
        };
        
        manifest.setExtra(extra);
        REQUIRE(manifest.getExtra() == extra);
        REQUIRE(manifest.getExtra()["author"] == "Test Author");
        REQUIRE(manifest.getExtra()["version"] == "1.0.0");
    }
    
    SECTION("Convert to JSON") {
        ContractManifest manifest;
        
        manifest.setName("TestContract");
        manifest.addSupportedStandard("NEP-17");
        manifest.addGroup("TestGroup");
        
        json abi = {
            {"methods", json::array()},
            {"events", json::array()}
        };
        manifest.setAbi(abi);
        
        json manifestJson = manifest.toJson();
        
        REQUIRE(manifestJson["name"] == "TestContract");
        REQUIRE(manifestJson["supportedstandards"].size() == 1);
        REQUIRE(manifestJson["supportedstandards"][0] == "NEP-17");
        REQUIRE(manifestJson["groups"].size() == 1);
        REQUIRE(manifestJson["groups"][0] == "TestGroup");
        REQUIRE(manifestJson["abi"] == abi);
    }
    
    SECTION("Parse from JSON") {
        json manifestJson = {
            {"name", "ParsedContract"},
            {"groups", json::array({"Group1", "Group2"})},
            {"supportedstandards", json::array({"NEP-17", "NEP-11"})},
            {"abi", {
                {"methods", json::array()},
                {"events", json::array()}
            }},
            {"permissions", json::array()},
            {"trusts", json::array()},
            {"extra", {{"key", "value"}}}
        };
        
        ContractManifest manifest = ContractManifest::fromJson(manifestJson);
        
        REQUIRE(manifest.getName() == "ParsedContract");
        REQUIRE(manifest.getGroups().size() == 2);
        REQUIRE(manifest.getSupportedStandards().size() == 2);
        REQUIRE(manifest.supportsStandard("NEP-17") == true);
        REQUIRE(manifest.supportsStandard("NEP-11") == true);
        REQUIRE(manifest.getExtra()["key"] == "value");
    }
    
    SECTION("Constructor from JSON") {
        json manifestJson = {
            {"name", "ConstructedContract"},
            {"groups", json::array()},
            {"supportedstandards", json::array({"NEP-17"})},
            {"abi", {
                {"methods", json::array({{"name", "test"}})},
                {"events", json::array()}
            }},
            {"permissions", json::array()},
            {"trusts", json::array()},
            {"extra", json::object()}
        };
        
        ContractManifest manifest(manifestJson);
        
        REQUIRE(manifest.getName() == "ConstructedContract");
        REQUIRE(manifest.supportsStandard("NEP-17") == true);
        REQUIRE(manifest.getMethods().size() == 1);
        REQUIRE(manifest.getMethods()[0]["name"] == "test");
    }
    
    SECTION("Empty methods and events") {
        ContractManifest manifest;
        
        // Test with no ABI set
        auto methods = manifest.getMethods();
        auto events = manifest.getEvents();
        
        REQUIRE(methods.empty());
        REQUIRE(events.empty());
        
        // Test with empty ABI
        json emptyAbi = {
            {"methods", json::array()},
            {"events", json::array()}
        };
        manifest.setAbi(emptyAbi);
        
        methods = manifest.getMethods();
        events = manifest.getEvents();
        
        REQUIRE(methods.empty());
        REQUIRE(events.empty());
    }
    
    SECTION("Complex manifest roundtrip") {
        ContractManifest original;
        
        original.setName("ComplexContract");
        original.addSupportedStandard("NEP-17");
        original.addSupportedStandard("NEP-11");
        original.addGroup("GroupA");
        original.addGroup("GroupB");
        
        json abi = {
            {"methods", json::array({
                {{"name", "method1"}, {"returntype", "Boolean"}},
                {{"name", "method2"}, {"returntype", "Integer"}}
            })},
            {"events", json::array({
                {{"name", "Event1"}}
            })}
        };
        original.setAbi(abi);
        
        json permissions = json::array({{"contract", "*"}, {"methods", "*"}});
        original.setPermissions(permissions);
        
        json trusts = json::array({"*"});
        original.setTrusts(trusts);
        
        json extra = {{"version", "1.0.0"}, {"author", "Test"}};
        original.setExtra(extra);
        
        // Convert to JSON and back
        json manifestJson = original.toJson();
        ContractManifest parsed = ContractManifest::fromJson(manifestJson);
        
        // Verify everything matches
        REQUIRE(parsed.getName() == original.getName());
        REQUIRE(parsed.getGroups() == original.getGroups());
        REQUIRE(parsed.getSupportedStandards() == original.getSupportedStandards());
        REQUIRE(parsed.getAbi() == original.getAbi());
        REQUIRE(parsed.getPermissions() == original.getPermissions());
        REQUIRE(parsed.getTrusts() == original.getTrusts());
        REQUIRE(parsed.getExtra() == original.getExtra());
    }
}