#include "neocpp/protocol/core/neo.hpp"
#include "neocpp/protocol/neo_rpc_client.hpp"
#include "neocpp/protocol/http_service.hpp"
#include "neocpp/protocol/response_types.hpp"
#include "neocpp/protocol/core/polling/block_polling.hpp"
#include "neocpp/neo_constants.hpp"

namespace neocpp {

Neo::Neo() : Neo("http://localhost:10332") {
}

Neo::Neo(const std::string& rpcUrl) : rpcUrl_(rpcUrl) {
    initialize();
}

void Neo::initialize() {
    httpService_ = std::make_shared<HttpService>(rpcUrl_);
    rpcClient_ = std::make_shared<NeoRpcClient>(rpcUrl_);
}

void Neo::subscribeToBlocks(std::function<void(uint32_t)> callback) {
    if (!blockPolling_) {
        blockPolling_ = std::make_shared<BlockPolling>(rpcClient_);
    }
    blockPolling_->subscribe(callback);
    blockPolling_->start();
}

void Neo::unsubscribeFromBlocks() {
    if (blockPolling_) {
        blockPolling_->stop();
    }
}

bool Neo::isConnected() const {
    try {
        auto version = rpcClient_->getVersion();
        return version != nullptr;
    } catch (...) {
        return false;
    }
}

uint32_t Neo::getNetworkMagic() const {
    auto version = rpcClient_->getVersion();
    if (version) {
        auto ver = version->getVersion();
        // The nonce field contains the network magic value in Neo N3
        // Check if it matches known network magic values
        if (ver.nonce == NeoConstants::NEO_N3_MAINNET_MAGIC) {
            return NeoConstants::NEO_N3_MAINNET_MAGIC;
        } else if (ver.nonce == NeoConstants::NEO_N3_TESTNET_MAGIC) {
            return NeoConstants::NEO_N3_TESTNET_MAGIC;
        }
        // Return the actual nonce value which represents the network magic
        return ver.nonce;
    }
    // Default to testnet if not connected
    return NeoConstants::NEO_N3_TESTNET_MAGIC;
}

std::string Neo::getNodeVersion() const {
    auto version = rpcClient_->getVersion();
    if (version) {
        auto ver = version->getVersion();
        return ver.userAgent;
    }
    return "";
}

void Neo::shutdown() {
    unsubscribeFromBlocks();
    if (httpService_) {
        // Cleanup HTTP service if needed
    }
}

} // namespace neocpp