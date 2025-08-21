#include <catch2/catch_test_macros.hpp>
#include "neocpp/protocol/core/response.hpp"
#include "neocpp/types/hash160.hpp"
#include "neocpp/types/hash256.hpp"
#include "neocpp/utils/hex.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using namespace neocpp;
using json = nlohmann::json;

TEST_CASE("Response Complete Tests", "[protocol]") {
    
    SECTION("Create success response") {
        json result = {
            {"balance", 1000000},
            {"address", "NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj"}
        };
        
        Response response(result);
        
        REQUIRE(response.isSuccess());
        REQUIRE(!response.hasError());
        REQUIRE(response.getResult() == result);
        REQUIRE(response.getJsonRpc() == "2.0");
        REQUIRE(response.getError() == nullptr);
    }
    
    SECTION("Create error response") {
        Response::Error error(-32602, "Invalid params");
        Response response(error);
        
        REQUIRE(!response.isSuccess());
        REQUIRE(response.hasError());
        REQUIRE(response.getError() != nullptr);
        REQUIRE(response.getError()->code == -32602);
        REQUIRE(response.getError()->message == "Invalid params");
        REQUIRE(response.getResult().is_null());
    }
    
    SECTION("Create error response with data") {
        json errorData = {{"details", "Missing required parameter"}};
        Response::Error error(-32602, "Invalid params", errorData);
        Response response(error);
        
        REQUIRE(response.hasError());
        REQUIRE(response.getError()->data == errorData);
        REQUIRE(response.getError()->data["details"] == "Missing required parameter");
    }
    
    SECTION("Parse standard error codes") {
        struct ErrorTestCase {
            int code;
            std::string message;
            std::string description;
        };
        
        std::vector<ErrorTestCase> testCases = {
            {-32700, "Parse error", "Invalid JSON was received"},
            {-32600, "Invalid Request", "The JSON sent is not a valid Request object"},
            {-32601, "Method not found", "The method does not exist / is not available"},
            {-32602, "Invalid params", "Invalid method parameter(s)"},
            {-32603, "Internal error", "Internal JSON-RPC error"},
            {-32000, "Server error", "Generic server error"}
        };
        
        for (const auto& tc : testCases) {
            Response::Error error(tc.code, tc.message);
            Response response(error);
            
            REQUIRE(response.getError()->code == tc.code);
            REQUIRE(response.getError()->message == tc.message);
        }
    }
    
    SECTION("Convert response to JSON") {
        json result = {{"value", 42}};
        Response response(result);
        response.setId(1);
        
        json j = response.toJson();
        
        REQUIRE(j["jsonrpc"] == "2.0");
        REQUIRE(j["result"] == result);
        REQUIRE(j["id"] == 1);
        REQUIRE(!j.contains("error"));
    }
    
    SECTION("Convert error response to JSON") {
        Response::Error error(-32601, "Method not found");
        Response response(error);
        response.setId(2);
        
        json j = response.toJson();
        
        REQUIRE(j["jsonrpc"] == "2.0");
        REQUIRE(j["error"]["code"] == -32601);
        REQUIRE(j["error"]["message"] == "Method not found");
        REQUIRE(j["id"] == 2);
        REQUIRE(!j.contains("result"));
    }
    
    SECTION("Create response from JSON") {
        json j = {
            {"jsonrpc", "2.0"},
            {"result", {{"balance", 500000}}},
            {"id", 1}
        };
        
        auto response = Response::fromJson(j);
        
        REQUIRE(response != nullptr);
        REQUIRE(response->isSuccess());
        REQUIRE(response->getResult()["balance"] == 500000);
        REQUIRE(response->getId() == 1);
    }
    
    SECTION("Create error response from JSON") {
        json j = {
            {"jsonrpc", "2.0"},
            {"error", {
                {"code", -32700},
                {"message", "Parse error"}
            }},
            {"id", nullptr}
        };
        
        auto response = Response::fromJson(j);
        
        REQUIRE(response != nullptr);
        REQUIRE(response->hasError());
        REQUIRE(response->getError()->code == -32700);
        REQUIRE(response->getError()->message == "Parse error");
    }
    
    SECTION("Get result as specific type") {
        Response response(json(42));
        
        int value = response.getResultAs<int>();
        REQUIRE(value == 42);
        
        Response response2(json("test string"));
        std::string str = response2.getResultAs<std::string>();
        REQUIRE(str == "test string");
        
        Response response3(json(true));
        bool boolValue = response3.getResultAs<bool>();
        REQUIRE(boolValue == true);
    }
    
    SECTION("Get complex result types") {
        json result = {
            {"array", {1, 2, 3}},
            {"object", {{"key", "value"}}},
            {"number", 3.14}
        };
        
        Response response(result);
        
        REQUIRE(response.getResult()["array"].is_array());
        REQUIRE(response.getResult()["array"].size() == 3);
        REQUIRE(response.getResult()["object"]["key"] == "value");
        REQUIRE(response.getResult()["number"].get<double>() == Approx(3.14));
    }
    
    // Neo-specific response parsing tests
    
    SECTION("Parse getBlock response") {
        json blockResult = {
            {"hash", "0x1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef"},
            {"size", 1024},
            {"version", 0},
            {"previousblockhash", "0xabcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890"},
            {"merkleroot", "0x567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef1234"},
            {"time", 1609459200},
            {"index", 100},
            {"nonce", "1234567890abcdef"},
            {"nextconsensus", "NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj"},
            {"witnesses", json::array()},
            {"tx", json::array()},
            {"confirmations", 10}
        };
        
        Response response(blockResult);
        
        REQUIRE(response.getResult()["hash"] == "0x1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef");
        REQUIRE(response.getResult()["index"] == 100);
        REQUIRE(response.getResult()["confirmations"] == 10);
    }
    
    SECTION("Parse getTransaction response") {
        json txResult = {
            {"hash", "0xabcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890"},
            {"size", 256},
            {"version", 0},
            {"nonce", 123456},
            {"sender", "NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj"},
            {"sysfee", "100000"},
            {"netfee", "50000"},
            {"validuntilblock", 1000},
            {"signers", json::array()},
            {"attributes", json::array()},
            {"script", "51c56b6c766b00527ac46c766b51527ac46203006c766b51c3616c7566"},
            {"witnesses", json::array()},
            {"blockhash", "0x1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef"},
            {"confirmations", 5},
            {"blocktime", 1609459200}
        };
        
        Response response(txResult);
        
        REQUIRE(response.getResult()["hash"] == "0xabcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890");
        REQUIRE(response.getResult()["nonce"] == 123456);
        REQUIRE(response.getResult()["sysfee"] == "100000");
    }
    
    SECTION("Parse invokeFunction response") {
        json invokeResult = {
            {"script", "10c00c0b746f74616c537570706c790c14f563ea40bc283d4de45040ec385a30b2a07340ef41c51f"},
            {"state", "HALT"},
            {"gasconsumed", "2007570"},
            {"exception", nullptr},
            {"stack", {
                {
                    {"type", "Integer"},
                    {"value", "100000000"}
                }
            }}
        };
        
        Response response(invokeResult);
        
        REQUIRE(response.getResult()["state"] == "HALT");
        REQUIRE(response.getResult()["gasconsumed"] == "2007570");
        REQUIRE(response.getResult()["stack"][0]["type"] == "Integer");
        REQUIRE(response.getResult()["stack"][0]["value"] == "100000000");
    }
    
    SECTION("Parse invokeFunction with error state") {
        json invokeResult = {
            {"state", "FAULT"},
            {"gasconsumed", "1000000"},
            {"exception", "Execution failed"},
            {"stack", json::array()}
        };
        
        Response response(invokeResult);
        
        REQUIRE(response.getResult()["state"] == "FAULT");
        REQUIRE(response.getResult()["exception"] == "Execution failed");
        REQUIRE(response.getResult()["stack"].empty());
    }
    
    SECTION("Parse getNep17Balances response") {
        json balancesResult = {
            {"address", "NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj"},
            {"balance", {
                {
                    {"assethash", "0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5"},
                    {"name", "NEO"},
                    {"symbol", "NEO"},
                    {"decimals", "0"},
                    {"amount", "50"},
                    {"lastupdatedblock", 1000}
                },
                {
                    {"assethash", "0xd2a4cff31913016155e38e474a2c06d08be276cf"},
                    {"name", "GAS"},
                    {"symbol", "GAS"},
                    {"decimals", "8"},
                    {"amount", "10000000000"},
                    {"lastupdatedblock", 999}
                }
            }}
        };
        
        Response response(balancesResult);
        
        REQUIRE(response.getResult()["address"] == "NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj");
        REQUIRE(response.getResult()["balance"].size() == 2);
        REQUIRE(response.getResult()["balance"][0]["symbol"] == "NEO");
        REQUIRE(response.getResult()["balance"][1]["symbol"] == "GAS");
    }
    
    SECTION("Parse getContractState response") {
        json contractResult = {
            {"id", 1},
            {"updatecounter", 0},
            {"hash", "0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5"},
            {"nef", {
                {"magic", 860243278},
                {"compiler", "neo-core-v3.0"},
                {"source", "https://github.com/neo-project/neo"},
                {"tokens", json::array()},
                {"script", "base64script"},
                {"checksum", 1234567890}
            }},
            {"manifest", {
                {"name", "NeoToken"},
                {"groups", json::array()},
                {"features", json::object()},
                {"supportedstandards", {"NEP-17"}},
                {"abi", json::object()},
                {"permissions", json::array()},
                {"trusts", json::array()},
                {"extra", json::object()}
            }}
        };
        
        Response response(contractResult);
        
        REQUIRE(response.getResult()["hash"] == "0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5");
        REQUIRE(response.getResult()["manifest"]["name"] == "NeoToken");
        REQUIRE(response.getResult()["manifest"]["supportedstandards"][0] == "NEP-17");
    }
    
    SECTION("Parse getApplicationLog response") {
        json appLogResult = {
            {"txid", "0xabcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890"},
            {"executions", {
                {
                    {"trigger", "Application"},
                    {"vmstate", "HALT"},
                    {"gasconsumed", "2007570"},
                    {"stack", json::array()},
                    {"notifications", {
                        {
                            {"contract", "0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5"},
                            {"eventname", "Transfer"},
                            {"state", {
                                {"type", "Array"},
                                {"value", json::array()}
                            }}
                        }
                    }}
                }
            }}
        };
        
        Response response(appLogResult);
        
        REQUIRE(response.getResult()["txid"] == "0xabcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890");
        REQUIRE(response.getResult()["executions"][0]["vmstate"] == "HALT");
        REQUIRE(response.getResult()["executions"][0]["notifications"][0]["eventname"] == "Transfer");
    }
    
    SECTION("Parse getValidators response") {
        json validatorsResult = {
            {
                {"publickey", "03b209fd4f53a7170ea4444e0cb0a6bb6a53c2bd016926989cf85f9b0fba17a70c"},
                {"votes", "50000000"},
                {"active", true}
            },
            {
                {"publickey", "02b3622bf4017bdfe317c58aed5f4c753f206b7db896046fa7d774bbc4bf7f8dc2"},
                {"votes", "30000000"},
                {"active", true}
            }
        };
        
        Response response(validatorsResult);
        
        REQUIRE(response.getResult().size() == 2);
        REQUIRE(response.getResult()[0]["votes"] == "50000000");
        REQUIRE(response.getResult()[0]["active"] == true);
    }
    
    SECTION("Parse traverseIterator response") {
        json iteratorResult = {
            {
                {"type", "ByteString"},
                {"value", "746573742076616c756531"}
            },
            {
                {"type", "ByteString"},
                {"value", "746573742076616c756532"}
            },
            {
                {"type", "Integer"},
                {"value", "100"}
            }
        };
        
        Response response(iteratorResult);
        
        REQUIRE(response.getResult().size() == 3);
        REQUIRE(response.getResult()[0]["type"] == "ByteString");
        REQUIRE(response.getResult()[2]["type"] == "Integer");
        REQUIRE(response.getResult()[2]["value"] == "100");
    }
    
    SECTION("Parse sendRawTransaction response") {
        json sendResult = {
            {"hash", "0xabcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890"}
        };
        
        Response response(sendResult);
        
        REQUIRE(response.getResult()["hash"] == "0xabcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890");
    }
    
    SECTION("Parse boolean result") {
        Response response(json(true));
        REQUIRE(response.getResult() == true);
        REQUIRE(response.getResultAs<bool>() == true);
        
        Response response2(json(false));
        REQUIRE(response2.getResult() == false);
        REQUIRE(response2.getResultAs<bool>() == false);
    }
    
    SECTION("Parse string result") {
        Response response(json("NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj"));
        REQUIRE(response.getResult() == "NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj");
        REQUIRE(response.getResultAs<std::string>() == "NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj");
    }
    
    SECTION("Parse number result") {
        Response response(json(12345));
        REQUIRE(response.getResult() == 12345);
        REQUIRE(response.getResultAs<int>() == 12345);
        
        Response response2(json(3.14159));
        REQUIRE(response2.getResultAs<double>() == Approx(3.14159));
    }
    
    SECTION("Parse null result") {
        Response response(json(nullptr));
        REQUIRE(response.getResult().is_null());
    }
    
    SECTION("Parse array result") {
        json arrayResult = {1, 2, 3, 4, 5};
        Response response(arrayResult);
        
        REQUIRE(response.getResult().is_array());
        REQUIRE(response.getResult().size() == 5);
        REQUIRE(response.getResult()[0] == 1);
        REQUIRE(response.getResult()[4] == 5);
    }
    
    SECTION("Convert to string") {
        json result = {{"key", "value"}};
        Response response(result);
        response.setId(123);
        
        std::string str = response.toString();
        REQUIRE(!str.empty());
        REQUIRE(str.find("\"jsonrpc\":\"2.0\"") != std::string::npos);
        REQUIRE(str.find("\"id\":123") != std::string::npos);
    }
    
    SECTION("Error response without ID") {
        Response::Error error(-32700, "Parse error");
        Response response(error);
        // ID defaults to 0
        
        json j = response.toJson();
        REQUIRE(j["id"] == 0);
    }
    
    SECTION("Update response properties") {
        Response response;
        
        // Set result
        json newResult = {{"updated", true}};
        response.setResult(newResult);
        REQUIRE(response.getResult() == newResult);
        
        // Set error
        Response::Error error(-32000, "Server error");
        response.setError(error);
        REQUIRE(response.hasError());
        REQUIRE(response.getError()->code == -32000);
        
        // Set ID
        response.setId(999);
        REQUIRE(response.getId() == 999);
    }
    
    SECTION("Response with large nested structure") {
        json complexResult = {
            {"level1", {
                {"level2", {
                    {"level3", {
                        {"level4", {
                            {"value", "deeply nested"}
                        }}
                    }}
                }}
            }},
            {"array", json::array()},
            {"number", 42}
        };
        
        // Add large array
        for (int i = 0; i < 100; i++) {
            complexResult["array"].push_back(i);
        }
        
        Response response(complexResult);
        
        REQUIRE(response.getResult()["level1"]["level2"]["level3"]["level4"]["value"] == "deeply nested");
        REQUIRE(response.getResult()["array"].size() == 100);
    }
    
    SECTION("Response with batch results") {
        json batchResult = json::array();
        
        for (int i = 0; i < 5; i++) {
            batchResult.push_back({
                {"id", i},
                {"result", i * 10}
            });
        }
        
        Response response(batchResult);
        
        REQUIRE(response.getResult().is_array());
        REQUIRE(response.getResult().size() == 5);
        REQUIRE(response.getResult()[0]["result"] == 0);
        REQUIRE(response.getResult()[4]["result"] == 40);
    }
}