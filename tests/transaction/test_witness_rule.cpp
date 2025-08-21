#include <catch2/catch_test_macros.hpp>
#include "neocpp/transaction/witness_rule.hpp"
#include "neocpp/types/hash160.hpp"
#include "neocpp/serialization/binary_writer.hpp"
#include "neocpp/serialization/binary_reader.hpp"
#include "neocpp/exceptions.hpp"

using namespace neocpp;

TEST_CASE("WitnessRule Tests", "[transaction]") {
    
    SECTION("WitnessRuleAction enum values") {
        REQUIRE(static_cast<uint8_t>(WitnessRuleAction::DENY) == 0x00);
        REQUIRE(static_cast<uint8_t>(WitnessRuleAction::ALLOW) == 0x01);
    }
    
    SECTION("WitnessConditionType enum values") {
        REQUIRE(static_cast<uint8_t>(WitnessConditionType::BOOLEAN) == 0x00);
        REQUIRE(static_cast<uint8_t>(WitnessConditionType::NOT) == 0x01);
        REQUIRE(static_cast<uint8_t>(WitnessConditionType::AND) == 0x02);
        REQUIRE(static_cast<uint8_t>(WitnessConditionType::OR) == 0x03);
        REQUIRE(static_cast<uint8_t>(WitnessConditionType::SCRIPT_HASH) == 0x18);
        REQUIRE(static_cast<uint8_t>(WitnessConditionType::GROUP) == 0x19);
        REQUIRE(static_cast<uint8_t>(WitnessConditionType::CALLED_BY_ENTRY) == 0x20);
        REQUIRE(static_cast<uint8_t>(WitnessConditionType::CALLED_BY_CONTRACT) == 0x28);
        REQUIRE(static_cast<uint8_t>(WitnessConditionType::CALLED_BY_GROUP) == 0x29);
    }
    
    SECTION("WitnessCondition boolean condition") {
        auto trueCondition = WitnessCondition::boolean(true);
        REQUIRE(trueCondition->getType() == WitnessConditionType::BOOLEAN);
        
        auto falseCondition = WitnessCondition::boolean(false);
        REQUIRE(falseCondition->getType() == WitnessConditionType::BOOLEAN);
    }
    
    SECTION("WitnessCondition NOT condition") {
        auto innerCondition = WitnessCondition::boolean(true);
        auto notCondition = WitnessCondition::notCondition(innerCondition);
        
        REQUIRE(notCondition->getType() == WitnessConditionType::NOT);
    }
    
    SECTION("WitnessCondition AND condition") {
        auto condition1 = WitnessCondition::boolean(true);
        auto condition2 = WitnessCondition::boolean(false);
        std::vector<SharedPtr<WitnessCondition>> conditions = {condition1, condition2};
        
        auto andCondition = WitnessCondition::andCondition(conditions);
        REQUIRE(andCondition->getType() == WitnessConditionType::AND);
    }
    
    SECTION("WitnessCondition OR condition") {
        auto condition1 = WitnessCondition::boolean(true);
        auto condition2 = WitnessCondition::boolean(false);
        std::vector<SharedPtr<WitnessCondition>> conditions = {condition1, condition2};
        
        auto orCondition = WitnessCondition::orCondition(conditions);
        REQUIRE(orCondition->getType() == WitnessConditionType::OR);
    }
    
    SECTION("WitnessCondition script hash condition") {
        Hash160 hash("1234567890123456789012345678901234567890");
        auto scriptHashCondition = WitnessCondition::scriptHash(hash);
        
        REQUIRE(scriptHashCondition->getType() == WitnessConditionType::SCRIPT_HASH);
    }
    
    SECTION("WitnessCondition called by entry condition") {
        auto calledByEntryCondition = WitnessCondition::calledByEntry();
        REQUIRE(calledByEntryCondition->getType() == WitnessConditionType::CALLED_BY_ENTRY);
    }
    
    SECTION("WitnessCondition size calculation") {
        // Boolean condition: 1 byte (type) + 1 byte (bool) = 2 bytes
        auto boolCondition = WitnessCondition::boolean(true);
        REQUIRE(boolCondition->getSize() == 2);
        
        // Called by entry condition: 1 byte (type) = 1 byte
        auto entryCondition = WitnessCondition::calledByEntry();
        REQUIRE(entryCondition->getSize() == 1);
        
        // Script hash condition: 1 byte (type) + 20 bytes (hash) = 21 bytes
        Hash160 hash("1234567890123456789012345678901234567890");
        auto scriptCondition = WitnessCondition::scriptHash(hash);
        REQUIRE(scriptCondition->getSize() == 21);
        
        // NOT condition: 1 byte (type) + size of inner condition
        auto notCondition = WitnessCondition::notCondition(boolCondition);
        REQUIRE(notCondition->getSize() == 1 + 2); // 3 bytes
        
        // AND condition with 2 boolean conditions: 1 byte (type) + 1 byte (count) + 2*2 bytes (conditions) = 6 bytes
        std::vector<SharedPtr<WitnessCondition>> conditions = {
            WitnessCondition::boolean(true), 
            WitnessCondition::boolean(false)
        };
        auto andCondition = WitnessCondition::andCondition(conditions);
        REQUIRE(andCondition->getSize() == 6);
    }
    
    SECTION("WitnessCondition serialization and deserialization - boolean") {
        auto originalTrue = WitnessCondition::boolean(true);
        auto originalFalse = WitnessCondition::boolean(false);
        
        // Test true condition
        BinaryWriter writer;
        originalTrue->serialize(writer);
        Bytes serialized = writer.toArray();
        
        BinaryReader reader(serialized);
        auto deserialized = WitnessCondition::deserialize(reader);
        
        REQUIRE(deserialized->getType() == WitnessConditionType::BOOLEAN);
        
        // Test false condition
        writer.clear();
        originalFalse->serialize(writer);
        serialized = writer.toArray();
        
        BinaryReader reader2(serialized);
        auto deserialized2 = WitnessCondition::deserialize(reader2);
        
        REQUIRE(deserialized2->getType() == WitnessConditionType::BOOLEAN);
    }
    
    SECTION("WitnessCondition serialization and deserialization - script hash") {
        Hash160 hash("abcdef0123456789012345678901234567890123");
        auto original = WitnessCondition::scriptHash(hash);
        
        BinaryWriter writer;
        original->serialize(writer);
        Bytes serialized = writer.toArray();
        
        BinaryReader reader(serialized);
        auto deserialized = WitnessCondition::deserialize(reader);
        
        REQUIRE(deserialized->getType() == WitnessConditionType::SCRIPT_HASH);
    }
    
    SECTION("WitnessCondition serialization and deserialization - called by entry") {
        auto original = WitnessCondition::calledByEntry();
        
        BinaryWriter writer;
        original->serialize(writer);
        Bytes serialized = writer.toArray();
        
        BinaryReader reader(serialized);
        auto deserialized = WitnessCondition::deserialize(reader);
        
        REQUIRE(deserialized->getType() == WitnessConditionType::CALLED_BY_ENTRY);
    }
    
    SECTION("WitnessCondition serialization and deserialization - NOT condition") {
        auto innerCondition = WitnessCondition::boolean(true);
        auto original = WitnessCondition::notCondition(innerCondition);
        
        BinaryWriter writer;
        original->serialize(writer);
        Bytes serialized = writer.toArray();
        
        BinaryReader reader(serialized);
        auto deserialized = WitnessCondition::deserialize(reader);
        
        REQUIRE(deserialized->getType() == WitnessConditionType::NOT);
    }
    
    SECTION("WitnessCondition serialization and deserialization - AND condition") {
        std::vector<SharedPtr<WitnessCondition>> conditions = {
            WitnessCondition::boolean(true),
            WitnessCondition::calledByEntry()
        };
        auto original = WitnessCondition::andCondition(conditions);
        
        BinaryWriter writer;
        original->serialize(writer);
        Bytes serialized = writer.toArray();
        
        BinaryReader reader(serialized);
        auto deserialized = WitnessCondition::deserialize(reader);
        
        REQUIRE(deserialized->getType() == WitnessConditionType::AND);
    }
    
    SECTION("WitnessCondition serialization and deserialization - OR condition") {
        std::vector<SharedPtr<WitnessCondition>> conditions = {
            WitnessCondition::boolean(false),
            WitnessCondition::calledByEntry()
        };
        auto original = WitnessCondition::orCondition(conditions);
        
        BinaryWriter writer;
        original->serialize(writer);
        Bytes serialized = writer.toArray();
        
        BinaryReader reader(serialized);
        auto deserialized = WitnessCondition::deserialize(reader);
        
        REQUIRE(deserialized->getType() == WitnessConditionType::OR);
    }
    
    SECTION("WitnessCondition deserialization with invalid type") {
        Bytes invalidData = {0xFF}; // Invalid condition type
        BinaryReader reader(invalidData);
        
        REQUIRE_THROWS_AS(WitnessCondition::deserialize(reader), DeserializationException);
    }
    
    SECTION("WitnessRule default constructor") {
        WitnessRule rule;
        REQUIRE(rule.getAction() == WitnessRuleAction::DENY);
        REQUIRE(rule.getCondition() == nullptr);
    }
    
    SECTION("WitnessRule constructor with action and condition") {
        auto condition = WitnessCondition::boolean(true);
        WitnessRule rule(WitnessRuleAction::ALLOW, condition);
        
        REQUIRE(rule.getAction() == WitnessRuleAction::ALLOW);
        REQUIRE(rule.getCondition() != nullptr);
        REQUIRE(rule.getCondition()->getType() == WitnessConditionType::BOOLEAN);
    }
    
    SECTION("WitnessRule setters") {
        WitnessRule rule;
        auto condition = WitnessCondition::calledByEntry();
        
        rule.setAction(WitnessRuleAction::ALLOW);
        rule.setCondition(condition);
        
        REQUIRE(rule.getAction() == WitnessRuleAction::ALLOW);
        REQUIRE(rule.getCondition()->getType() == WitnessConditionType::CALLED_BY_ENTRY);
    }
    
    SECTION("WitnessRule size calculation") {
        auto condition = WitnessCondition::boolean(true); // 2 bytes
        WitnessRule rule(WitnessRuleAction::ALLOW, condition);
        
        // Size should be 1 byte (action) + 2 bytes (condition) = 3 bytes
        REQUIRE(rule.getSize() == 3);
        
        // Rule without condition should be 1 byte
        WitnessRule emptyRule;
        REQUIRE(emptyRule.getSize() == 1);
    }
    
    SECTION("WitnessRule serialization and deserialization") {
        auto condition = WitnessCondition::boolean(true);
        WitnessRule original(WitnessRuleAction::ALLOW, condition);
        
        BinaryWriter writer;
        original.serialize(writer);
        Bytes serialized = writer.toArray();
        
        BinaryReader reader(serialized);
        auto deserialized = WitnessRule::deserialize(reader);
        
        REQUIRE(deserialized->getAction() == WitnessRuleAction::ALLOW);
        REQUIRE(deserialized->getCondition() != nullptr);
        REQUIRE(deserialized->getCondition()->getType() == WitnessConditionType::BOOLEAN);
    }
    
    SECTION("WitnessRule serialization with complex condition") {
        // Create a complex nested condition: NOT(AND(boolean(true), calledByEntry()))
        std::vector<SharedPtr<WitnessCondition>> andConditions = {
            WitnessCondition::boolean(true),
            WitnessCondition::calledByEntry()
        };
        auto andCondition = WitnessCondition::andCondition(andConditions);
        auto notCondition = WitnessCondition::notCondition(andCondition);
        
        WitnessRule original(WitnessRuleAction::DENY, notCondition);
        
        BinaryWriter writer;
        original.serialize(writer);
        Bytes serialized = writer.toArray();
        
        BinaryReader reader(serialized);
        auto deserialized = WitnessRule::deserialize(reader);
        
        REQUIRE(deserialized->getAction() == WitnessRuleAction::DENY);
        REQUIRE(deserialized->getCondition()->getType() == WitnessConditionType::NOT);
    }
    
    SECTION("WitnessRule JSON serialization") {
        auto condition = WitnessCondition::boolean(true);
        WitnessRule rule(WitnessRuleAction::ALLOW, condition);
        
        nlohmann::json json = rule.toJson();
        REQUIRE(json["action"] == "Allow");
        
        WitnessRule denyRule(WitnessRuleAction::DENY, condition);
        nlohmann::json denyJson = denyRule.toJson();
        REQUIRE(denyJson["action"] == "Deny");
    }
    
    SECTION("WitnessRule with OR condition containing multiple types") {
        Hash160 scriptHash("1234567890123456789012345678901234567890");
        std::vector<SharedPtr<WitnessCondition>> orConditions = {
            WitnessCondition::boolean(false),
            WitnessCondition::scriptHash(scriptHash),
            WitnessCondition::calledByEntry()
        };
        auto orCondition = WitnessCondition::orCondition(orConditions);
        
        WitnessRule rule(WitnessRuleAction::ALLOW, orCondition);
        
        BinaryWriter writer;
        rule.serialize(writer);
        Bytes serialized = writer.toArray();
        
        BinaryReader reader(serialized);
        auto deserialized = WitnessRule::deserialize(reader);
        
        REQUIRE(deserialized->getAction() == WitnessRuleAction::ALLOW);
        REQUIRE(deserialized->getCondition()->getType() == WitnessConditionType::OR);
    }
    
    SECTION("WitnessRule deeply nested conditions") {
        // Create: NOT(OR(boolean(true), AND(calledByEntry(), boolean(false))))
        std::vector<SharedPtr<WitnessCondition>> innerAndConditions = {
            WitnessCondition::calledByEntry(),
            WitnessCondition::boolean(false)
        };
        auto innerAnd = WitnessCondition::andCondition(innerAndConditions);
        
        std::vector<SharedPtr<WitnessCondition>> orConditions = {
            WitnessCondition::boolean(true),
            innerAnd
        };
        auto orCondition = WitnessCondition::orCondition(orConditions);
        auto notCondition = WitnessCondition::notCondition(orCondition);
        
        WitnessRule rule(WitnessRuleAction::DENY, notCondition);
        
        BinaryWriter writer;
        rule.serialize(writer);
        Bytes serialized = writer.toArray();
        
        BinaryReader reader(serialized);
        auto deserialized = WitnessRule::deserialize(reader);
        
        REQUIRE(deserialized->getAction() == WitnessRuleAction::DENY);
        REQUIRE(deserialized->getCondition()->getType() == WitnessConditionType::NOT);
        
        // Verify the serialized size is reasonable for the complex structure
        REQUIRE(serialized.size() > 10); // Should be reasonably large for nested structure
        REQUIRE(serialized.size() < 100); // But not excessively large
    }
    
    SECTION("WitnessRule with empty AND/OR conditions") {
        // Test AND condition with empty vector
        std::vector<SharedPtr<WitnessCondition>> emptyConditions;
        auto emptyAndCondition = WitnessCondition::andCondition(emptyConditions);
        
        WitnessRule rule(WitnessRuleAction::ALLOW, emptyAndCondition);
        
        BinaryWriter writer;
        rule.serialize(writer);
        Bytes serialized = writer.toArray();
        
        BinaryReader reader(serialized);
        auto deserialized = WitnessRule::deserialize(reader);
        
        REQUIRE(deserialized->getAction() == WitnessRuleAction::ALLOW);
        REQUIRE(deserialized->getCondition()->getType() == WitnessConditionType::AND);
    }
}