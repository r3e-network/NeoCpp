#pragma once

#include "neocpp/transaction/signer.hpp"
#include <memory>

namespace neocpp {

// Forward declaration
class Account;

/// Account-based transaction signer
class AccountSigner : public Signer {
private:
    SharedPtr<Account> account_;
    
public:
    /// Constructor
    /// @param account The account to sign with
    /// @param scopes The witness scopes
    AccountSigner(const SharedPtr<Account>& account, WitnessScope scopes = WitnessScope::CALLED_BY_ENTRY);
    
    /// Destructor
    ~AccountSigner() = default;
    
    /// Get the account
    const SharedPtr<Account>& getAccount() const { return account_; }
    
    /// Create witness for this signer
    /// @param message The message to sign
    /// @return The witness
    SharedPtr<Witness> createWitness(const Bytes& message) const;
    
    /// Sign a message
    /// @param message The message to sign
    /// @return The signature
    Bytes sign(const Bytes& message) const;
};

} // namespace neocpp