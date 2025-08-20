#include "neocpp/transaction/transaction_attribute.hpp"
#include "neocpp/serialization/binary_writer.hpp"
#include "neocpp/serialization/binary_reader.hpp"
#include "neocpp/exceptions.hpp"

namespace neocpp {

void TransactionAttribute::serialize(BinaryWriter& writer) const {
    writer.writeUInt8(static_cast<uint8_t>(getType()));
    serializeWithoutType(writer);
}

SharedPtr<TransactionAttribute> TransactionAttribute::deserialize(BinaryReader& reader) {
    uint8_t type = reader.readUInt8();
    
    switch (static_cast<TransactionAttributeType>(type)) {
        case TransactionAttributeType::HIGH_PRIORITY:
            return std::make_shared<HighPriorityAttribute>();
            
        case TransactionAttributeType::ORACLE_RESPONSE: {
            uint64_t id = reader.readUInt64();
            uint8_t code = reader.readUInt8();
            Bytes result = reader.readVarBytes();
            return std::make_shared<OracleResponseAttribute>(id, code, result);
        }
        
        case TransactionAttributeType::NOT_VALID_BEFORE: {
            uint32_t height = reader.readUInt32();
            return std::make_shared<NotValidBeforeAttribute>(height);
        }
        
        case TransactionAttributeType::CONFLICTS: {
            Hash256 hash = Hash256::deserialize(reader);
            return std::make_shared<ConflictsAttribute>(hash);
        }
        
        default:
            throw DeserializationException("Unknown transaction attribute type: " + std::to_string(type));
    }
}

void OracleResponseAttribute::serializeWithoutType(BinaryWriter& writer) const {
    writer.writeUInt64(id_);
    writer.writeUInt8(code_);
    writer.writeVarBytes(result_);
}

void NotValidBeforeAttribute::serializeWithoutType(BinaryWriter& writer) const {
    writer.writeUInt32(height_);
}

void ConflictsAttribute::serializeWithoutType(BinaryWriter& writer) const {
    hash_.serialize(writer);
}

} // namespace neocpp