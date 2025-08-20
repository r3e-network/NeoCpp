#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "neocpp/types/types.hpp"
#include "neocpp/types/hash256.hpp"
#include "neocpp/types/hash160.hpp"

namespace neocpp {

/// Base class for RPC responses
class NeoResponse {
protected:
    nlohmann::json rawJson_;
    
public:
    explicit NeoResponse(const nlohmann::json& json) : rawJson_(json) {}
    virtual ~NeoResponse() = default;
    
    const nlohmann::json& getRawJson() const { return rawJson_; }
};

/// Version response
class NeoGetVersionResponse : public NeoResponse {
public:
    struct Version {
        int tcpPort;
        int wsPort;
        uint32_t nonce;
        std::string userAgent;
        uint32_t startHeight;
        uint32_t currentBlockIndex;
        uint32_t currentStateRootIndex;
    };
    
    explicit NeoGetVersionResponse(const nlohmann::json& json);
    
    Version getVersion() const;
};

/// Block response
class NeoGetBlockResponse : public NeoResponse {
public:
    struct Block {
        Hash256 hash;
        uint32_t size;
        uint32_t version;
        Hash256 previousBlockHash;
        Hash256 merkleRoot;
        uint64_t timestamp;
        uint64_t nonce;
        uint32_t index;
        uint8_t primaryIndex;
        Hash160 nextConsensus;
        std::vector<nlohmann::json> witnesses;
        std::vector<nlohmann::json> transactions;
    };
    
    explicit NeoGetBlockResponse(const nlohmann::json& json);
    
    Block getBlock() const;
};

/// Transaction response
class NeoGetRawTransactionResponse : public NeoResponse {
public:
    struct Transaction {
        Hash256 hash;
        uint32_t size;
        uint8_t version;
        uint32_t nonce;
        std::string sender;
        int64_t systemFee;
        int64_t networkFee;
        uint32_t validUntilBlock;
        std::vector<nlohmann::json> signers;
        std::vector<nlohmann::json> attributes;
        std::string script;
        std::vector<nlohmann::json> witnesses;
        uint32_t blockHash;
        uint32_t confirmations;
        uint64_t blockTime;
    };
    
    explicit NeoGetRawTransactionResponse(const nlohmann::json& json);
    
    Transaction getTransaction() const;
};

/// Application log response
class NeoGetApplicationLogResponse : public NeoResponse {
public:
    struct Execution {
        std::string trigger;
        std::string vmState;
        int64_t gasConsumed;
        std::vector<nlohmann::json> stack;
        std::vector<nlohmann::json> notifications;
        std::string exception;
    };
    
    struct ApplicationLog {
        Hash256 txId;
        uint32_t blockHash;
        std::vector<Execution> executions;
    };
    
    explicit NeoGetApplicationLogResponse(const nlohmann::json& json);
    
    ApplicationLog getApplicationLog() const;
};

/// Contract state response
class NeoGetContractStateResponse : public NeoResponse {
public:
    struct ContractState {
        int id;
        uint32_t updateCounter;
        Hash160 hash;
        std::string nef;
        nlohmann::json manifest;
    };
    
    explicit NeoGetContractStateResponse(const nlohmann::json& json);
    
    ContractState getContractState() const;
};

/// NEP-17 balances response
class NeoGetNep17BalancesResponse : public NeoResponse {
public:
    struct Balance {
        Hash160 assetHash;
        std::string symbol;
        int decimals;
        std::string name;
        std::string amount;
        uint64_t lastUpdatedBlock;
    };
    
    struct Balances {
        std::string address;
        std::vector<Balance> balances;
    };
    
    explicit NeoGetNep17BalancesResponse(const nlohmann::json& json);
    
    Balances getBalances() const;
};

/// Invoke result response
class NeoInvokeResultResponse : public NeoResponse {
public:
    struct InvokeResult {
        std::string script;
        std::string state;
        int64_t gasConsumed;
        std::string exception;
        std::vector<nlohmann::json> stack;
        std::vector<nlohmann::json> notifications;
        std::vector<nlohmann::json> diagnostics;
        std::string session;
    };
    
    explicit NeoInvokeResultResponse(const nlohmann::json& json);
    
    InvokeResult getResult() const;
};

/// Peers response
class NeoGetPeersResponse : public NeoResponse {
public:
    struct Peer {
        std::string address;
        int port;
    };
    
    struct Peers {
        std::vector<Peer> unconnected;
        std::vector<Peer> bad;
        std::vector<Peer> connected;
    };
    
    explicit NeoGetPeersResponse(const nlohmann::json& json);
    
    Peers getPeers() const;
};

/// Unclaimed GAS response
class NeoGetUnclaimedGasResponse : public NeoResponse {
public:
    struct UnclaimedGas {
        std::string unclaimed;
        std::string address;
    };
    
    explicit NeoGetUnclaimedGasResponse(const nlohmann::json& json);
    
    UnclaimedGas getUnclaimedGas() const;
};

/// Wallet balance response
class NeoGetWalletBalanceResponse : public NeoResponse {
public:
    struct WalletBalance {
        std::string balance;
        std::string address;
        Hash160 assetHash;
    };
    
    explicit NeoGetWalletBalanceResponse(const nlohmann::json& json);
    
    WalletBalance getBalance() const;
};

} // namespace neocpp