#include "neocpp/protocol/core/witnessrule/witness_condition.hpp"
#include "neocpp/serialization/binary_writer.hpp"
#include "neocpp/serialization/binary_reader.hpp"
#include "neocpp/utils/hex.hpp"

namespace neocpp {

SharedPtr<WitnessCondition> WitnessCondition::deserialize(BinaryReader& reader) {
    auto type = static_cast<WitnessConditionType>(reader.readByte());
    
    switch (type) {
        case WitnessConditionType::BOOLEAN: {
            bool value = reader.readBool();
            return std::make_shared<BooleanCondition>(value);
        }
        
        case WitnessConditionType::NOT: {
            auto expression = deserialize(reader);
            return std::make_shared<NotCondition>(expression);
        }
        
        case WitnessConditionType::AND: {
            auto count = reader.readVarInt();
            std::vector<SharedPtr<WitnessCondition>> expressions;
            for (size_t i = 0; i < count; ++i) {
                expressions.push_back(deserialize(reader));
            }
            return std::make_shared<AndCondition>(expressions);
        }
        
        case WitnessConditionType::OR: {
            auto count = reader.readVarInt();
            std::vector<SharedPtr<WitnessCondition>> expressions;
            for (size_t i = 0; i < count; ++i) {
                expressions.push_back(deserialize(reader));
            }
            return std::make_shared<OrCondition>(expressions);
        }
        
        case WitnessConditionType::SCRIPT_HASH: {
            Hash160 scriptHash(reader.readBytes(20));
            return std::make_shared<ScriptHashCondition>(scriptHash);
        }
        
        case WitnessConditionType::GROUP: {
            auto pubKey = reader.readBytes(33);
            return std::make_shared<GroupCondition>(pubKey);
        }
        
        case WitnessConditionType::CALLED_BY_ENTRY:
            return std::make_shared<CalledByEntryCondition>();
        
        case WitnessConditionType::CALLED_BY_CONTRACT: {
            Hash160 scriptHash(reader.readBytes(20));
            return std::make_shared<CalledByContractCondition>(scriptHash);
        }
        
        case WitnessConditionType::CALLED_BY_GROUP: {
            auto pubKey = reader.readBytes(33);
            return std::make_shared<CalledByGroupCondition>(pubKey);
        }
        
        default:
            throw std::runtime_error("Unknown witness condition type");
    }
}

// BooleanCondition implementation
size_t BooleanCondition::getSize() const {
    return 1 + 1; // type + boolean
}

void BooleanCondition::serialize(BinaryWriter& writer) const {
    writer.writeByte(static_cast<uint8_t>(type_));
    writer.writeBool(value_);
}

nlohmann::json BooleanCondition::toJson() const {
    return {
        {"type", "Boolean"},
        {"value", value_}
    };
}

// NotCondition implementation
size_t NotCondition::getSize() const {
    return 1 + expression_->getSize(); // type + expression
}

void NotCondition::serialize(BinaryWriter& writer) const {
    writer.writeByte(static_cast<uint8_t>(type_));
    expression_->serialize(writer);
}

nlohmann::json NotCondition::toJson() const {
    return {
        {"type", "Not"},
        {"expression", expression_->toJson()}
    };
}

// AndCondition implementation
size_t AndCondition::getSize() const {
    size_t size = 1; // type
    size += 1 + expressions_.size() * expressions_[0]->getSize();  // varInt + expressions
    for (const auto& expr : expressions_) {
        size += expr->getSize();
    }
    return size;
}

void AndCondition::serialize(BinaryWriter& writer) const {
    writer.writeByte(static_cast<uint8_t>(type_));
    writer.writeVarInt(expressions_.size());
    for (const auto& expr : expressions_) {
        expr->serialize(writer);
    }
}

nlohmann::json AndCondition::toJson() const {
    nlohmann::json exprs = nlohmann::json::array();
    for (const auto& expr : expressions_) {
        exprs.push_back(expr->toJson());
    }
    return {
        {"type", "And"},
        {"expressions", exprs}
    };
}

// OrCondition implementation
size_t OrCondition::getSize() const {
    size_t size = 1; // type
    size += 1 + expressions_.size() * expressions_[0]->getSize();  // varInt + expressions
    for (const auto& expr : expressions_) {
        size += expr->getSize();
    }
    return size;
}

void OrCondition::serialize(BinaryWriter& writer) const {
    writer.writeByte(static_cast<uint8_t>(type_));
    writer.writeVarInt(expressions_.size());
    for (const auto& expr : expressions_) {
        expr->serialize(writer);
    }
}

nlohmann::json OrCondition::toJson() const {
    nlohmann::json exprs = nlohmann::json::array();
    for (const auto& expr : expressions_) {
        exprs.push_back(expr->toJson());
    }
    return {
        {"type", "Or"},
        {"expressions", exprs}
    };
}

// ScriptHashCondition implementation
size_t ScriptHashCondition::getSize() const {
    return 1 + 20; // type + hash160
}

void ScriptHashCondition::serialize(BinaryWriter& writer) const {
    writer.writeByte(static_cast<uint8_t>(type_));
    writer.writeBytes(scriptHash_.toArray());
}

nlohmann::json ScriptHashCondition::toJson() const {
    return {
        {"type", "ScriptHash"},
        {"hash", scriptHash_.toString()}
    };
}

// GroupCondition implementation
size_t GroupCondition::getSize() const {
    return 1 + 33; // type + public key
}

void GroupCondition::serialize(BinaryWriter& writer) const {
    writer.writeByte(static_cast<uint8_t>(type_));
    writer.writeBytes(pubKey_);
}

nlohmann::json GroupCondition::toJson() const {
    return {
        {"type", "Group"},
        {"pubkey", Hex::encode(pubKey_)}
    };
}

// CalledByEntryCondition implementation
size_t CalledByEntryCondition::getSize() const {
    return 1; // type only
}

void CalledByEntryCondition::serialize(BinaryWriter& writer) const {
    writer.writeByte(static_cast<uint8_t>(type_));
}

nlohmann::json CalledByEntryCondition::toJson() const {
    return {
        {"type", "CalledByEntry"}
    };
}

// CalledByContractCondition implementation
size_t CalledByContractCondition::getSize() const {
    return 1 + 20; // type + hash160
}

void CalledByContractCondition::serialize(BinaryWriter& writer) const {
    writer.writeByte(static_cast<uint8_t>(type_));
    writer.writeBytes(scriptHash_.toArray());
}

nlohmann::json CalledByContractCondition::toJson() const {
    return {
        {"type", "CalledByContract"},
        {"hash", scriptHash_.toString()}
    };
}

// CalledByGroupCondition implementation
size_t CalledByGroupCondition::getSize() const {
    return 1 + 33; // type + public key
}

void CalledByGroupCondition::serialize(BinaryWriter& writer) const {
    writer.writeByte(static_cast<uint8_t>(type_));
    writer.writeBytes(pubKey_);
}

nlohmann::json CalledByGroupCondition::toJson() const {
    return {
        {"type", "CalledByGroup"},
        {"pubkey", Hex::encode(pubKey_)}
    };
}

} // namespace neocpp