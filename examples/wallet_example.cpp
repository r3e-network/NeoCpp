#include <neocpp/neocpp.hpp>
#include <iostream>

using namespace neocpp;

int main() {
    try {
        // Create a new wallet
        auto wallet = std::make_shared<Wallet>("MyWallet", "1.0");
        
        // Create a new account
        auto account = wallet->createAccount("Main Account");
        std::cout << "Created account: " << account->getAddress() << std::endl;
        
        // Export private key as WIF
        std::string wif = account->exportWIF();
        std::cout << "WIF: " << wif << std::endl;
        
        // Lock account with password
        account->lock("mypassword");
        std::cout << "Account locked" << std::endl;
        
        // Unlock account
        if (account->unlock("mypassword")) {
            std::cout << "Account unlocked successfully" << std::endl;
        }
        
        // Save wallet to file
        wallet->save("wallet.json", "mypassword");
        std::cout << "Wallet saved to wallet.json" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}