#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include "neocpp/types/types.hpp"

namespace neocpp {

/// Binary writer for Neo serialization
class BinaryWriter {
private:
    std::vector<uint8_t> buffer_;
    
public:
    BinaryWriter() = default;
    ~BinaryWriter() = default;
    
    /// Write a single byte
    void writeByte(uint8_t value);
    
    /// Write a boolean value
    void writeBool(bool value);
    
    /// Write bytes
    void writeBytes(const Bytes& bytes);
    void writeBytes(const uint8_t* data, size_t length);
    
    /// Write integers (little-endian)
    void writeInt8(int8_t value);
    void writeUInt8(uint8_t value);
    void writeInt16(int16_t value);
    void writeUInt16(uint16_t value);
    void writeInt32(int32_t value);
    void writeUInt32(uint32_t value);
    void writeInt64(int64_t value);
    void writeUInt64(uint64_t value);
    
    /// Write variable length integer
    void writeVarInt(uint64_t value);
    
    /// Write variable length bytes
    void writeVarBytes(const Bytes& bytes);
    
    /// Write variable length string
    void writeVarString(const std::string& str);
    
    /// Write fixed length string
    void writeFixedString(const std::string& str, size_t length);
    
    /// Write a serializable object
    template<typename T>
    void writeSerializable(const T& obj) {
        obj.serialize(*this);
    }
    
    /// Write an array of serializable objects
    template<typename T>
    void writeSerializableArray(const std::vector<T>& array) {
        writeVarInt(array.size());
        for (const auto& item : array) {
            item.serialize(*this);
        }
    }
    
    /// Get the written bytes
    const Bytes& toArray() const { return buffer_; }
    
    /// Get the current size of the buffer
    size_t size() const { return buffer_.size(); }
    
    /// Clear the buffer
    void clear() { buffer_.clear(); }
    
    /// Reserve capacity
    void reserve(size_t capacity) { buffer_.reserve(capacity); }
};

} // namespace neocpp