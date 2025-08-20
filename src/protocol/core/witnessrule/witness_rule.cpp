#include "neocpp/protocol/core/witnessrule/witness_rule.hpp"
#include "neocpp/serialization/binary_writer.hpp"
#include "neocpp/serialization/binary_reader.hpp"

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
    
    // Condition parsing would need to be implemented based on type
    // This is simplified for now
    
    return rule;
}

} // namespace neocpp