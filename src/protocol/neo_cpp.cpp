#include "neocpp/protocol/neo_cpp.hpp"
#include "neocpp/protocol/neo_rpc_client.hpp"
#include "neocpp/protocol/http_service.hpp"
#include "neocpp/transaction/transaction.hpp"
#include "neocpp/exceptions.hpp"

namespace neocpp {

NeoCpp::NeoCpp(const NeoCppConfig& config, SharedPtr<HttpService> httpService)
    : config_(config), httpService_(httpService) {
    if (!httpService) {
        throw IllegalArgumentException("HTTP service cannot be null");
    }
    // Extract base URL from HTTP service if needed
    rpcClient_ = std::make_shared<NeoRpcClient>(httpService->getUrl());
}

NeoCpp::NeoCpp(const std::string& url)
    : config_(NeoCppConfig()) {
    if (url.empty()) {
        throw IllegalArgumentException("URL cannot be empty");
    }
    httpService_ = std::make_shared<HttpService>(url);
    rpcClient_ = std::make_shared<NeoRpcClient>(url);
}

SharedPtr<NeoCpp> NeoCpp::build(SharedPtr<HttpService> httpService, const NeoCppConfig& config) {
    return std::make_shared<NeoCpp>(config, httpService);
}

SharedPtr<NeoCpp> NeoCpp::build(const std::string& url, const NeoCppConfig& config) {
    auto httpService = std::make_shared<HttpService>(url);
    return std::make_shared<NeoCpp>(config, httpService);
}

// Convenience methods that delegate to RPC client

uint32_t NeoCpp::getBlockCount() {
    return rpcClient_->getBlockCount();
}

Hash256 NeoCpp::getBestBlockHash() {
    return rpcClient_->getBestBlockHash();
}

SharedPtr<NeoGetBlockResponse> NeoCpp::getBlock(const Hash256& hash, bool verbose) {
    return rpcClient_->getBlock(hash, verbose);
}

SharedPtr<NeoGetBlockResponse> NeoCpp::getBlock(uint32_t index, bool verbose) {
    return rpcClient_->getBlock(index, verbose);
}

SharedPtr<NeoGetRawTransactionResponse> NeoCpp::getTransaction(const Hash256& txId, bool verbose) {
    return rpcClient_->getRawTransaction(txId, verbose);
}

SharedPtr<NeoGetContractStateResponse> NeoCpp::getContractState(const Hash160& scriptHash) {
    return rpcClient_->getContractState(scriptHash);
}

SharedPtr<NeoGetNep17BalancesResponse> NeoCpp::getNep17Balances(const std::string& address) {
    return rpcClient_->getNep17Balances(address);
}

SharedPtr<NeoInvokeResultResponse> NeoCpp::invokeFunction(const Hash160& scriptHash,
                                                          const std::string& method,
                                                          const nlohmann::json& params,
                                                          const nlohmann::json& signers) {
    return rpcClient_->invokeFunction(scriptHash, method, params, signers);
}

Hash256 NeoCpp::sendRawTransaction(const SharedPtr<Transaction>& transaction) {
    return rpcClient_->sendRawTransaction(transaction);
}

nlohmann::json NeoCpp::validateAddress(const std::string& address) {
    return rpcClient_->validateAddress(address);
}

SharedPtr<NeoGetVersionResponse> NeoCpp::getVersion() {
    return rpcClient_->getVersion();
}

} // namespace neocpp