#pragma once

#include <memory>
#include <string>
#include <map>
#include <functional>
#include <nlohmann/json.hpp>
#include "neocpp/protocol/core/request.hpp"
#include "neocpp/protocol/core/response.hpp"
#include "neocpp/types/hash160.hpp"
#include "neocpp/types/hash256.hpp"
#include "neocpp/transaction/transaction.hpp"

namespace neocpp {
namespace test {

/// Mock RPC client for testing
class MockRpcClient {
public:
    using RequestHandler = std::function<SharedPtr<Response>(const Request&)>;
    
private:
    std::map<std::string, RequestHandler> handlers_;
    std::map<std::string, json> defaultResponses_;
    uint32_t blockCount_ = 1000;
    bool connected_ = true;
    int requestCount_ = 0;
    int errorRate_ = 0; // Percentage of requests that should fail
    
public:
    MockRpcClient() {
        setupDefaultHandlers();
        setupDefaultResponses();
    }
    
    /// Send request and receive response
    SharedPtr<Response> send(const Request& request) {
        requestCount_++;
        
        // Simulate connection failure
        if (!connected_) {
            return std::make_shared<Response>(
                Response::Error(-32000, "Connection failed")
            );
        }
        
        // Simulate random errors based on error rate
        if (errorRate_ > 0 && (requestCount_ % 100) < errorRate_) {
            return std::make_shared<Response>(
                Response::Error(-32603, "Internal error")
            );
        }
        
        // Check for custom handler
        auto it = handlers_.find(request.getMethod());
        if (it != handlers_.end()) {
            return it->second(request);
        }
        
        // Check for default response
        auto defaultIt = defaultResponses_.find(request.getMethod());
        if (defaultIt != defaultResponses_.end()) {
            return std::make_shared<Response>(defaultIt->second);
        }
        
        // Method not found
        return std::make_shared<Response>(
            Response::Error(-32601, "Method not found")
        );
    }
    
    /// Register custom handler for method
    void registerHandler(const std::string& method, RequestHandler handler) {
        handlers_[method] = handler;
    }
    
    /// Set default response for method
    void setDefaultResponse(const std::string& method, const json& response) {
        defaultResponses_[method] = response;
    }
    
    /// Simulate connection state
    void setConnected(bool connected) {
        connected_ = connected;
    }
    
    /// Set error rate (0-100)
    void setErrorRate(int rate) {
        errorRate_ = std::max(0, std::min(100, rate));
    }
    
    /// Get request count
    int getRequestCount() const {
        return requestCount_;
    }
    
    /// Reset request count
    void resetRequestCount() {
        requestCount_ = 0;
    }
    
    /// Set block count
    void setBlockCount(uint32_t count) {
        blockCount_ = count;
    }
    
    /// Get block count
    uint32_t getBlockCount() const {
        return blockCount_;
    }
    
private:
    void setupDefaultHandlers() {
        // getblockcount handler
        handlers_["getblockcount"] = [this](const Request&) {
            return std::make_shared<Response>(json(blockCount_));
        };
        
        // getbestblockhash handler
        handlers_["getbestblockhash"] = [](const Request&) {
            return std::make_shared<Response>(
                json("0x1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef")
            );
        };
        
        // getversion handler
        handlers_["getversion"] = [](const Request&) {
            json version = {
                {"tcpport", 10333},
                {"wsport", 10334},
                {"nonce", 1234567890},
                {"useragent", "/Neo:3.0.0/"},
                {"protocol", {
                    {"addressversion", 53},
                    {"network", 860833102},
                    {"validatorscount", 7},
                    {"msperblock", 15000},
                    {"maxtraceableblocks", 2102400},
                    {"maxvaliduntilblockincrement", 86400},
                    {"maxtransactionsperblock", 512},
                    {"memorypoolmaxtransactions", 50000},
                    {"initialgasdistribution", 5200000000000000}
                }}
            };
            return std::make_shared<Response>(version);
        };
        
        // getconnectioncount handler
        handlers_["getconnectioncount"] = [](const Request&) {
            return std::make_shared<Response>(json(5));
        };
        
        // validateaddress handler
        handlers_["validateaddress"] = [](const Request& req) {
            if (req.getParams().empty()) {
                return std::make_shared<Response>(
                    Response::Error(-32602, "Invalid params")
                );
            }
            
            std::string address = req.getParams()[0];
            bool isValid = address.length() == 34 && address[0] == 'N';
            
            json result = {
                {"address", address},
                {"isvalid", isValid}
            };
            return std::make_shared<Response>(result);
        };
    }
    
    void setupDefaultResponses() {
        // Default NEO token response
        defaultResponses_["getcontractstate"] = {
            {"id", -5},
            {"updatecounter", 0},
            {"hash", "0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5"},
            {"nef", {
                {"magic", 860243278},
                {"compiler", "neo-core-v3.0"},
                {"source", ""},
                {"tokens", json::array()},
                {"script", ""},
                {"checksum", 0}
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
        
        // Default invoke response
        defaultResponses_["invokefunction"] = {
            {"script", ""},
            {"state", "HALT"},
            {"gasconsumed", "2007570"},
            {"exception", nullptr},
            {"stack", json::array()}
        };
        
        // Default block response
        defaultResponses_["getblock"] = {
            {"hash", "0x0000000000000000000000000000000000000000000000000000000000000000"},
            {"size", 1024},
            {"version", 0},
            {"previousblockhash", "0x0000000000000000000000000000000000000000000000000000000000000000"},
            {"merkleroot", "0x0000000000000000000000000000000000000000000000000000000000000000"},
            {"time", 1609459200},
            {"index", 0},
            {"nonce", "0000000000000000"},
            {"nextconsensus", "NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj"},
            {"witnesses", json::array()},
            {"tx", json::array()},
            {"confirmations", 1}
        };
        
        // Default transaction response
        defaultResponses_["getrawtransaction"] = {
            {"hash", "0x0000000000000000000000000000000000000000000000000000000000000000"},
            {"size", 256},
            {"version", 0},
            {"nonce", 0},
            {"sender", "NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj"},
            {"sysfee", "100000"},
            {"netfee", "50000"},
            {"validuntilblock", 1000},
            {"signers", json::array()},
            {"attributes", json::array()},
            {"script", ""},
            {"witnesses", json::array()}
        };
        
        // Default NEP-17 balances
        defaultResponses_["getnep17balances"] = {
            {"address", ""},
            {"balance", json::array()}
        };
        
        // Default committee
        defaultResponses_["getcommittee"] = json::array({
            "03b209fd4f53a7170ea4444e0cb0a6bb6a53c2bd016926989cf85f9b0fba17a70c",
            "02b3622bf4017bdfe317c58aed5f4c753f206b7db896046fa7d774bbc4bf7f8dc2"
        });
        
        // Default validators
        defaultResponses_["getnextblockvalidators"] = json::array();
    }
};

/// Mock transaction builder helper
class MockTransactionBuilder {
private:
    SharedPtr<MockRpcClient> client_;
    
public:
    explicit MockTransactionBuilder(SharedPtr<MockRpcClient> client) 
        : client_(client) {}
    
    SharedPtr<Transaction> buildTestTransaction() {
        auto tx = std::make_shared<Transaction>();
        tx->setVersion(0);
        tx->setNonce(123456);
        tx->setValidUntilBlock(client_->getBlockCount() + 100);
        tx->setSystemFee(100000);
        tx->setNetworkFee(50000);
        tx->setScript({0x51}); // PUSH1
        return tx;
    }
};

/// Mock stack item factory
class MockStackItemFactory {
public:
    static SharedPtr<StackItem> createInteger(int64_t value) {
        return std::make_shared<IntegerStackItem>(value);
    }
    
    static SharedPtr<StackItem> createBoolean(bool value) {
        return std::make_shared<BooleanStackItem>(value);
    }
    
    static SharedPtr<StackItem> createByteString(const Bytes& value) {
        return std::make_shared<ByteStringStackItem>(value);
    }
    
    static SharedPtr<StackItem> createArray(const std::vector<SharedPtr<StackItem>>& items) {
        return std::make_shared<ArrayStackItem>(items);
    }
    
    static SharedPtr<StackItem> createMap() {
        return std::make_shared<MapStackItem>();
    }
    
    static SharedPtr<StackItem> createStruct(const std::vector<SharedPtr<StackItem>>& items) {
        return std::make_shared<StructStackItem>(items);
    }
    
    static SharedPtr<StackItem> createInteropInterface() {
        return std::make_shared<InteropInterfaceStackItem>();
    }
};

} // namespace test
} // namespace neocpp