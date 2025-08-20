#include "neocpp/protocol/neo_rpc_client.hpp"
#include "neocpp/protocol/http_service.hpp"
#include "neocpp/protocol/response_types_impl.hpp"
#include "neocpp/transaction/transaction.hpp"
#include "neocpp/transaction/signer.hpp"
#include "neocpp/serialization/binary_writer.hpp"
#include "neocpp/utils/hex.hpp"
#include "neocpp/utils/base64.hpp"
#include "neocpp/types/contract_parameter.hpp"
#include "neocpp/exceptions.hpp"
#include <sstream>

namespace neocpp {

NeoRpcClient::NeoRpcClient(const std::string& url) 
    : url_(url), requestId_(1) {
    httpService_ = std::make_shared<HttpService>(url);
}

// Helper method to create JSON-RPC request
static nlohmann::json createRequest(const std::string& method, const nlohmann::json& params, int id) {
    return nlohmann::json{
        {"jsonrpc", "2.0"},
        {"method", method},
        {"params", params},
        {"id", id}
    };
}

// Helper method to handle response
static nlohmann::json handleResponse(const nlohmann::json& response) {
    if (response.contains("error")) {
        std::string message = response["error"]["message"];
        throw RpcException("RPC error: " + message);
    }
    
    if (!response.contains("result")) {
        throw RpcException("Invalid RPC response: missing result");
    }
    
    return response["result"];
}

SharedPtr<NeoGetVersionResponse> NeoRpcClient::getVersion() {
    auto request = createRequest("getversion", nlohmann::json::array(), requestId_++);
    auto response = httpService_->post(request);
    auto result = handleResponse(response);
    
    auto versionResponse = std::make_shared<NeoGetVersionResponse>();
    versionResponse->parseJson(result);
    return versionResponse;
}

int NeoRpcClient::getConnectionCount() {
    auto request = createRequest("getconnectioncount", nlohmann::json::array(), requestId_++);
    auto response = httpService_->post(request);
    auto result = handleResponse(response);
    return result.get<int>();
}

SharedPtr<NeoGetPeersResponse> NeoRpcClient::getPeers() {
    auto request = createRequest("getpeers", nlohmann::json::array(), requestId_++);
    auto response = httpService_->post(request);
    auto result = handleResponse(response);
    
    auto peersResponse = std::make_shared<NeoGetPeersResponse>();
    peersResponse->parseJson(result);
    return peersResponse;
}

nlohmann::json NeoRpcClient::validateAddress(const std::string& address) {
    auto request = createRequest("validateaddress", nlohmann::json::array({address}), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

Hash256 NeoRpcClient::getBestBlockHash() {
    auto request = createRequest("getbestblockhash", nlohmann::json::array(), requestId_++);
    auto response = httpService_->post(request);
    auto result = handleResponse(response);
    return Hash256::fromHexString(result.get<std::string>());
}

SharedPtr<NeoGetBlockResponse> NeoRpcClient::getBlock(const Hash256& hash, bool verbose) {
    auto params = nlohmann::json::array({hash.toString(), verbose});
    auto request = createRequest("getblock", params, requestId_++);
    auto response = httpService_->post(request);
    auto result = handleResponse(response);
    
    auto blockResponse = std::make_shared<NeoGetBlockResponse>();
    blockResponse->parseJson(result);
    return blockResponse;
}

SharedPtr<NeoGetBlockResponse> NeoRpcClient::getBlock(uint32_t index, bool verbose) {
    auto params = nlohmann::json::array({index, verbose});
    auto request = createRequest("getblock", params, requestId_++);
    auto response = httpService_->post(request);
    auto result = handleResponse(response);
    
    auto blockResponse = std::make_shared<NeoGetBlockResponse>();
    blockResponse->parseJson(result);
    return blockResponse;
}

uint32_t NeoRpcClient::getBlockCount() {
    auto request = createRequest("getblockcount", nlohmann::json::array(), requestId_++);
    auto response = httpService_->post(request);
    auto result = handleResponse(response);
    return result.get<uint32_t>();
}

Hash256 NeoRpcClient::getBlockHash(uint32_t index) {
    auto request = createRequest("getblockhash", nlohmann::json::array({index}), requestId_++);
    auto response = httpService_->post(request);
    auto result = handleResponse(response);
    return Hash256::fromHexString(result.get<std::string>());
}

nlohmann::json NeoRpcClient::getBlockHeader(const Hash256& hash, bool verbose) {
    auto params = nlohmann::json::array({hash.toString(), verbose});
    auto request = createRequest("getblockheader", params, requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::getBlockHeader(uint32_t index, bool verbose) {
    auto params = nlohmann::json::array({index, verbose});
    auto request = createRequest("getblockheader", params, requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::getCommittee() {
    auto request = createRequest("getcommittee", nlohmann::json::array(), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

SharedPtr<NeoGetContractStateResponse> NeoRpcClient::getContractState(const Hash160& hash) {
    auto request = createRequest("getcontractstate", nlohmann::json::array({hash.toString()}), requestId_++);
    auto response = httpService_->post(request);
    auto result = handleResponse(response);
    
    auto contractResponse = std::make_shared<NeoGetContractStateResponse>();
    contractResponse->parseJson(result);
    return contractResponse;
}

nlohmann::json NeoRpcClient::getNativeContracts() {
    auto request = createRequest("getnativecontracts", nlohmann::json::array(), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::getNextBlockValidators() {
    auto request = createRequest("getnextblockvalidators", nlohmann::json::array(), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

SharedPtr<NeoGetRawTransactionResponse> NeoRpcClient::getRawTransaction(const Hash256& hash, bool verbose) {
    auto params = nlohmann::json::array({hash.toString(), verbose});
    auto request = createRequest("getrawtransaction", params, requestId_++);
    auto response = httpService_->post(request);
    auto result = handleResponse(response);
    
    auto txResponse = std::make_shared<NeoGetRawTransactionResponse>();
    txResponse->parseJson(result);
    return txResponse;
}

SharedPtr<NeoGetApplicationLogResponse> NeoRpcClient::getApplicationLog(const Hash256& hash) {
    auto request = createRequest("getapplicationlog", nlohmann::json::array({hash.toString()}), requestId_++);
    auto response = httpService_->post(request);
    auto result = handleResponse(response);
    
    auto logResponse = std::make_shared<NeoGetApplicationLogResponse>();
    logResponse->parseJson(result);
    return logResponse;
}

nlohmann::json NeoRpcClient::getStorage(const Hash160& scriptHash, const std::string& key) {
    Bytes keyBytes = Hex::decode(key);
    std::string base64Key = Base64::encode(keyBytes);
    auto params = nlohmann::json::array({scriptHash.toString(), base64Key});
    auto request = createRequest("getstorage", params, requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::getTransactionHeight(const Hash256& hash) {
    auto request = createRequest("gettransactionheight", nlohmann::json::array({hash.toString()}), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

SharedPtr<NeoGetUnclaimedGasResponse> NeoRpcClient::getUnclaimedGas(const std::string& address) {
    auto request = createRequest("getunclaimedgas", nlohmann::json::array({address}), requestId_++);
    auto response = httpService_->post(request);
    auto result = handleResponse(response);
    
    auto gasResponse = std::make_shared<NeoGetUnclaimedGasResponse>();
    gasResponse->parseJson(result);
    return gasResponse;
}

SharedPtr<NeoGetNep17BalancesResponse> NeoRpcClient::getNep17Balances(const std::string& address) {
    auto request = createRequest("getnep17balances", nlohmann::json::array({address}), requestId_++);
    auto response = httpService_->post(request);
    auto result = handleResponse(response);
    
    auto balancesResponse = std::make_shared<NeoGetNep17BalancesResponse>();
    balancesResponse->parseJson(result);
    return balancesResponse;
}

nlohmann::json NeoRpcClient::getNep17Transfers(const std::string& address, uint64_t startTime, uint64_t endTime) {
    auto params = nlohmann::json::array({address, startTime, endTime});
    auto request = createRequest("getnep17transfers", params, requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::getNep11Balances(const std::string& address) {
    auto request = createRequest("getnep11balances", nlohmann::json::array({address}), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::getNep11Transfers(const std::string& address, uint64_t startTime, uint64_t endTime) {
    auto params = nlohmann::json::array({address, startTime, endTime});
    auto request = createRequest("getnep11transfers", params, requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::getNep11Properties(const Hash160& contractHash, const std::string& tokenId) {
    auto params = nlohmann::json::array({contractHash.toString(), tokenId});
    auto request = createRequest("getnep11properties", params, requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

SharedPtr<NeoInvokeResultResponse> NeoRpcClient::invokeFunction(const Hash160& scriptHash, 
                                                                const std::string& operation,
                                                                const std::vector<ContractParameter>& params,
                                                                const std::vector<SharedPtr<Signer>>& signers) {
    nlohmann::json paramsArray = nlohmann::json::array();
    for (const auto& param : params) {
        paramsArray.push_back(param.toJson());
    }
    
    nlohmann::json signersArray = nlohmann::json::array();
    for (const auto& signer : signers) {
        signersArray.push_back(signer->toJson());
    }
    
    auto requestParams = nlohmann::json::array({
        scriptHash.toString(),
        operation,
        paramsArray,
        signersArray
    });
    
    auto request = createRequest("invokefunction", requestParams, requestId_++);
    auto response = httpService_->post(request);
    auto result = handleResponse(response);
    
    auto invokeResponse = std::make_shared<NeoInvokeResultResponse>();
    invokeResponse->parseJson(result);
    return invokeResponse;
}

SharedPtr<NeoInvokeResultResponse> NeoRpcClient::invokeScript(const Bytes& script,
                                                              const std::vector<SharedPtr<Signer>>& signers) {
    std::string base64Script = Base64::encode(script);
    
    nlohmann::json signersArray = nlohmann::json::array();
    for (const auto& signer : signers) {
        signersArray.push_back(signer->toJson());
    }
    
    auto params = nlohmann::json::array({base64Script, signersArray});
    auto request = createRequest("invokescript", params, requestId_++);
    auto response = httpService_->post(request);
    auto result = handleResponse(response);
    
    auto invokeResponse = std::make_shared<NeoInvokeResultResponse>();
    invokeResponse->parseJson(result);
    return invokeResponse;
}

SharedPtr<NeoInvokeResultResponse> NeoRpcClient::invokeContractVerify(const Hash160& scriptHash,
                                                                      const std::vector<ContractParameter>& params,
                                                                      const std::vector<SharedPtr<Signer>>& signers) {
    nlohmann::json paramsArray = nlohmann::json::array();
    for (const auto& param : params) {
        paramsArray.push_back(param.toJson());
    }
    
    nlohmann::json signersArray = nlohmann::json::array();
    for (const auto& signer : signers) {
        signersArray.push_back(signer->toJson());
    }
    
    auto requestParams = nlohmann::json::array({
        scriptHash.toString(),
        paramsArray,
        signersArray
    });
    
    auto request = createRequest("invokecontractverify", requestParams, requestId_++);
    auto response = httpService_->post(request);
    auto result = handleResponse(response);
    
    auto invokeResponse = std::make_shared<NeoInvokeResultResponse>();
    invokeResponse->parseJson(result);
    return invokeResponse;
}

nlohmann::json NeoRpcClient::traverseIterator(const std::string& sessionId, const std::string& iteratorId, int count) {
    auto params = nlohmann::json::array({sessionId, iteratorId, count});
    auto request = createRequest("traverseiterator", params, requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::terminateSession(const std::string& sessionId) {
    auto request = createRequest("terminatesession", nlohmann::json::array({sessionId}), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

Hash256 NeoRpcClient::sendRawTransaction(const SharedPtr<Transaction>& transaction) {
    BinaryWriter writer;
    transaction->serialize(writer);
    Bytes rawTx = writer.toArray();
    std::string base64Tx = Base64::encode(rawTx);
    
    auto request = createRequest("sendrawtransaction", nlohmann::json::array({base64Tx}), requestId_++);
    auto response = httpService_->post(request);
    auto result = handleResponse(response);
    
    return Hash256::fromHexString(result["hash"].get<std::string>());
}

nlohmann::json NeoRpcClient::submitBlock(const std::string& blockHex) {
    auto request = createRequest("submitblock", nlohmann::json::array({blockHex}), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

SharedPtr<NeoGetWalletBalanceResponse> NeoRpcClient::getWalletBalance(const Hash160& assetId) {
    auto request = createRequest("getwalletbalance", nlohmann::json::array({assetId.toString()}), requestId_++);
    auto response = httpService_->post(request);
    auto result = handleResponse(response);
    
    auto balanceResponse = std::make_shared<NeoGetWalletBalanceResponse>();
    balanceResponse->parseJson(result);
    return balanceResponse;
}

nlohmann::json NeoRpcClient::getWalletUnspent(const Hash160& assetId) {
    auto request = createRequest("getwalletunspent", nlohmann::json::array({assetId.toString()}), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::dumpPrivKey(const std::string& address) {
    auto request = createRequest("dumpprivkey", nlohmann::json::array({address}), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::getNewAddress() {
    auto request = createRequest("getnewaddress", nlohmann::json::array(), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::importPrivKey(const std::string& wif) {
    auto request = createRequest("importprivkey", nlohmann::json::array({wif}), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::listAddress() {
    auto request = createRequest("listaddress", nlohmann::json::array(), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::openWallet(const std::string& path, const std::string& password) {
    auto params = nlohmann::json::array({path, password});
    auto request = createRequest("openwallet", params, requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::closeWallet() {
    auto request = createRequest("closewallet", nlohmann::json::array(), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::sendFrom(const Hash160& assetId, const std::string& from, 
                                      const std::string& to, const std::string& amount) {
    auto params = nlohmann::json::array({assetId.toString(), from, to, amount});
    auto request = createRequest("sendfrom", params, requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::sendMany(const nlohmann::json& transfers) {
    auto request = createRequest("sendmany", nlohmann::json::array({transfers}), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::sendToAddress(const Hash160& assetId, const std::string& address, 
                                           const std::string& amount) {
    auto params = nlohmann::json::array({assetId.toString(), address, amount});
    auto request = createRequest("sendtoaddress", params, requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::calculateNetworkFee(const Bytes& transaction) {
    std::string base64Tx = Base64::encode(transaction);
    auto request = createRequest("calculatenetworkfee", nlohmann::json::array({base64Tx}), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::getMemPool(bool shouldGetUnverified) {
    auto request = createRequest("getrawmempool", nlohmann::json::array({shouldGetUnverified}), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::getStateHeight() {
    auto request = createRequest("getstateheight", nlohmann::json::array(), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::getStateRoot(uint32_t index) {
    auto request = createRequest("getstateroot", nlohmann::json::array({index}), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::getProof(const Hash256& rootHash, const Hash160& contractHash, const std::string& key) {
    Bytes keyBytes = Hex::decode(key);
    std::string base64Key = Base64::encode(keyBytes);
    auto params = nlohmann::json::array({rootHash.toString(), contractHash.toString(), base64Key});
    auto request = createRequest("getproof", params, requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::verifyProof(const Hash256& rootHash, const std::string& proofData) {
    auto params = nlohmann::json::array({rootHash.toString(), proofData});
    auto request = createRequest("verifyproof", params, requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::listPlugins() {
    auto request = createRequest("listplugins", nlohmann::json::array(), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

nlohmann::json NeoRpcClient::getSettings() {
    auto request = createRequest("getsettings", nlohmann::json::array(), requestId_++);
    auto response = httpService_->post(request);
    return handleResponse(response);
}

} // namespace neocpp