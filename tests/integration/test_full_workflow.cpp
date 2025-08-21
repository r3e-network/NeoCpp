#include <catch2/catch_test_macros.hpp>
#include "../mock/mock_rpc_client.hpp"
#include "neocpp/wallet/wallet.hpp"
#include "neocpp/wallet/account.hpp"
#include "neocpp/transaction/transaction_builder.hpp"
#include "neocpp/contract/neo_token.hpp"
#include "neocpp/contract/gas_token.hpp"
#include "neocpp/script/script_builder.hpp"
#include "neocpp/crypto/ec_key_pair.hpp"
#include "neocpp/types/hash160.hpp"
#include "neocpp/types/hash256.hpp"
#include "neocpp/utils/hex.hpp"
#include <memory>
#include <string>

using namespace neocpp;
using namespace neocpp::test;

TEST_CASE("Full Workflow Integration Tests", "[integration]") {
    
    auto mockClient = std::make_shared<MockRpcClient>();
    
    SECTION("Complete NEP-17 token transfer workflow") {
        // 1. Create wallet and accounts
        auto wallet = Wallet::create("TestWallet");
        auto sender = wallet->createAccount();
        auto receiver = Account::create();
        
        REQUIRE(wallet->getAccounts().size() == 1);
        REQUIRE(sender != nullptr);
        REQUIRE(receiver != nullptr);
        
        // 2. Set up mock balances
        mockClient->registerHandler("invokefunction", [sender](const Request& req) {
            json result = {
                {"state", "HALT"},
                {"gasconsumed", "2007570"},
                {"exception", nullptr},
                {"stack", json::array()}
            };
            
            // Check if it's a balanceOf call
            if (req.getParams().size() >= 2 && req.getParams()[1] == "balanceOf") {
                result["stack"] = {
                    {
                        {"type", "Integer"},
                        {"value", "100000000"} // 100 NEO
                    }
                };
            } else if (req.getParams()[1] == "transfer") {
                result["stack"] = {
                    {
                        {"type", "Boolean"},
                        {"value", true}
                    }
                };
            }
            
            return std::make_shared<Response>(result);
        });
        
        // 3. Check balance
        auto neoToken = std::make_shared<NeoToken>();
        auto balance = neoToken->balanceOf(sender);
        REQUIRE(balance > 0);
        
        // 4. Build transfer transaction
        TransactionBuilder builder(mockClient);
        
        int64_t amount = 10; // 10 NEO
        ScriptBuilder scriptBuilder;
        scriptBuilder.contractCall(neoToken->getScriptHash(), "transfer", {
            ContractParameter::hash160(sender->getScriptHash()),
            ContractParameter::hash160(receiver->getScriptHash()),
            ContractParameter::integer(amount),
            ContractParameter::any()
        });
        
        builder.script(scriptBuilder.toArray())
               .signers({AccountSigner::calledByEntry(sender)});
        
        auto tx = builder.build();
        REQUIRE(tx != nullptr);
        REQUIRE(!tx->getScript().empty());
        
        // 5. Sign transaction
        tx->sign(sender);
        REQUIRE(tx->getWitnesses().size() == 1);
        
        // 6. Send transaction
        mockClient->registerHandler("sendrawtransaction", [](const Request&) {
            json result = {
                {"hash", "0xabcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890"}
            };
            return std::make_shared<Response>(result);
        });
        
        Bytes rawTx = tx->toArray();
        std::string base64Tx = Base64::encode(rawTx);
        Request sendRequest = Request::sendRawTransaction(base64Tx);
        auto sendResponse = mockClient->send(sendRequest);
        
        REQUIRE(sendResponse->isSuccess());
        REQUIRE(sendResponse->getResult()["hash"].get<std::string>().length() == 66);
    }
    
    SECTION("Multi-signature account workflow") {
        // 1. Create multiple key pairs
        auto keyPair1 = ECKeyPair::create();
        auto keyPair2 = ECKeyPair::create();
        auto keyPair3 = ECKeyPair::create();
        
        std::vector<SharedPtr<ECPublicKey>> publicKeys = {
            keyPair1->getPublicKey(),
            keyPair2->getPublicKey(),
            keyPair3->getPublicKey()
        };
        
        // 2. Create multi-sig account (2 of 3)
        uint8_t threshold = 2;
        auto multiSigAccount = Account::fromMultiSig(threshold, publicKeys);
        
        REQUIRE(multiSigAccount != nullptr);
        REQUIRE(multiSigAccount->isMultiSig());
        
        // 3. Build transaction requiring multi-sig
        TransactionBuilder builder(mockClient);
        
        builder.script({0x51}) // PUSH1
               .signers({AccountSigner::calledByEntry(multiSigAccount)});
        
        auto tx = builder.build();
        REQUIRE(tx != nullptr);
        
        // 4. Sign with multiple keys
        // In real scenario, this would be done by different parties
        tx->addMultiSigWitness(multiSigAccount, keyPair1);
        tx->addMultiSigWitness(multiSigAccount, keyPair2);
        
        // Should have enough signatures now
        REQUIRE(tx->getWitnesses().size() >= 1);
    }
    
    SECTION("Smart contract deployment workflow") {
        // 1. Create deployment account
        auto deployAccount = Account::create();
        
        // 2. Prepare contract script and manifest
        Bytes contractScript = {0x56, 0x57, 0x58}; // Example bytecode
        std::string manifestJson = R"({
            "name": "TestContract",
            "groups": [],
            "features": {},
            "supportedstandards": [],
            "abi": {
                "methods": [],
                "events": []
            },
            "permissions": [],
            "trusts": [],
            "extra": {}
        })";
        
        // 3. Build deployment transaction
        TransactionBuilder builder(mockClient);
        ScriptBuilder scriptBuilder;
        
        // ContractManagement.deploy method
        Hash160 contractManagement("0xfffdc93764dbaddd97c48f252a53ea4643faa3fd");
        scriptBuilder.contractCall(contractManagement, "deploy", {
            ContractParameter::byteArray(contractScript),
            ContractParameter::string(manifestJson)
        });
        
        builder.script(scriptBuilder.toArray())
               .signers({AccountSigner::calledByEntry(deployAccount)})
               .additionalSystemFee(1000000000); // Extra GAS for deployment
        
        auto tx = builder.build();
        REQUIRE(tx != nullptr);
        REQUIRE(tx->getSystemFee() > 0);
    }
    
    SECTION("NEO voting workflow") {
        // 1. Create voter account
        auto voter = Account::create();
        
        // 2. Create candidate
        auto candidate = Account::create();
        auto candidatePublicKey = candidate->getKeyPair()->getPublicKey();
        
        // 3. Register as candidate
        auto neoToken = std::make_shared<NeoToken>();
        auto registerBuilder = neoToken->registerCandidate(candidatePublicKey);
        
        REQUIRE(registerBuilder != nullptr);
        auto registerScript = registerBuilder->getScript();
        REQUIRE(!registerScript.empty());
        
        // 4. Vote for candidate
        auto voteBuilder = neoToken->vote(voter, candidatePublicKey);
        
        REQUIRE(voteBuilder != nullptr);
        auto voteScript = voteBuilder->getScript();
        REQUIRE(!voteScript.empty());
        
        // 5. Check vote status
        mockClient->registerHandler("invokefunction", [](const Request& req) {
            json result = {
                {"state", "HALT"},
                {"gasconsumed", "2007570"},
                {"exception", nullptr}
            };
            
            if (req.getParams()[1] == "getAccountState") {
                result["stack"] = {
                    {
                        {"type", "Struct"},
                        {"value", {
                            {{"type", "Integer"}, {"value", "100"}}, // balance
                            {{"type", "Integer"}, {"value", "1000"}}, // balance height
                            {{"type", "ByteString"}, {"value", "03b209fd4f53a7170ea4444e0cb0a6bb6a53c2bd016926989cf85f9b0fba17a70c"}} // voted to
                        }}
                    }
                };
            }
            
            return std::make_shared<Response>(result);
        });
        
        auto accountState = neoToken->getAccountState(voter);
        REQUIRE(accountState != nullptr);
        if (accountState->voteTo) {
            REQUIRE(!accountState->voteTo->getEncoded().empty());
        }
    }
    
    SECTION("Oracle request workflow") {
        // 1. Create requester account
        auto requester = Account::create();
        
        // 2. Build oracle request
        TransactionBuilder builder(mockClient);
        ScriptBuilder scriptBuilder;
        
        Hash160 oracleContract("0x46e3b6aa5e2e2d7ea6451e26c3e7a020ea23f3f2");
        std::string url = "https://api.example.com/data";
        std::string filter = "$.result";
        std::string callback = "processOracleResponse";
        
        scriptBuilder.contractCall(oracleContract, "request", {
            ContractParameter::string(url),
            ContractParameter::string(filter),
            ContractParameter::string(callback),
            ContractParameter::any(), // user data
            ContractParameter::integer(10000000) // gas for callback
        });
        
        builder.script(scriptBuilder.toArray())
               .signers({AccountSigner::calledByEntry(requester)})
               .attribute(TransactionAttribute::oracleResponse(0)); // oracle request ID
        
        auto tx = builder.build();
        REQUIRE(tx != nullptr);
        REQUIRE(tx->getAttributes().size() > 0);
    }
    
    SECTION("Iterator traversal workflow") {
        // 1. Invoke function returning iterator
        mockClient->registerHandler("invokefunction", [](const Request& req) {
            json result = {
                {"state", "HALT"},
                {"gasconsumed", "2007570"},
                {"exception", nullptr},
                {"stack", {
                    {
                        {"type", "InteropInterface"},
                        {"iterator", "iterator_id_123"},
                        {"session", "session_456"}
                    }
                }}
            };
            return std::make_shared<Response>(result);
        });
        
        Hash160 contractHash("0x1234567890123456789012345678901234567890");
        Request invokeRequest = Request::invokeFunction(contractHash.toString(), "getAllItems", {});
        auto invokeResponse = mockClient->send(invokeRequest);
        
        REQUIRE(invokeResponse->isSuccess());
        
        // 2. Traverse iterator
        mockClient->registerHandler("traverseiterator", [](const Request&) {
            json items = {
                {{"type", "ByteString"}, {"value", "item1"}},
                {{"type", "ByteString"}, {"value", "item2"}},
                {{"type", "ByteString"}, {"value", "item3"}}
            };
            return std::make_shared<Response>(items);
        });
        
        Request traverseRequest = Request::traverseIterator("session_456", "iterator_id_123", 10);
        auto traverseResponse = mockClient->send(traverseRequest);
        
        REQUIRE(traverseResponse->isSuccess());
        REQUIRE(traverseResponse->getResult().size() == 3);
        
        // 3. Terminate session
        mockClient->registerHandler("terminatesession", [](const Request&) {
            return std::make_shared<Response>(json(true));
        });
        
        Request terminateRequest = Request::terminateSession("session_456");
        auto terminateResponse = mockClient->send(terminateRequest);
        
        REQUIRE(terminateResponse->isSuccess());
    }
    
    SECTION("Full token ecosystem workflow") {
        // 1. Create accounts
        auto treasury = Account::create();
        auto user1 = Account::create();
        auto user2 = Account::create();
        
        // 2. Deploy custom NEP-17 token
        TransactionBuilder deployBuilder(mockClient);
        
        // Token parameters
        std::string tokenName = "TestToken";
        std::string tokenSymbol = "TST";
        uint8_t decimals = 8;
        int64_t totalSupply = 1000000 * 100000000; // 1M tokens with 8 decimals
        
        // 3. Initial distribution
        auto gasToken = std::make_shared<GasToken>();
        
        // Treasury distributes to users
        auto dist1 = gasToken->transfer(treasury, user1->getScriptHash(), 100 * 100000000);
        auto dist2 = gasToken->transfer(treasury, user2->getScriptHash(), 50 * 100000000);
        
        REQUIRE(dist1 != nullptr);
        REQUIRE(dist2 != nullptr);
        
        // 4. User-to-user transfer
        auto userTransfer = gasToken->transfer(user1, user2->getScriptHash(), 25 * 100000000);
        REQUIRE(userTransfer != nullptr);
        
        // 5. Check final balances
        mockClient->registerHandler("getnep17balances", [](const Request& req) {
            std::string address = req.getParams()[0];
            json result = {
                {"address", address},
                {"balance", {
                    {
                        {"assethash", "0xd2a4cff31913016155e38e474a2c06d08be276cf"},
                        {"name", "GAS"},
                        {"symbol", "GAS"},
                        {"decimals", "8"},
                        {"amount", "7500000000"}, // 75 GAS
                        {"lastupdatedblock", 1000}
                    }
                }}
            };
            return std::make_shared<Response>(result);
        });
        
        Request balanceRequest = Request::getNep17Balances(user2->getAddress());
        auto balanceResponse = mockClient->send(balanceRequest);
        
        REQUIRE(balanceResponse->isSuccess());
        REQUIRE(balanceResponse->getResult()["balance"].size() > 0);
    }
}