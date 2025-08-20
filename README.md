# NeoCpp - Neo N3 C++ SDK

A comprehensive C++ SDK for interacting with the Neo N3 blockchain, providing full functionality for wallet management, smart contract interaction, and blockchain operations.

**Status: ✅ PRODUCTION READY** - Successfully converted from Neo Swift SDK to C++ with 100% test pass rate

[![CI](https://github.com/r3e-network/NeoCpp/actions/workflows/ci.yml/badge.svg)](https://github.com/r3e-network/NeoCpp/actions)
[![Tests](https://img.shields.io/badge/tests-692%20passing-brightgreen)](./TESTING.md)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://isocpp.org/std/the-standard)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

## Features

- 🔐 **Full Cryptography Support**: ECDSA signatures, key generation, NEP-2 encryption
- 👛 **Wallet Management**: Create, import, and manage Neo wallets and accounts
- 📜 **Smart Contracts**: Deploy and interact with Neo N3 smart contracts
- 🔗 **RPC Client**: Complete JSON-RPC client for Neo nodes
- 💰 **Token Support**: NEP-17 (fungible) and NEP-11 (NFT) token standards
- 🏗️ **Transaction Builder**: Fluent API for constructing and signing transactions
- 🔧 **Script Builder**: Construct Neo VM scripts programmatically

## Requirements

- C++17 compatible compiler
- CMake 3.20 or higher
- OpenSSL
- libcurl
- nlohmann/json (fetched automatically)
- secp256k1 (fetched automatically)

## Installation

### Building from Source

```bash
# Clone the repository
git clone https://github.com/r3e-network/NeoCpp.git
cd NeoCpp

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build
make -j$(nproc)

# Run tests (optional)
make test

# Install
sudo make install
```

### CMake Integration

Add NeoCpp to your project using CMake:

```cmake
find_package(NeoCpp REQUIRED)
target_link_libraries(your_target NeoCpp::neocpp)
```

Or using FetchContent:

```cmake
include(FetchContent)
FetchContent_Declare(
    NeoCpp
    GIT_REPOSITORY https://github.com/r3e-network/NeoCpp.git
    GIT_TAG main
)
FetchContent_MakeAvailable(NeoCpp)
target_link_libraries(your_target neocpp)
```

## Quick Start

### Creating a Wallet

```cpp
#include <neocpp/neocpp.hpp>

using namespace neocpp;

// Create a new wallet
auto wallet = std::make_shared<Wallet>("MyWallet");

// Create a new account
auto account = wallet->createAccount("Main Account");

// Get the address
std::cout << "Address: " << account->getAddress() << std::endl;

// Export private key as WIF
std::cout << "WIF: " << account->exportWIF() << std::endl;

// Save wallet to file
wallet->save("wallet.json", "password123");
```

### Connecting to Neo Node

```cpp
#include <neocpp/neocpp.hpp>

using namespace neocpp;

// Create RPC client
auto client = std::make_shared<NeoRpcClient>("http://localhost:10332");

// Get node version
auto version = client->getVersion();
std::cout << "Node version: " << version->getRawJson().dump() << std::endl;

// Get current block count
uint32_t blockCount = client->getBlockCount();
std::cout << "Block count: " << blockCount << std::endl;
```

### Transferring Tokens

```cpp
#include <neocpp/neocpp.hpp>

using namespace neocpp;

// Create RPC client
auto client = std::make_shared<NeoRpcClient>("http://localhost:10332");

// Load wallet
auto wallet = Wallet::load("wallet.json", "password123");
auto account = wallet->getDefaultAccount();

// Create transaction builder
TransactionBuilder builder(client);

// Transfer 10 GAS
builder.transferGas(account, "NTh9TnZTstvAePEYWDGLLxidBikz7Xfj5j", 10 * 100000000)
       .setValidUntilBlockRelative(100)
       .calculateNetworkFee();

// Build and sign transaction
auto tx = builder.buildAndSign();

// Send transaction
Hash256 txHash = client->sendRawTransaction(tx);
std::cout << "Transaction sent: " << txHash.toString() << std::endl;
```

### Interacting with Smart Contracts

```cpp
#include <neocpp/neocpp.hpp>

using namespace neocpp;

// Create RPC client
auto client = std::make_shared<NeoRpcClient>("http://localhost:10332");

// Contract script hash
Hash160 contractHash("0xd2a4cff31913016155e38e474a2c06d08be276cf");

// Create smart contract instance
SmartContract contract(contractHash, client);

// Call read-only method
auto result = contract.invokeFunction("symbol");
std::cout << "Token symbol: " << result["stack"][0]["value"].get<std::string>() << std::endl;

// Build transaction for state-changing method
auto account = Account::fromWIF("L1ZW4XvSLVxNQzNGwpDZg1H8mjkVCtsncYqPL9BvvAg8VkqTwqGy");
auto builder = contract.buildInvokeTx("transfer", {
    ContractParameter::hash160(account->getScriptHash()),
    ContractParameter::hash160(Hash160::fromAddress("NTh9TnZTstvAePEYWDGLLxidBikz7Xfj5j")),
    ContractParameter::integer(1000000),
    ContractParameter::any()
}, account);

// Sign and send
auto tx = builder->buildAndSign();
client->sendRawTransaction(tx);
```

## API Documentation

### Core Types

- `Hash160` - 20-byte hash (script hashes, contract hashes)
- `Hash256` - 32-byte hash (transaction hashes, block hashes)
- `ECKeyPair` - Elliptic curve key pair
- `ECPoint` - Point on elliptic curve
- `ECDSASignature` - ECDSA signature

### Wallet Components

- `Wallet` - Wallet container for accounts
- `Account` - Individual account with keys
- `NEP2` - NEP-2 encryption/decryption
- `WIF` - Wallet Import Format encoding

### Transaction Components

- `Transaction` - Neo transaction
- `TransactionBuilder` - Fluent transaction builder
- `Signer` - Transaction signer with witness scope
- `Witness` - Transaction witness (signature)

### Smart Contract Components

- `SmartContract` - Base smart contract class
- `FungibleToken` - NEP-17 token contract
- `NonFungibleToken` - NEP-11 NFT contract
- `NeoToken` - Native NEO token
- `GasToken` - Native GAS token

### RPC Client

- `NeoRpcClient` - JSON-RPC client for Neo nodes
- Various response types for different RPC methods

## Examples

See the `examples` directory for complete examples:

- `wallet_example.cpp` - Wallet creation and management
- `transfer_example.cpp` - Token transfers
- `contract_example.cpp` - Smart contract interaction
- `nft_example.cpp` - NFT operations

## Testing

The SDK includes comprehensive tests with **100% pass rate** (692 assertions in 30 test cases).

```bash
cd build
./tests/neocpp_tests  # Run all tests
./tests/neocpp_tests --success  # Show all test results
./tests/neocpp_tests "NEP2*"  # Run specific tests
```

See [TESTING.md](./TESTING.md) for detailed testing documentation.

## Production Readiness

This SDK is **production ready** with:

- ✅ **100% Test Pass Rate**: All 692 assertions passing
- ✅ **Memory Safe**: No leaks detected with AddressSanitizer
- ✅ **Security Hardened**: NEP-2 encryption, secure random generation
- ✅ **Performance Optimized**: Sub-millisecond operations
- ✅ **CI/CD Ready**: GitHub Actions configuration included

See [PRODUCTION_READINESS.md](./PRODUCTION_READINESS.md) for the complete checklist.

## Contributing

Contributions are welcome! Please feel free to submit pull requests.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

This C++ SDK is a port of the NeoSwift SDK, maintaining compatibility with Neo N3 blockchain standards.