#include "neocpp/protocol/core/witnessrule/witness_rule.hpp"
#include "neocpp/protocol/core/witnessrule/witness_condition.hpp"
#include "neocpp/serialization/binary_writer.hpp"
#include "neocpp/serialization/binary_reader.hpp"
#include "neocpp/utils/hex.hpp"

namespace neocpp {

size_t WitnessRule::getSize() const {
    size_t size = 1; // action
    if (condition_) {
        size += condition_->getSize();
    }
    return size;
}

void WitnessRule::serialize(BinaryWriter& writer) const {
    writer.writeByte(static_cast<uint8_t>(action_));
    if (condition_) {
        condition_->serialize(writer);
    }
}

WitnessRule WitnessRule::deserialize(BinaryReader& reader) {
    WitnessRule rule;
    rule.action_ = static_cast<WitnessAction>(reader.readByte());
    rule.condition_ = WitnessCondition::deserialize(reader);
    return rule;
}

nlohmann::json WitnessRule::toJson() const {
    nlohmann::json json;
    json["action"] = toString(action_);
    if (condition_) {
        json["condition"] = condition_->toJson();
    }
    return json;
}

SharedPtr<WitnessRule> WitnessRule::fromJson(const nlohmann::json& json) {
    auto rule = std::make_shared<WitnessRule>();
    
    if (json.contains("action")) {
        rule->action_ = witnessActionFromString(json["action"].get<std::string>());
    }
    
    // Parse condition based on type
    if (json.contains("condition") && !json["condition"].is_null()) {
        rule->condition_ = parseConditionFromJson(json["condition"]);
    }
    
    return rule;
}

SharedPtr<WitnessCondition> WitnessRule::parseConditionFromJson(const nlohmann::json& json) {
    if (!json.contains("type")) {
        return nullptr;
    }
    
    std::string typeStr = json["type"].get<std::string>();
    
    if (typeStr == "Boolean") {
        bool value = json.value("value", false);
        return std::make_shared<BooleanCondition>(value);
    }
    else if (typeStr == "Not") {
        if (json.contains("expression")) {
            auto expr = parseConditionFromJson(json["expression"]);
            return std::make_shared<NotCondition>(expr);
        }
    }
    else if (typeStr == "And") {
        if (json.contains("expressions") && json["expressions"].is_array()) {
            std::vector<SharedPtr<WitnessCondition>> expressions;
            for (const auto& expr : json["expressions"]) {
                auto condition = parseConditionFromJson(expr);
                if (condition) {
                    expressions.push_back(condition);
                }
            }
            return std::make_shared<AndCondition>(expressions);
        }
    }
    else if (typeStr == "Or") {
        if (json.contains("expressions") && json["expressions"].is_array()) {
            std::vector<SharedPtr<WitnessCondition>> expressions;
            for (const auto& expr : json["expressions"]) {
                auto condition = parseConditionFromJson(expr);
                if (condition) {
                    expressions.push_back(condition);
                }
            }
            return std::make_shared<OrCondition>(expressions);
        }
    }
    else if (typeStr == "ScriptHash") {
        if (json.contains("hash")) {
            std::string hashStr = json["hash"].get<std::string>();
            Hash160 hash(Hex::decode(hashStr));
            return std::make_shared<ScriptHashCondition>(hash);
        }
    }
    else if (typeStr == "Group") {
        if (json.contains("publicKey")) {
            std::string pubKeyStr = json["publicKey"].get<std::string>();
            Bytes pubKey = Hex::decode(pubKeyStr);
            return std::make_shared<GroupCondition>(pubKey);
        }
    }
    else if (typeStr == "CalledByEntry") {
        return std::make_shared<CalledByEntryCondition>();
    }
    else if (typeStr == "CalledByContract") {
        if (json.contains("hash")) {
            std::string hashStr = json["hash"].get<std::string>();
            Hash160 hash(Hex::decode(hashStr));
            return std::make_shared<CalledByContractCondition>(hash);
        }
    }
    else if (typeStr == "CalledByGroup") {
        if (json.contains("publicKey")) {
            std::string pubKeyStr = json["publicKey"].get<std::string>();
            Bytes pubKey = Hex::decode(pubKeyStr);
            return std::make_shared<CalledByGroupCondition>(pubKey);
        }
    }
    
    return nullptr;
}

} // namespace neocpp