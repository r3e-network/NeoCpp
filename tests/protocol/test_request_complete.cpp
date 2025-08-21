#include <catch2/catch_test_macros.hpp>
#include "neocpp/protocol/core/request.hpp"
#include "neocpp/types/hash160.hpp"
#include "neocpp/types/hash256.hpp"
#include "neocpp/types/contract_parameter.hpp"
#include "neocpp/transaction/signer.hpp"
#include "neocpp/utils/hex.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

using namespace neocpp;
using json = nlohmann::json;

TEST_CASE("Request Complete Tests", "[protocol]") {
    
    SECTION("GetBestBlockHash") {
        Request request = Request::getBestBlockHash();
        
        REQUIRE(request.getMethod() == "getbestblockhash");
        REQUIRE(request.getParams().empty());
        
        json j = request.toJson();
        REQUIRE(j["method"] == "getbestblockhash");
        REQUIRE(j["params"] == json::array());
    }
    
    SECTION("GetBlockHash") {
        uint32_t blockIndex = 1000;
        Request request = Request::getBlockHash(blockIndex);
        
        REQUIRE(request.getMethod() == "getblockhash");
        REQUIRE(request.getParams().size() == 1);
        REQUIRE(request.getParams()[0] == blockIndex);
        
        json j = request.toJson();
        REQUIRE(j["params"][0] == blockIndex);
    }
    
    SECTION("GetBlock - Index") {
        uint32_t blockIndex = 500;
        Request request = Request::getBlock(blockIndex, true);
        
        REQUIRE(request.getMethod() == "getblock");
        REQUIRE(request.getParams().size() == 2);
        REQUIRE(request.getParams()[0] == blockIndex);
        REQUIRE(request.getParams()[1] == true);
    }
    
    SECTION("GetBlock - Index only header") {
        uint32_t blockIndex = 500;
        Request request = Request::getBlockHeader(blockIndex);
        
        REQUIRE(request.getMethod() == "getblockheader");
        REQUIRE(request.getParams().size() == 2);
        REQUIRE(request.getParams()[0] == blockIndex);
        REQUIRE(request.getParams()[1] == true); // verbose
    }
    
    SECTION("GetBlock - Hash") {
        Hash256 blockHash("0x1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef");
        Request request = Request::getBlock(blockHash.toString(), true);
        
        REQUIRE(request.getMethod() == "getblock");
        REQUIRE(request.getParams().size() == 2);
        REQUIRE(request.getParams()[0] == blockHash.toString());
        REQUIRE(request.getParams()[1] == true);
    }
    
    SECTION("GetRawBlock - Index") {
        uint32_t blockIndex = 100;
        Request request = Request::getRawBlock(blockIndex);
        
        REQUIRE(request.getMethod() == "getblock");
        REQUIRE(request.getParams().size() == 2);
        REQUIRE(request.getParams()[0] == blockIndex);
        REQUIRE(request.getParams()[1] == false); // not verbose
    }
    
    SECTION("GetBlockHeaderCount") {
        Request request = Request::getBlockHeaderCount();
        
        REQUIRE(request.getMethod() == "getblockheadercount");
        REQUIRE(request.getParams().empty());
    }
    
    SECTION("GetBlockCount") {
        Request request = Request::getBlockCount();
        
        REQUIRE(request.getMethod() == "getblockcount");
        REQUIRE(request.getParams().empty());
    }
    
    SECTION("GetNativeContracts") {
        Request request = Request::getNativeContracts();
        
        REQUIRE(request.getMethod() == "getnativecontracts");
        REQUIRE(request.getParams().empty());
    }
    
    SECTION("GetContractState") {
        Hash160 contractHash("0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5");
        Request request = Request::getContractState(contractHash.toString());
        
        REQUIRE(request.getMethod() == "getcontractstate");
        REQUIRE(request.getParams().size() == 1);
        REQUIRE(request.getParams()[0] == contractHash.toString());
    }
    
    SECTION("GetContractState - by name") {
        std::string contractName = "NeoToken";
        Request request = Request::getContractState(contractName);
        
        REQUIRE(request.getMethod() == "getcontractstate");
        REQUIRE(request.getParams().size() == 1);
        REQUIRE(request.getParams()[0] == contractName);
    }
    
    SECTION("GetMemPool") {
        Request request = Request::getMemPool();
        
        REQUIRE(request.getMethod() == "getrawmempool");
        REQUIRE(request.getParams().size() == 1);
        REQUIRE(request.getParams()[0] == true); // verbose
    }
    
    SECTION("GetRawMemPool") {
        Request request = Request::getRawMemPool();
        
        REQUIRE(request.getMethod() == "getrawmempool");
        REQUIRE(request.getParams().size() == 1);
        REQUIRE(request.getParams()[0] == false); // not verbose
    }
    
    SECTION("GetTransaction") {
        Hash256 txHash("0xabcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890");
        Request request = Request::getTransaction(txHash.toString());
        
        REQUIRE(request.getMethod() == "gettransaction");
        REQUIRE(request.getParams().size() == 2);
        REQUIRE(request.getParams()[0] == txHash.toString());
        REQUIRE(request.getParams()[1] == true); // verbose
    }
    
    SECTION("GetRawTransaction") {
        Hash256 txHash("0xabcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890");
        Request request = Request::getRawTransaction(txHash.toString());
        
        REQUIRE(request.getMethod() == "getrawtransaction");
        REQUIRE(request.getParams().size() == 2);
        REQUIRE(request.getParams()[0] == txHash.toString());
        REQUIRE(request.getParams()[1] == false); // not verbose
    }
    
    SECTION("GetStorage") {
        Hash160 contractHash("0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5");
        std::string key = "totalSupply";
        Request request = Request::getStorage(contractHash.toString(), key);
        
        REQUIRE(request.getMethod() == "getstorage");
        REQUIRE(request.getParams().size() == 2);
        REQUIRE(request.getParams()[0] == contractHash.toString());
        REQUIRE(request.getParams()[1] == key);
    }
    
    SECTION("GetTransactionHeight") {
        Hash256 txHash("0xabcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890");
        Request request = Request::getTransactionHeight(txHash.toString());
        
        REQUIRE(request.getMethod() == "gettransactionheight");
        REQUIRE(request.getParams().size() == 1);
        REQUIRE(request.getParams()[0] == txHash.toString());
    }
    
    SECTION("GetNextBlockValidators") {
        Request request = Request::getNextBlockValidators();
        
        REQUIRE(request.getMethod() == "getnextblockvalidators");
        REQUIRE(request.getParams().empty());
    }
    
    SECTION("GetCommittee") {
        Request request = Request::getCommittee();
        
        REQUIRE(request.getMethod() == "getcommittee");
        REQUIRE(request.getParams().empty());
    }
    
    SECTION("GetConnectionCount") {
        Request request = Request::getConnectionCount();
        
        REQUIRE(request.getMethod() == "getconnectioncount");
        REQUIRE(request.getParams().empty());
    }
    
    SECTION("GetPeers") {
        Request request = Request::getPeers();
        
        REQUIRE(request.getMethod() == "getpeers");
        REQUIRE(request.getParams().empty());
    }
    
    SECTION("GetVersion") {
        Request request = Request::getVersion();
        
        REQUIRE(request.getMethod() == "getversion");
        REQUIRE(request.getParams().empty());
    }
    
    SECTION("SendRawTransaction") {
        Bytes rawTx = {0x00, 0x01, 0x02, 0x03, 0x04};
        std::string base64Tx = "AAECAwQ="; // Base64 encoded
        Request request = Request::sendRawTransaction(base64Tx);
        
        REQUIRE(request.getMethod() == "sendrawtransaction");
        REQUIRE(request.getParams().size() == 1);
        REQUIRE(request.getParams()[0] == base64Tx);
    }
    
    SECTION("SubmitBlock") {
        std::string blockHex = "0001020304";
        Request request = Request::submitBlock(blockHex);
        
        REQUIRE(request.getMethod() == "submitblock");
        REQUIRE(request.getParams().size() == 1);
        REQUIRE(request.getParams()[0] == blockHex);
    }
    
    SECTION("InvokeFunction") {
        Hash160 contractHash("0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5");
        std::string method = "balanceOf";
        std::vector<ContractParameter> params = {
            ContractParameter::hash160(Hash160("0x1234567890123456789012345678901234567890"))
        };
        
        Request request = Request::invokeFunction(contractHash.toString(), method, params);
        
        REQUIRE(request.getMethod() == "invokefunction");
        REQUIRE(request.getParams().size() == 3);
        REQUIRE(request.getParams()[0] == contractHash.toString());
        REQUIRE(request.getParams()[1] == method);
        REQUIRE(request.getParams()[2].is_array());
    }
    
    SECTION("InvokeFunction - with signers") {
        Hash160 contractHash("0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5");
        std::string method = "transfer";
        std::vector<ContractParameter> params = {
            ContractParameter::hash160(Hash160("0x1234567890123456789012345678901234567890")),
            ContractParameter::hash160(Hash160("0xabcdef1234567890abcdef1234567890abcdef12")),
            ContractParameter::integer(100)
        };
        
        auto signer = Signer::calledByEntry(Hash160("0x1234567890123456789012345678901234567890"));
        std::vector<SharedPtr<Signer>> signers = {signer};
        
        Request request = Request::invokeFunctionWithSigners(contractHash.toString(), method, params, signers);
        
        REQUIRE(request.getMethod() == "invokefunction");
        REQUIRE(request.getParams().size() == 4);
        REQUIRE(request.getParams()[3].is_array()); // signers array
    }
    
    SECTION("InvokeFunctionDiagnostics") {
        Hash160 contractHash("0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5");
        std::string method = "symbol";
        
        Request request = Request::invokeFunctionDiagnostics(contractHash.toString(), method, {});
        
        REQUIRE(request.getMethod() == "invokefunction");
        REQUIRE(request.getParams().size() == 4);
        REQUIRE(request.getParams()[3] == true); // diagnostics flag
    }
    
    SECTION("InvokeScript") {
        Bytes script = {0x51, 0x52, 0x53}; // PUSH1 PUSH2 PUSH3
        std::string base64Script = Base64::encode(script);
        
        Request request = Request::invokeScript(base64Script);
        
        REQUIRE(request.getMethod() == "invokescript");
        REQUIRE(request.getParams().size() == 1);
        REQUIRE(request.getParams()[0] == base64Script);
    }
    
    SECTION("InvokeScriptDiagnostics") {
        Bytes script = {0x51};
        std::string base64Script = Base64::encode(script);
        
        Request request = Request::invokeScriptDiagnostics(base64Script);
        
        REQUIRE(request.getMethod() == "invokescript");
        REQUIRE(request.getParams().size() == 2);
        REQUIRE(request.getParams()[0] == base64Script);
        REQUIRE(request.getParams()[1] == true); // diagnostics
    }
    
    SECTION("InvokeScriptWithSigner") {
        Bytes script = {0x51};
        std::string base64Script = Base64::encode(script);
        auto signer = Signer::global(Hash160("0x1234567890123456789012345678901234567890"));
        
        Request request = Request::invokeScriptWithSigner(base64Script, {signer});
        
        REQUIRE(request.getMethod() == "invokescript");
        REQUIRE(request.getParams().size() == 2);
        REQUIRE(request.getParams()[0] == base64Script);
        REQUIRE(request.getParams()[1].is_array()); // signers
    }
    
    SECTION("TraverseIterator") {
        std::string sessionId = "session123";
        std::string iteratorId = "iterator456";
        uint32_t count = 10;
        
        Request request = Request::traverseIterator(sessionId, iteratorId, count);
        
        REQUIRE(request.getMethod() == "traverseiterator");
        REQUIRE(request.getParams().size() == 3);
        REQUIRE(request.getParams()[0] == sessionId);
        REQUIRE(request.getParams()[1] == iteratorId);
        REQUIRE(request.getParams()[2] == count);
    }
    
    SECTION("TerminateSession") {
        std::string sessionId = "session123";
        
        Request request = Request::terminateSession(sessionId);
        
        REQUIRE(request.getMethod() == "terminatesession");
        REQUIRE(request.getParams().size() == 1);
        REQUIRE(request.getParams()[0] == sessionId);
    }
    
    SECTION("InvokeContractVerify") {
        Hash160 contractHash("0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5");
        std::vector<ContractParameter> params = {
            ContractParameter::boolean(true)
        };
        auto signer = Signer::none(Hash160("0x1234567890123456789012345678901234567890"));
        
        Request request = Request::invokeContractVerify(contractHash.toString(), params, {signer});
        
        REQUIRE(request.getMethod() == "invokecontractverify");
        REQUIRE(request.getParams().size() == 3);
        REQUIRE(request.getParams()[0] == contractHash.toString());
        REQUIRE(request.getParams()[1].is_array()); // params
        REQUIRE(request.getParams()[2].is_array()); // signers
    }
    
    SECTION("ListPlugins") {
        Request request = Request::listPlugins();
        
        REQUIRE(request.getMethod() == "listplugins");
        REQUIRE(request.getParams().empty());
    }
    
    SECTION("ValidateAddress") {
        std::string address = "NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj";
        Request request = Request::validateAddress(address);
        
        REQUIRE(request.getMethod() == "validateaddress");
        REQUIRE(request.getParams().size() == 1);
        REQUIRE(request.getParams()[0] == address);
    }
    
    SECTION("GetNep17Transfers") {
        std::string address = "NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj";
        Request request = Request::getNep17Transfers(address);
        
        REQUIRE(request.getMethod() == "getnep17transfers");
        REQUIRE(request.getParams().size() == 1);
        REQUIRE(request.getParams()[0] == address);
    }
    
    SECTION("GetNep17Transfers - with timestamps") {
        std::string address = "NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj";
        uint64_t from = 1000000;
        uint64_t to = 2000000;
        
        Request request = Request::getNep17Transfers(address, from, to);
        
        REQUIRE(request.getMethod() == "getnep17transfers");
        REQUIRE(request.getParams().size() == 3);
        REQUIRE(request.getParams()[0] == address);
        REQUIRE(request.getParams()[1] == from);
        REQUIRE(request.getParams()[2] == to);
    }
    
    SECTION("GetNep17Balances") {
        std::string address = "NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj";
        Request request = Request::getNep17Balances(address);
        
        REQUIRE(request.getMethod() == "getnep17balances");
        REQUIRE(request.getParams().size() == 1);
        REQUIRE(request.getParams()[0] == address);
    }
    
    SECTION("GetApplicationLog") {
        Hash256 txHash("0xabcdef1234567890abcdef1234567890abcdef1234567890abcdef1234567890");
        Request request = Request::getApplicationLog(txHash.toString());
        
        REQUIRE(request.getMethod() == "getapplicationlog");
        REQUIRE(request.getParams().size() == 1);
        REQUIRE(request.getParams()[0] == txHash.toString());
    }
    
    SECTION("GetStateRoot") {
        uint32_t blockIndex = 1000;
        Request request = Request::getStateRoot(blockIndex);
        
        REQUIRE(request.getMethod() == "getstateroot");
        REQUIRE(request.getParams().size() == 1);
        REQUIRE(request.getParams()[0] == blockIndex);
    }
    
    SECTION("GetProof") {
        Hash256 rootHash("0x1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef");
        Hash160 contractHash("0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5");
        std::string key = "balance";
        
        Request request = Request::getProof(rootHash.toString(), contractHash.toString(), key);
        
        REQUIRE(request.getMethod() == "getproof");
        REQUIRE(request.getParams().size() == 3);
        REQUIRE(request.getParams()[0] == rootHash.toString());
        REQUIRE(request.getParams()[1] == contractHash.toString());
        REQUIRE(request.getParams()[2] == key);
    }
    
    SECTION("VerifyProof") {
        Hash256 rootHash("0x1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef");
        std::string proofData = "proof_base64_data";
        
        Request request = Request::verifyProof(rootHash.toString(), proofData);
        
        REQUIRE(request.getMethod() == "verifyproof");
        REQUIRE(request.getParams().size() == 2);
        REQUIRE(request.getParams()[0] == rootHash.toString());
        REQUIRE(request.getParams()[1] == proofData);
    }
    
    SECTION("GetStateHeight") {
        Request request = Request::getStateHeight();
        
        REQUIRE(request.getMethod() == "getstateheight");
        REQUIRE(request.getParams().empty());
    }
    
    SECTION("GetState") {
        Hash256 rootHash("0x1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef");
        Hash160 contractHash("0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5");
        std::string key = "totalSupply";
        
        Request request = Request::getState(rootHash.toString(), contractHash.toString(), key);
        
        REQUIRE(request.getMethod() == "getstate");
        REQUIRE(request.getParams().size() == 3);
        REQUIRE(request.getParams()[0] == rootHash.toString());
        REQUIRE(request.getParams()[1] == contractHash.toString());
        REQUIRE(request.getParams()[2] == key);
    }
    
    SECTION("FindStates") {
        Hash256 rootHash("0x1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef");
        Hash160 contractHash("0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5");
        std::string prefix = "balance";
        
        Request request = Request::findStates(rootHash.toString(), contractHash.toString(), prefix);
        
        REQUIRE(request.getMethod() == "findstates");
        REQUIRE(request.getParams().size() == 3);
        REQUIRE(request.getParams()[0] == rootHash.toString());
        REQUIRE(request.getParams()[1] == contractHash.toString());
        REQUIRE(request.getParams()[2] == prefix);
    }
    
    SECTION("FindStates - with count") {
        Hash256 rootHash("0x1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef");
        Hash160 contractHash("0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5");
        std::string prefix = "balance";
        uint32_t count = 100;
        
        Request request = Request::findStates(rootHash.toString(), contractHash.toString(), prefix, "", count);
        
        REQUIRE(request.getMethod() == "findstates");
        REQUIRE(request.getParams().size() == 5);
        REQUIRE(request.getParams()[0] == rootHash.toString());
        REQUIRE(request.getParams()[1] == contractHash.toString());
        REQUIRE(request.getParams()[2] == prefix);
        REQUIRE(request.getParams()[3] == "");
        REQUIRE(request.getParams()[4] == count);
    }
    
    SECTION("GetNep11Balances") {
        std::string address = "NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj";
        Request request = Request::getNep11Balances(address);
        
        REQUIRE(request.getMethod() == "getnep11balances");
        REQUIRE(request.getParams().size() == 1);
        REQUIRE(request.getParams()[0] == address);
    }
    
    SECTION("GetNep11Transfers") {
        std::string address = "NZNos2WqTbu5oCgyfss9kUJgBXJqhuYAaj";
        Request request = Request::getNep11Transfers(address);
        
        REQUIRE(request.getMethod() == "getnep11transfers");
        REQUIRE(request.getParams().size() == 1);
        REQUIRE(request.getParams()[0] == address);
    }
    
    SECTION("GetNep11Properties") {
        Hash160 contractHash("0xef4073a0f2b305a38ec4050e4d3d28bc40ea63f5");
        std::string tokenId = "token123";
        
        Request request = Request::getNep11Properties(contractHash.toString(), tokenId);
        
        REQUIRE(request.getMethod() == "getnep11properties");
        REQUIRE(request.getParams().size() == 2);
        REQUIRE(request.getParams()[0] == contractHash.toString());
        REQUIRE(request.getParams()[1] == tokenId);
    }
}