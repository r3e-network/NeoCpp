#include "neocpp/protocol/core/polling/block_polling.hpp"
#include "neocpp/protocol/neo_rpc_client.hpp"

namespace neocpp {

BlockPolling::BlockPolling(const SharedPtr<NeoRpcClient>& rpcClient, std::chrono::milliseconds pollInterval)
    : rpcClient_(rpcClient), running_(false), lastBlockIndex_(0), pollInterval_(pollInterval) {
}

BlockPolling::~BlockPolling() {
    stop();
}

void BlockPolling::start() {
    if (running_) {
        return;
    }
    
    running_ = true;
    pollingThread_ = std::make_unique<std::thread>(&BlockPolling::pollLoop, this);
}

void BlockPolling::stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    if (pollingThread_ && pollingThread_->joinable()) {
        pollingThread_->join();
    }
}

void BlockPolling::subscribe(std::function<void(uint32_t)> callback) {
    callbacks_.push_back(callback);
}

void BlockPolling::clearSubscriptions() {
    callbacks_.clear();
}

void BlockPolling::pollLoop() {
    while (running_) {
        try {
            auto blockCount = rpcClient_->getBlockCount();
            if (blockCount > 0) {
                uint32_t currentBlock = blockCount - 1;
                if (currentBlock > lastBlockIndex_) {
                    lastBlockIndex_ = currentBlock;
                    notifySubscribers(currentBlock);
                }
            }
        } catch (...) {
            // Ignore errors and continue polling
        }
        
        std::this_thread::sleep_for(pollInterval_);
    }
}

void BlockPolling::notifySubscribers(uint32_t blockIndex) {
    for (const auto& callback : callbacks_) {
        try {
            callback(blockIndex);
        } catch (...) {
            // Ignore callback errors
        }
    }
}

} // namespace neocpp