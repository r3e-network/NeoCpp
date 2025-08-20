#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>
#include "neocpp/types/types.hpp"

namespace neocpp {

/// Binary reader for Neo deserialization
class BinaryReader {
private:
    const uint8_t* data_;
    size_t size_;
    size_t position_;
    
public:
    /// Constructor from byte array
    BinaryReader(const Bytes& data);
    BinaryReader(const uint8_t* data, size_t size);
    
    ~BinaryReader() = default;
    
    /// Read a single byte
    uint8_t readByte();
    
    /// Read a boolean value
    bool readBool();
    
    /// Read bytes
    Bytes readBytes(size_t count);
    void readBytes(uint8_t* buffer, size_t count);
    
    /// Read integers (little-endian)
    int8_t readInt8();
    uint8_t readUInt8();
    int16_t readInt16();
    uint16_t readUInt16();
    int32_t readInt32();
    uint32_t readUInt32();
    int64_t readInt64();
    uint64_t readUInt64();
    
    /// Read variable length integer
    uint64_t readVarInt();
    
    /// Read variable length bytes
    Bytes readVarBytes();
    
    /// Read variable length string
    std::string readVarString();
    
    /// Read fixed length string
    std::string readFixedString(size_t length);
    
    /// Read a deserializable object
    template<typename T>
    T readSerializable() {
        return T::deserialize(*this);
    }
    
    /// Read an array of deserializable objects
    template<typename T>
    std::vector<T> readSerializableArray() {
        uint64_t count = readVarInt();
        std::vector<T> result;
        result.reserve(count);
        for (uint64_t i = 0; i < count; ++i) {
            result.push_back(T::deserialize(*this));
        }
        return result;
    }
    
    /// Check if more data is available
    bool hasMore() const { return position_ < size_; }
    
    /// Get remaining bytes count
    size_t remaining() const { return size_ - position_; }
    
    /// Get current position
    size_t position() const { return position_; }
    
    /// Skip bytes
    void skip(size_t count);
    
    /// Reset position to beginning
    void reset() { position_ = 0; }
    
    /// Seek to specific position
    void seek(size_t position);
};

} // namespace neocpp