#include <catch2/catch_test_macros.hpp>
#include "../mock/mock_rpc_client.hpp"
#include "neocpp/protocol/core/request.hpp"
#include "neocpp/protocol/core/response.hpp"
#include "neocpp/types/hash160.hpp"
#include "neocpp/types/hash256.hpp"
#include <memory>
#include <string>

using namespace neocpp;
using namespace neocpp::test;
using json = nlohmann::json;

TEST_CASE("RPC Integration Tests", "[protocol][integration]") {
    
    auto mockClient = std::make_shared<MockRpcClient>();
    
    SECTION("Basic request-response flow") {
        Request request = Request::getBlockCount();
        auto response = mockClient->send(request);
        
        REQUIRE(response != nullptr);
        REQUIRE(response->isSuccess());
        REQUIRE(response->getResultAs<uint32_t>() == 1000);
    }
    
    SECTION("Method not found error") {
        Request request("nonexistent_method", {});
        auto response = mockClient->send(request);
        
        REQUIRE(response != nullptr);
        REQUIRE(response->hasError());
        REQUIRE(response->getError()->code == -32601);
        REQUIRE(response->getError()->message == "Method not found");
    }
    
    SECTION("Connection failure simulation") {
        mockClient->setConnected(false);
        
        Request request = Request::getBlockCount();
        auto response = mockClient->send(request);
        
        REQUIRE(response->hasError());
        REQUIRE(response->getError()->code == -32000);
        REQUIRE(response->getError()->message == "Connection failed");
        
        // Restore connection
        mockClient->setConnected(true);
        response = mockClient->send(request);
        REQUIRE(response->isSuccess());
    }
    
    SECTION("Error rate simulation") {
        mockClient->setErrorRate(50); // 50% error rate
        
        int successCount = 0;
        int errorCount = 0;
        
        for (int i = 0; i < 100; i++) {
            Request request = Request::getBlockCount();
            auto response = mockClient->send(request);
            
            if (response->isSuccess()) {
                successCount++;
            } else {
                errorCount++;
            }
        }
        
        // Should have roughly 50% errors
        REQUIRE(errorCount > 30);
        REQUIRE(errorCount < 70);
    }
    
    SECTION("Custom handler registration") {
        // Register custom handler for a method
        mockClient->registerHandler("custom_method", [](const Request& req) {
            json result = {
                {"custom", true},
                {"params_count", req.getParams().size()}
            };
            return std::make_shared<Response>(result);
        });
        
        Request request("custom_method", {1, 2, 3});
        auto response = mockClient->send(request);
        
        REQUIRE(response->isSuccess());
        REQUIRE(response->getResult()["custom"] == true);
        REQUIRE(response->getResult()["params_count"] == 3);
    }
    
    SECTION("Override default response") {
        // Override default getcontractstate response
        json customContract = {
            {"id", 100},
            {"hash", "0x1234567890123456789012345678901234567890"},
            {"manifest", {
                {"name", "CustomContract"}
            }}
        };
        
        mockClient->setDefaultResponse("getcontractstate", customContract);
        
        Request request = Request::getContractState("0x1234567890123456789012345678901234567890");
        auto response = mockClient->send(request);
        
        REQUIRE(response->isSuccess());
        REQUIRE(response->getResult()["id"] == 100);
        REQUIRE(response->getResult()["manifest"]["name"] == "CustomContract");
    }
    
    SECTION("Request counting") {
        mockClient->resetRequestCount();
        REQUIRE(mockClient->getRequestCount() == 0);
        
        for (int i = 0; i < 10; i++) {
            Request request = Request::getBlockCount();
            mockClient->send(request);
        }
        
        REQUIRE(mockClient->getRequestCount() == 10);
    }
    
    SECTION("Block count management") {
        mockClient->setBlockCount(5000);
        
        Request request = Request::getBlockCount();
        auto response = mockClient->send(request);
        
        REQUIRE(response->getResultAs<uint32_t>() == 5000);
    }
    
    SECTION("Complex request with parameters") {
        Hash160 contractHash("0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5");
        std::vector<ContractParameter> params = {
            ContractParameter::hash160(Hash160("0x1234567890123456789012345678901234567890")),
            ContractParameter::integer(100)
        };
        
        Request request = Request::invokeFunction(contractHash.toString(), "transfer", params);
        auto response = mockClient->send(request);
        
        REQUIRE(response->isSuccess());
        REQUIRE(response->getResult()["state"] == "HALT");
    }
    
    SECTION("Batch-like sequential requests") {
        std::vector<Request> requests = {
            Request::getBlockCount(),
            Request::getBestBlockHash(),
            Request::getVersion(),
            Request::getConnectionCount()
        };
        
        std::vector<SharedPtr<Response>> responses;
        
        for (const auto& req : requests) {
            responses.push_back(mockClient->send(req));
        }
        
        REQUIRE(responses.size() == 4);
        REQUIRE(responses[0]->isSuccess());
        REQUIRE(responses[1]->isSuccess());
        REQUIRE(responses[2]->isSuccess());
        REQUIRE(responses[3]->isSuccess());
        
        REQUIRE(responses[0]->getResultAs<uint32_t>() == 1000);
        REQUIRE(responses[1]->getResultAs<std::string>().length() == 66);
        REQUIRE(responses[2]->getResult()["protocol"]["network"] == 860833102);
        REQUIRE(responses[3]->getResultAs<int>() == 5);
    }
    
    SECTION("Address validation") {
        // Valid address
        Request validReq = Request::validateAddress("NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj");
        auto validResponse = mockClient->send(validReq);
        
        REQUIRE(validResponse->isSuccess());
        REQUIRE(validResponse->getResult()["isvalid"] == true);
        
        // Invalid address
        Request invalidReq = Request::validateAddress("InvalidAddress");
        auto invalidResponse = mockClient->send(invalidReq);
        
        REQUIRE(invalidResponse->isSuccess());
        REQUIRE(invalidResponse->getResult()["isvalid"] == false);
    }
    
    SECTION("Simulating invoke function with stack result") {
        // Register handler that returns stack items
        mockClient->registerHandler("invokefunction", [](const Request& req) {
            json result = {
                {"script", ""},
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
            
            // Check if it's a balanceOf call
            if (req.getParams().size() >= 2 && req.getParams()[1] == "balanceOf") {
                result["stack"][0]["value"] = "50000000";
            }
            
            return std::make_shared<Response>(result);
        });
        
        Hash160 contractHash("0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5");
        Request request = Request::invokeFunction(contractHash.toString(), "balanceOf", {
            ContractParameter::hash160(Hash160("0x1234567890123456789012345678901234567890"))
        });
        
        auto response = mockClient->send(request);
        
        REQUIRE(response->isSuccess());
        REQUIRE(response->getResult()["state"] == "HALT");
        REQUIRE(response->getResult()["stack"][0]["value"] == "50000000");
    }
    
    SECTION("Transaction sending simulation") {
        // Register handler for sendrawtransaction
        mockClient->registerHandler("sendrawtransaction", [](const Request& req) {
            if (req.getParams().empty()) {
                return std::make_shared<Response>(
                    Response::Error(-32602, "Invalid params")
                );
            }
            
            // Generate mock transaction hash
            json result = {
                {"hash", "0xabcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890"}
            };
            return std::make_shared<Response>(result);
        });
        
        Request request = Request::sendRawTransaction("base64encodedtransaction");
        auto response = mockClient->send(request);
        
        REQUIRE(response->isSuccess());
        REQUIRE(response->getResult()["hash"].get<std::string>().length() == 66);
    }
    
    SECTION("Iterator traversal simulation") {
        // Register handler for traverseiterator
        mockClient->registerHandler("traverseiterator", [](const Request& req) {
            if (req.getParams().size() < 3) {
                return std::make_shared<Response>(
                    Response::Error(-32602, "Invalid params")
                );
            }
            
            uint32_t count = req.getParams()[2].get<uint32_t>();
            json items = json::array();
            
            for (uint32_t i = 0; i < count && i < 10; i++) {
                items.push_back({
                    {"type", "ByteString"},
                    {"value", "item" + std::to_string(i)}
                });
            }
            
            return std::make_shared<Response>(items);
        });
        
        Request request = Request::traverseIterator("session123", "iterator456", 5);
        auto response = mockClient->send(request);
        
        REQUIRE(response->isSuccess());
        REQUIRE(response->getResult().size() == 5);
        REQUIRE(response->getResult()[0]["type"] == "ByteString");
    }
    
    SECTION("NEP-17 transfers simulation") {
        // Register handler for getnep17transfers
        mockClient->registerHandler("getnep17transfers", [](const Request& req) {
            if (req.getParams().empty()) {
                return std::make_shared<Response>(
                    Response::Error(-32602, "Invalid params")
                );
            }
            
            json result = {
                {"sent", json::array()},
                {"received", {
                    {
                        {"timestamp", 1609459200000},
                        {"assethash", "0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5"},
                        {"transferaddress", "NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj"},
                        {"amount", "100"},
                        {"blockindex", 500},
                        {"transfernotifyindex", 0},
                        {"txhash", "0x1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef"}
                    }
                }},
                {"address", req.getParams()[0]}
            };
            
            return std::make_shared<Response>(result);
        });
        
        Request request = Request::getNep17Transfers("NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj");
        auto response = mockClient->send(request);
        
        REQUIRE(response->isSuccess());
        REQUIRE(response->getResult()["received"].size() == 1);
        REQUIRE(response->getResult()["received"][0]["amount"] == "100");
    }
}