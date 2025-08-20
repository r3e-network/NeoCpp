#include <neocpp/neocpp.hpp>
#include <iostream>

using namespace neocpp;

int main() {
    try {
        // Create a transaction
        auto tx = std::make_shared<Transaction>();
        tx->setNonce(12345);
        tx->setSystemFee(1000000);
        tx->setNetworkFee(500000);
        tx->setValidUntilBlock(1000);
        
        // Create a signer
        Hash160 accountHash;
        auto signer = std::make_shared<Signer>(accountHash, WitnessScope::CALLED_BY_ENTRY);
        tx->addSigner(signer);
        
        // Build script
        ScriptBuilder sb;
        sb.emitPush(100);
        sb.emitPush("Hello NEO");
        tx->setScript(sb.toArray());
        
        // Get transaction hash
        Hash256 txHash = tx->getHash();
        std::cout << "Transaction hash: " << txHash.toString() << std::endl;
        
        // Serialize transaction
        BinaryWriter writer;
        tx->serialize(writer);
        Bytes serialized = writer.toArray();
        std::cout << "Serialized size: " << serialized.size() << " bytes" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}