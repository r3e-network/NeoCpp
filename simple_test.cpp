#include <iostream>
#include <iomanip>
#include <neocpp/neocpp.hpp>

using namespace neocpp;

void printHex(const std::string& label, const Bytes& data) {
    std::cout << label << ": ";
    for (uint8_t byte : data) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
    }
    std::cout << std::dec << std::endl;
}

int main() {
    std::cout << "Neo C++ SDK Simple Test" << std::endl;
    std::cout << "========================" << std::endl;
    
    try {
        // Test 1: Basic hashing
        std::cout << "\n1. Testing hash functions..." << std::endl;
        std::string message = "Hello, Neo!";
        Bytes messageBytes(message.begin(), message.end());
        
        Bytes sha256Hash = HashUtils::sha256(messageBytes);
        printHex("   SHA256", sha256Hash);
        
        Bytes hash256 = HashUtils::doubleSha256(messageBytes);
        printHex("   Hash256", hash256);
        
        Bytes hash160 = HashUtils::sha256ThenRipemd160(messageBytes);
        printHex("   Hash160", hash160);
        
        // Test 2: Base58 encoding
        std::cout << "\n2. Testing Base58 encoding..." << std::endl;
        Bytes testData = {0x00, 0x01, 0x02, 0x03, 0x04};
        std::string encoded = Base58::encode(testData);
        std::cout << "   Encoded: " << encoded << std::endl;
        
        Bytes decoded = Base58::decode(encoded);
        printHex("   Decoded", decoded);
        
        // Test 3: Script builder
        std::cout << "\n3. Testing script builder..." << std::endl;
        ScriptBuilder sb;
        sb.pushInteger(42);
        sb.pushString("NEO");
        sb.emit(OpCode::NOP);
        
        Bytes script = sb.toArray();
        std::cout << "   Script size: " << script.size() << " bytes" << std::endl;
        printHex("   Script", script);
        
        // Test 4: Hash types
        std::cout << "\n4. Testing hash types..." << std::endl;
        Hash256 hash256Type(hash256);
        std::cout << "   Hash256: " << hash256Type.toString() << std::endl;
        
        Hash160 hash160Type(hash160);
        std::cout << "   Hash160: " << hash160Type.toString() << std::endl;
        
        // Test 5: Transaction creation (basic)
        std::cout << "\n5. Creating basic transaction..." << std::endl;
        auto tx = std::make_shared<Transaction>();
        tx->setNonce(12345);
        tx->setSystemFee(1000000);
        tx->setNetworkFee(500000);
        tx->setValidUntilBlock(1000);
        tx->setScript(script);
        
        Hash256 txHash = tx->getHash();
        std::cout << "   Transaction hash: " << txHash.toString() << std::endl;
        std::cout << "   Version: " << (int)tx->getVersion() << std::endl;
        std::cout << "   Nonce: " << tx->getNonce() << std::endl;
        
        std::cout << "\nAll tests completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}