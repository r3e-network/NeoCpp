#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "neocpp/types/types.hpp"

namespace neocpp {

// Forward declarations
class NeoRpcClient;
class HttpService;
class BlockPolling;

/// Main Neo blockchain interface
class Neo {
protected:
    SharedPtr<NeoRpcClient> rpcClient_;
    SharedPtr<HttpService> httpService_;
    SharedPtr<BlockPolling> blockPolling_;
    std::string rpcUrl_;
    
public:
    /// Constructor with default configuration
    Neo();
    
    /// Constructor with RPC URL
    /// @param rpcUrl The RPC endpoint URL
    explicit Neo(const std::string& rpcUrl);
    
    /// Destructor
    virtual ~Neo() = default;
    
    /// Get RPC client
    SharedPtr<NeoRpcClient> getRpcClient() const { return rpcClient_; }
    
    /// Get HTTP service
    SharedPtr<HttpService> getHttpService() const { return httpService_; }
    
    /// Get RPC URL
    const std::string& getRpcUrl() const { return rpcUrl_; }
    
    /// Subscribe to new blocks
    /// @param callback The callback function
    void subscribeToBlocks(std::function<void(uint32_t)> callback);
    
    /// Unsubscribe from blocks
    void unsubscribeFromBlocks();
    
    /// Check if connected
    /// @return True if connected to node
    bool isConnected() const;
    
    /// Get network magic
    /// @return The network magic number
    uint32_t getNetworkMagic() const;
    
    /// Get node version
    /// @return The node version info
    std::string getNodeVersion() const;
    
    /// Shutdown and cleanup
    void shutdown();
    
protected:
    /// Initialize components
    void initialize();
};

} // namespace neocpp