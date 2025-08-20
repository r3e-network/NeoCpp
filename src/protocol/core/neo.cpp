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
        return ver.nonce;  // Use nonce as network magic for now
    }
    return 0;
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