#pragma once

#include <memory>
#include <vector>
#include <functional>
#include "neocpp/types/types.hpp"
#include "neocpp/types/hash160.hpp"

namespace neocpp {

// Forward declarations
class Transaction;
class Signer;
class Witness;
class NeoRpcClient;
class Account;
class ContractParameter;

/// Builder class for constructing Neo transactions
class TransactionBuilder {
private:
    SharedPtr<Transaction> transaction_;
    SharedPtr<NeoRpcClient> client_;
    std::vector<SharedPtr<Account>> signingAccounts_;
    
public:
    /// Constructor
    /// @param client The RPC client to use for blockchain queries
    explicit TransactionBuilder(const SharedPtr<NeoRpcClient>& client = nullptr);
    
    /// Destructor
    ~TransactionBuilder() = default;
    
    /// Set the RPC client
    /// @param client The RPC client
    /// @return Reference to this builder
    TransactionBuilder& setClient(const SharedPtr<NeoRpcClient>& client);
    
    /// Set the nonce (random value)
    /// @param nonce The nonce value
    /// @return Reference to this builder
    TransactionBuilder& setNonce(uint32_t nonce);
    
    /// Set a random nonce
    /// @return Reference to this builder
    TransactionBuilder& setRandomNonce();
    
    /// Set the valid until block
    /// @param block The block number
    /// @return Reference to this builder
    TransactionBuilder& setValidUntilBlock(uint32_t block);
    
    /// Set valid until block relative to current
    /// @param blocksFromNow Number of blocks from current
    /// @return Reference to this builder
    TransactionBuilder& setValidUntilBlockRelative(uint32_t blocksFromNow);
    
    /// Set the system fee
    /// @param fee The system fee in GAS fractions
    /// @return Reference to this builder
    TransactionBuilder& setSystemFee(int64_t fee);
    
    /// Set the network fee
    /// @param fee The network fee in GAS fractions
    /// @return Reference to this builder
    TransactionBuilder& setNetworkFee(int64_t fee);
    
    /// Calculate and set the network fee automatically
    /// @return Reference to this builder
    TransactionBuilder& calculateNetworkFee();
    
    /// Add a signer
    /// @param signer The signer
    /// @return Reference to this builder
    TransactionBuilder& addSigner(const SharedPtr<Signer>& signer);
    
    /// Add a signer from account
    /// @param account The account to sign with
    /// @param scopes The witness scopes
    /// @return Reference to this builder
    TransactionBuilder& addSigner(const SharedPtr<Account>& account, uint8_t scopes = 0x01);
    
    /// Set the transaction script
    /// @param script The script bytes
    /// @return Reference to this builder
    TransactionBuilder& setScript(const Bytes& script);
    
    /// Call a contract method
    /// @param scriptHash The contract script hash
    /// @param method The method name
    /// @param params The parameters
    /// @return Reference to this builder
    TransactionBuilder& callContract(const Hash160& scriptHash, const std::string& method, const std::vector<ContractParameter>& params = {});
    
    /// Transfer NEO
    /// @param from The sender account
    /// @param to The recipient address
    /// @param amount The amount of NEO
    /// @return Reference to this builder
    TransactionBuilder& transferNeo(const SharedPtr<Account>& from, const std::string& to, int64_t amount);
    
    /// Transfer GAS
    /// @param from The sender account
    /// @param to The recipient address
    /// @param amount The amount of GAS (in fractions)
    /// @return Reference to this builder
    TransactionBuilder& transferGas(const SharedPtr<Account>& from, const std::string& to, int64_t amount);
    
    /// Transfer NEP-17 token
    /// @param tokenHash The token contract hash
    /// @param from The sender account
    /// @param to The recipient address
    /// @param amount The amount to transfer
    /// @param decimals The token decimals
    /// @return Reference to this builder
    TransactionBuilder& transferNep17(const Hash160& tokenHash, const SharedPtr<Account>& from, const std::string& to, int64_t amount, int decimals = 0);
    
    /// Add a witness
    /// @param witness The witness
    /// @return Reference to this builder
    TransactionBuilder& addWitness(const SharedPtr<Witness>& witness);
    
    /// Sign the transaction with an account
    /// @param account The account to sign with
    /// @return Reference to this builder
    TransactionBuilder& sign(const SharedPtr<Account>& account);
    
    /// Sign the transaction with all added signers
    /// @return Reference to this builder
    TransactionBuilder& signWithSigners();
    
    /// Build the transaction
    /// @return The built transaction
    SharedPtr<Transaction> build();
    
    /// Build and sign the transaction
    /// @return The signed transaction
    SharedPtr<Transaction> buildAndSign();
    
    /// Get the current transaction being built
    /// @return The transaction
    SharedPtr<Transaction> getTransaction() const { return transaction_; }
    
private:
    /// Initialize transaction with defaults
    void initializeTransaction();
    
    /// Add witness for account
    void addWitnessForAccount(const SharedPtr<Account>& account);
    
    /// Sort signers according to Neo rules
    void sortSigners();
    
    /// Sort witnesses according to signers
    void sortWitnesses();
};

} // namespace neocpp