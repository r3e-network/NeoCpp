#pragma once

#include <cstdint>
#include <string>

namespace neocpp {

/// Neo VM operation codes
enum class OpCode : uint8_t {
    // Constants
    PUSHINT8 = 0x00,
    PUSHINT16 = 0x01,
    PUSHINT32 = 0x02,
    PUSHINT64 = 0x03,
    PUSHINT128 = 0x04,
    PUSHINT256 = 0x05,
    PUSHA = 0x0A,
    PUSHNULL = 0x0B,
    PUSHDATA1 = 0x0C,
    PUSHDATA2 = 0x0D,
    PUSHDATA4 = 0x0E,
    PUSHM1 = 0x0F,
    PUSH0 = 0x10,
    PUSH1 = 0x11,
    PUSH2 = 0x12,
    PUSH3 = 0x13,
    PUSH4 = 0x14,
    PUSH5 = 0x15,
    PUSH6 = 0x16,
    PUSH7 = 0x17,
    PUSH8 = 0x18,
    PUSH9 = 0x19,
    PUSH10 = 0x1A,
    PUSH11 = 0x1B,
    PUSH12 = 0x1C,
    PUSH13 = 0x1D,
    PUSH14 = 0x1E,
    PUSH15 = 0x1F,
    PUSH16 = 0x20,
    
    // Flow control
    NOP = 0x21,
    JMP = 0x22,
    JMP_L = 0x23,
    JMPIF = 0x24,
    JMPIF_L = 0x25,
    JMPIFNOT = 0x26,
    JMPIFNOT_L = 0x27,
    JMPEQ = 0x28,
    JMPEQ_L = 0x29,
    JMPNE = 0x2A,
    JMPNE_L = 0x2B,
    JMPGT = 0x2C,
    JMPGT_L = 0x2D,
    JMPGE = 0x2E,
    JMPGE_L = 0x2F,
    JMPLT = 0x30,
    JMPLT_L = 0x31,
    JMPLE = 0x32,
    JMPLE_L = 0x33,
    CALL = 0x34,
    CALL_L = 0x35,
    CALLA = 0x36,
    CALLT = 0x37,
    ABORT = 0x38,
    ASSERT = 0x39,
    THROW = 0x3A,
    TRY = 0x3B,
    TRY_L = 0x3C,
    ENDTRY = 0x3D,
    ENDTRY_L = 0x3E,
    ENDFINALLY = 0x3F,
    RET = 0x40,
    SYSCALL = 0x41,
    
    // Stack
    DEPTH = 0x43,
    DROP = 0x45,
    NIP = 0x46,
    XDROP = 0x48,
    CLEAR = 0x49,
    DUP = 0x4A,
    OVER = 0x4B,
    PICK = 0x4D,
    TUCK = 0x4E,
    SWAP = 0x50,
    ROT = 0x51,
    ROLL = 0x52,
    REVERSE3 = 0x53,
    REVERSE4 = 0x54,
    REVERSEN = 0x55,
    
    // Slot
    INITSSLOT = 0x56,
    INITSLOT = 0x57,
    LDSFLD0 = 0x58,
    LDSFLD1 = 0x59,
    LDSFLD2 = 0x5A,
    LDSFLD3 = 0x5B,
    LDSFLD4 = 0x5C,
    LDSFLD5 = 0x5D,
    LDSFLD6 = 0x5E,
    LDSFLD = 0x5F,
    STSFLD0 = 0x60,
    STSFLD1 = 0x61,
    STSFLD2 = 0x62,
    STSFLD3 = 0x63,
    STSFLD4 = 0x64,
    STSFLD5 = 0x65,
    STSFLD6 = 0x66,
    STSFLD = 0x67,
    LDLOC0 = 0x68,
    LDLOC1 = 0x69,
    LDLOC2 = 0x6A,
    LDLOC3 = 0x6B,
    LDLOC4 = 0x6C,
    LDLOC5 = 0x6D,
    LDLOC6 = 0x6E,
    LDLOC = 0x6F,
    STLOC0 = 0x70,
    STLOC1 = 0x71,
    STLOC2 = 0x72,
    STLOC3 = 0x73,
    STLOC4 = 0x74,
    STLOC5 = 0x75,
    STLOC6 = 0x76,
    STLOC = 0x77,
    LDARG0 = 0x78,
    LDARG1 = 0x79,
    LDARG2 = 0x7A,
    LDARG3 = 0x7B,
    LDARG4 = 0x7C,
    LDARG5 = 0x7D,
    LDARG6 = 0x7E,
    LDARG = 0x7F,
    STARG0 = 0x80,
    STARG1 = 0x81,
    STARG2 = 0x82,
    STARG3 = 0x83,
    STARG4 = 0x84,
    STARG5 = 0x85,
    STARG6 = 0x86,
    STARG = 0x87,
    
    // Compound
    NEWBUFFER = 0x88,
    MEMCPY = 0x89,
    CAT = 0x8B,
    SUBSTR = 0x8C,
    LEFT = 0x8D,
    RIGHT = 0x8E,
    
    // Arithmetic
    INVERT = 0x90,
    AND = 0x91,
    OR = 0x92,
    XOR = 0x93,
    EQUAL = 0x97,
    NOTEQUAL = 0x98,
    SIGN = 0x99,
    ABS = 0x9A,
    NEGATE = 0x9B,
    INC = 0x9C,
    DEC = 0x9D,
    ADD = 0x9E,
    SUB = 0x9F,
    MUL = 0xA0,
    DIV = 0xA1,
    MOD = 0xA2,
    POW = 0xA3,
    SQRT = 0xA4,
    SHL = 0xA8,
    SHR = 0xA9,
    NOT = 0xAA,
    BOOLAND = 0xAB,
    BOOLOR = 0xAC,
    NZ = 0xB1,
    NUMEQUAL = 0xB3,
    NUMNOTEQUAL = 0xB4,
    LT = 0xB5,
    LE = 0xB6,
    GT = 0xB7,
    GE = 0xB8,
    MIN = 0xB9,
    MAX = 0xBA,
    WITHIN = 0xBB,
    
    // Compound
    PACKMAP = 0xBE,
    PACKSTRUCT = 0xBF,
    PACK = 0xC0,
    UNPACK = 0xC1,
    NEWARRAY0 = 0xC2,
    NEWARRAY = 0xC3,
    NEWARRAY_T = 0xC4,
    NEWSTRUCT0 = 0xC5,
    NEWSTRUCT = 0xC6,
    NEWMAP = 0xC8,
    SIZE = 0xCA,
    HASKEY = 0xCB,
    KEYS = 0xCC,
    VALUES = 0xCD,
    PICKITEM = 0xCE,
    APPEND = 0xCF,
    SETITEM = 0xD0,
    REVERSEITEMS = 0xD1,
    REMOVE = 0xD2,
    CLEARITEMS = 0xD3,
    POPITEM = 0xD4,
    
    // Type
    ISNULL = 0xD8,
    ISTYPE = 0xD9,
    CONVERT = 0xDB
};

/// Helper class for OpCode operations
class OpCodeHelper {
public:
    /// Convert OpCode to byte value
    static uint8_t toByte(OpCode opcode) {
        return static_cast<uint8_t>(opcode);
    }
    
    /// Convert byte value to OpCode
    static OpCode fromByte(uint8_t value) {
        return static_cast<OpCode>(value);
    }
    
    /// Get the name of an OpCode
    static std::string getName(OpCode opcode);
    
    /// Get the size of the OpCode operand
    static int getOperandSize(OpCode opcode);
    
    /// Check if OpCode is a push operation
    static bool isPush(OpCode opcode);
    
    /// Get the push value for simple push opcodes
    static int getPushValue(OpCode opcode);
};

} // namespace neocpp