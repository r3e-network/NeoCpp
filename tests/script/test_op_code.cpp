#include <catch2/catch_test_macros.hpp>
#include "neocpp/script/op_code.hpp"
#include "neocpp/script/script_builder.hpp"
#include <string>

using namespace neocpp;

TEST_CASE("OpCode Tests", "[script]") {
    
    SECTION("OpCode byte conversion") {
        // Test conversion to/from byte
        REQUIRE(OpCodeHelper::toByte(OpCode::PUSH0) == 0x10);
        REQUIRE(OpCodeHelper::toByte(OpCode::PUSH1) == 0x11);
        REQUIRE(OpCodeHelper::toByte(OpCode::NOP) == 0x21);
        REQUIRE(OpCodeHelper::toByte(OpCode::SYSCALL) == 0x41);
        REQUIRE(OpCodeHelper::toByte(OpCode::DUP) == 0x4A);
        REQUIRE(OpCodeHelper::toByte(OpCode::ADD) == 0x9E);
        
        // Test reverse conversion
        REQUIRE(OpCodeHelper::fromByte(0x10) == OpCode::PUSH0);
        REQUIRE(OpCodeHelper::fromByte(0x11) == OpCode::PUSH1);
        REQUIRE(OpCodeHelper::fromByte(0x21) == OpCode::NOP);
        REQUIRE(OpCodeHelper::fromByte(0x41) == OpCode::SYSCALL);
    }
    
    SECTION("Push opcodes range") {
        // PUSH0 to PUSH16 should be consecutive
        uint8_t push0 = OpCodeHelper::toByte(OpCode::PUSH0);
        uint8_t push1 = OpCodeHelper::toByte(OpCode::PUSH1);
        uint8_t push16 = OpCodeHelper::toByte(OpCode::PUSH16);
        
        REQUIRE(push1 == push0 + 1);
        REQUIRE(push16 == push0 + 16);
        
        // Check all PUSH opcodes
        for (int i = 0; i <= 16; i++) {
            OpCode pushOp = OpCodeHelper::fromByte(push0 + i);
            if (OpCodeHelper::isPush(pushOp)) {
                int value = OpCodeHelper::getPushValue(pushOp);
                REQUIRE(value == i);
            }
        }
    }
    
    SECTION("Special push opcodes") {
        REQUIRE(OpCodeHelper::toByte(OpCode::PUSHM1) == 0x0F);  // Push -1
        REQUIRE(OpCodeHelper::toByte(OpCode::PUSHNULL) == 0x0B);
        REQUIRE(OpCodeHelper::toByte(OpCode::PUSHDATA1) == 0x0C);
        REQUIRE(OpCodeHelper::toByte(OpCode::PUSHDATA2) == 0x0D);
        REQUIRE(OpCodeHelper::toByte(OpCode::PUSHDATA4) == 0x0E);
        
        // Integer push opcodes
        REQUIRE(OpCodeHelper::toByte(OpCode::PUSHINT8) == 0x00);
        REQUIRE(OpCodeHelper::toByte(OpCode::PUSHINT16) == 0x01);
        REQUIRE(OpCodeHelper::toByte(OpCode::PUSHINT32) == 0x02);
        REQUIRE(OpCodeHelper::toByte(OpCode::PUSHINT64) == 0x03);
        REQUIRE(OpCodeHelper::toByte(OpCode::PUSHINT128) == 0x04);
        REQUIRE(OpCodeHelper::toByte(OpCode::PUSHINT256) == 0x05);
    }
    
    SECTION("Flow control opcodes") {
        // Jump opcodes
        REQUIRE(OpCodeHelper::toByte(OpCode::JMP) == 0x22);
        REQUIRE(OpCodeHelper::toByte(OpCode::JMP_L) == 0x23);
        REQUIRE(OpCodeHelper::toByte(OpCode::JMPIF) == 0x24);
        REQUIRE(OpCodeHelper::toByte(OpCode::JMPIF_L) == 0x25);
        REQUIRE(OpCodeHelper::toByte(OpCode::JMPIFNOT) == 0x26);
        REQUIRE(OpCodeHelper::toByte(OpCode::JMPIFNOT_L) == 0x27);
        
        // Call and return
        REQUIRE(OpCodeHelper::toByte(OpCode::CALL) == 0x34);
        REQUIRE(OpCodeHelper::toByte(OpCode::CALL_L) == 0x35);
        REQUIRE(OpCodeHelper::toByte(OpCode::RET) == 0x40);
        REQUIRE(OpCodeHelper::toByte(OpCode::SYSCALL) == 0x41);
        
        // Exception handling
        REQUIRE(OpCodeHelper::toByte(OpCode::ABORT) == 0x38);
        REQUIRE(OpCodeHelper::toByte(OpCode::ASSERT) == 0x39);
        REQUIRE(OpCodeHelper::toByte(OpCode::THROW) == 0x3A);
    }
    
    SECTION("Stack manipulation opcodes") {
        REQUIRE(OpCodeHelper::toByte(OpCode::DEPTH) == 0x43);
        REQUIRE(OpCodeHelper::toByte(OpCode::DROP) == 0x45);
        REQUIRE(OpCodeHelper::toByte(OpCode::DUP) == 0x4A);
        REQUIRE(OpCodeHelper::toByte(OpCode::OVER) == 0x4B);
        REQUIRE(OpCodeHelper::toByte(OpCode::PICK) == 0x4D);
        REQUIRE(OpCodeHelper::toByte(OpCode::SWAP) == 0x50);
        REQUIRE(OpCodeHelper::toByte(OpCode::ROT) == 0x51);
        REQUIRE(OpCodeHelper::toByte(OpCode::CLEAR) == 0x49);
    }
    
    SECTION("Arithmetic opcodes") {
        REQUIRE(OpCodeHelper::toByte(OpCode::ADD) == 0x9E);
        REQUIRE(OpCodeHelper::toByte(OpCode::SUB) == 0x9F);
        REQUIRE(OpCodeHelper::toByte(OpCode::MUL) == 0xA0);
        REQUIRE(OpCodeHelper::toByte(OpCode::DIV) == 0xA1);
        REQUIRE(OpCodeHelper::toByte(OpCode::MOD) == 0xA2);
        REQUIRE(OpCodeHelper::toByte(OpCode::POW) == 0xA3);
        REQUIRE(OpCodeHelper::toByte(OpCode::SQRT) == 0xA4);
        REQUIRE(OpCodeHelper::toByte(OpCode::ABS) == 0x9A);
        REQUIRE(OpCodeHelper::toByte(OpCode::NEGATE) == 0x9B);
        REQUIRE(OpCodeHelper::toByte(OpCode::INC) == 0x9C);
        REQUIRE(OpCodeHelper::toByte(OpCode::DEC) == 0x9D);
    }
    
    SECTION("Logical opcodes") {
        REQUIRE(OpCodeHelper::toByte(OpCode::AND) == 0x91);
        REQUIRE(OpCodeHelper::toByte(OpCode::OR) == 0x92);
        REQUIRE(OpCodeHelper::toByte(OpCode::XOR) == 0x93);
        REQUIRE(OpCodeHelper::toByte(OpCode::NOT) == 0xAA);
        REQUIRE(OpCodeHelper::toByte(OpCode::BOOLAND) == 0xAB);
        REQUIRE(OpCodeHelper::toByte(OpCode::BOOLOR) == 0xAC);
        REQUIRE(OpCodeHelper::toByte(OpCode::INVERT) == 0x90);
    }
    
    SECTION("Comparison opcodes") {
        REQUIRE(OpCodeHelper::toByte(OpCode::EQUAL) == 0x97);
        REQUIRE(OpCodeHelper::toByte(OpCode::NOTEQUAL) == 0x98);
        REQUIRE(OpCodeHelper::toByte(OpCode::LT) == 0xB5);
        REQUIRE(OpCodeHelper::toByte(OpCode::LE) == 0xB6);
        REQUIRE(OpCodeHelper::toByte(OpCode::GT) == 0xB7);
        REQUIRE(OpCodeHelper::toByte(OpCode::GE) == 0xB8);
        REQUIRE(OpCodeHelper::toByte(OpCode::MIN) == 0xB9);
        REQUIRE(OpCodeHelper::toByte(OpCode::MAX) == 0xBA);
        REQUIRE(OpCodeHelper::toByte(OpCode::WITHIN) == 0xBB);
    }
    
    SECTION("Array/Struct opcodes") {
        REQUIRE(OpCodeHelper::toByte(OpCode::NEWARRAY0) == 0xC2);
        REQUIRE(OpCodeHelper::toByte(OpCode::NEWARRAY) == 0xC3);
        REQUIRE(OpCodeHelper::toByte(OpCode::NEWARRAY_T) == 0xC4);
        REQUIRE(OpCodeHelper::toByte(OpCode::NEWSTRUCT0) == 0xC5);
        REQUIRE(OpCodeHelper::toByte(OpCode::NEWSTRUCT) == 0xC6);
        REQUIRE(OpCodeHelper::toByte(OpCode::NEWMAP) == 0xC8);
        REQUIRE(OpCodeHelper::toByte(OpCode::PACK) == 0xC0);
        REQUIRE(OpCodeHelper::toByte(OpCode::UNPACK) == 0xC1);
        REQUIRE(OpCodeHelper::toByte(OpCode::APPEND) == 0xCF);
        REQUIRE(OpCodeHelper::toByte(OpCode::SETITEM) == 0xD0);
        REQUIRE(OpCodeHelper::toByte(OpCode::PICKITEM) == 0xCE);
    }
    
    SECTION("Slot opcodes") {
        // Local variable slots
        REQUIRE(OpCodeHelper::toByte(OpCode::LDLOC0) == 0x68);
        REQUIRE(OpCodeHelper::toByte(OpCode::LDLOC1) == 0x69);
        REQUIRE(OpCodeHelper::toByte(OpCode::LDLOC6) == 0x6E);
        REQUIRE(OpCodeHelper::toByte(OpCode::LDLOC) == 0x6F);
        
        REQUIRE(OpCodeHelper::toByte(OpCode::STLOC0) == 0x70);
        REQUIRE(OpCodeHelper::toByte(OpCode::STLOC1) == 0x71);
        REQUIRE(OpCodeHelper::toByte(OpCode::STLOC6) == 0x76);
        REQUIRE(OpCodeHelper::toByte(OpCode::STLOC) == 0x77);
        
        // Argument slots
        REQUIRE(OpCodeHelper::toByte(OpCode::LDARG0) == 0x78);
        REQUIRE(OpCodeHelper::toByte(OpCode::LDARG6) == 0x7E);
        REQUIRE(OpCodeHelper::toByte(OpCode::LDARG) == 0x7F);
        
        REQUIRE(OpCodeHelper::toByte(OpCode::STARG0) == 0x80);
        REQUIRE(OpCodeHelper::toByte(OpCode::STARG6) == 0x86);
        REQUIRE(OpCodeHelper::toByte(OpCode::STARG) == 0x87);
        
        // Static field slots
        REQUIRE(OpCodeHelper::toByte(OpCode::LDSFLD0) == 0x58);
        REQUIRE(OpCodeHelper::toByte(OpCode::LDSFLD6) == 0x5E);
        REQUIRE(OpCodeHelper::toByte(OpCode::LDSFLD) == 0x5F);
        
        REQUIRE(OpCodeHelper::toByte(OpCode::STSFLD0) == 0x60);
        REQUIRE(OpCodeHelper::toByte(OpCode::STSFLD6) == 0x66);
        REQUIRE(OpCodeHelper::toByte(OpCode::STSFLD) == 0x67);
    }
    
    SECTION("Type checking opcodes") {
        REQUIRE(OpCodeHelper::toByte(OpCode::ISNULL) == 0xD8);
        REQUIRE(OpCodeHelper::toByte(OpCode::ISTYPE) == 0xD9);
        REQUIRE(OpCodeHelper::toByte(OpCode::CONVERT) == 0xDB);
    }
    
    SECTION("OpCode in ScriptBuilder") {
        ScriptBuilder builder;
        
        // Test emitting various opcodes
        builder.emit(OpCode::NOP);
        builder.emit(OpCode::PUSH1);
        builder.emit(OpCode::PUSH2);
        builder.emit(OpCode::ADD);
        
        Bytes script = builder.toArray();
        REQUIRE(script.size() == 4);
        REQUIRE(script[0] == OpCodeHelper::toByte(OpCode::NOP));
        REQUIRE(script[1] == OpCodeHelper::toByte(OpCode::PUSH1));
        REQUIRE(script[2] == OpCodeHelper::toByte(OpCode::PUSH2));
        REQUIRE(script[3] == OpCodeHelper::toByte(OpCode::ADD));
    }
    
    SECTION("OpCode operand sizes") {
        // Test opcodes with different operand sizes
        REQUIRE(OpCodeHelper::getOperandSize(OpCode::NOP) == 0);
        REQUIRE(OpCodeHelper::getOperandSize(OpCode::PUSH1) == 0);
        REQUIRE(OpCodeHelper::getOperandSize(OpCode::PUSHINT8) == 1);
        REQUIRE(OpCodeHelper::getOperandSize(OpCode::PUSHINT16) == 2);
        REQUIRE(OpCodeHelper::getOperandSize(OpCode::PUSHINT32) == 4);
        REQUIRE(OpCodeHelper::getOperandSize(OpCode::JMP) == 1);  // 1-byte offset
        REQUIRE(OpCodeHelper::getOperandSize(OpCode::JMP_L) == 4); // 4-byte offset
        REQUIRE(OpCodeHelper::getOperandSize(OpCode::PUSHDATA1) == 1); // 1-byte length prefix
        REQUIRE(OpCodeHelper::getOperandSize(OpCode::PUSHDATA2) == 2); // 2-byte length prefix
        REQUIRE(OpCodeHelper::getOperandSize(OpCode::PUSHDATA4) == 4); // 4-byte length prefix
        REQUIRE(OpCodeHelper::getOperandSize(OpCode::SYSCALL) == 4); // 4-byte interop hash
    }
}