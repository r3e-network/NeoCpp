#include "neocpp/serialization/binary_reader.hpp"
#include "neocpp/exceptions.hpp"
#include <cstring>

namespace neocpp {

BinaryReader::BinaryReader(const Bytes& data) 
    : data_(data.data()), size_(data.size()), position_(0) {
}

BinaryReader::BinaryReader(const uint8_t* data, size_t size)
    : data_(data), size_(size), position_(0) {
}

uint8_t BinaryReader::readByte() {
    if (position_ >= size_) {
        throw DeserializationException("Attempted to read beyond end of data");
    }
    return data_[position_++];
}

bool BinaryReader::readBool() {
    return readByte() != 0;
}

Bytes BinaryReader::readBytes(size_t count) {
    if (position_ + count > size_) {
        throw DeserializationException("Attempted to read beyond end of data");
    }
    Bytes result(data_ + position_, data_ + position_ + count);
    position_ += count;
    return result;
}

void BinaryReader::readBytes(uint8_t* buffer, size_t count) {
    if (position_ + count > size_) {
        throw DeserializationException("Attempted to read beyond end of data");
    }
    std::memcpy(buffer, data_ + position_, count);
    position_ += count;
}

int8_t BinaryReader::readInt8() {
    return static_cast<int8_t>(readByte());
}

uint8_t BinaryReader::readUInt8() {
    return readByte();
}

int16_t BinaryReader::readInt16() {
    uint8_t b0 = readByte();
    uint8_t b1 = readByte();
    return static_cast<int16_t>(b0 | (b1 << 8));
}

uint16_t BinaryReader::readUInt16() {
    uint8_t b0 = readByte();
    uint8_t b1 = readByte();
    return static_cast<uint16_t>(b0 | (b1 << 8));
}

int32_t BinaryReader::readInt32() {
    uint8_t b0 = readByte();
    uint8_t b1 = readByte();
    uint8_t b2 = readByte();
    uint8_t b3 = readByte();
    return static_cast<int32_t>(b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
}

uint32_t BinaryReader::readUInt32() {
    uint8_t b0 = readByte();
    uint8_t b1 = readByte();
    uint8_t b2 = readByte();
    uint8_t b3 = readByte();
    return static_cast<uint32_t>(b0 | (b1 << 8) | (b2 << 16) | (b3 << 24));
}

int64_t BinaryReader::readInt64() {
    int64_t result = 0;
    for (int i = 0; i < 8; ++i) {
        result |= static_cast<int64_t>(readByte()) << (i * 8);
    }
    return result;
}

uint64_t BinaryReader::readUInt64() {
    uint64_t result = 0;
    for (int i = 0; i < 8; ++i) {
        result |= static_cast<uint64_t>(readByte()) << (i * 8);
    }
    return result;
}

uint64_t BinaryReader::readVarInt() {
    uint8_t first = readByte();
    if (first < 0xFD) {
        return first;
    } else if (first == 0xFD) {
        return readUInt16();
    } else if (first == 0xFE) {
        return readUInt32();
    } else {
        return readUInt64();
    }
}

Bytes BinaryReader::readVarBytes() {
    uint64_t length = readVarInt();
    return readBytes(static_cast<size_t>(length));
}

std::string BinaryReader::readVarString() {
    uint64_t length = readVarInt();
    Bytes bytes = readBytes(static_cast<size_t>(length));
    return std::string(bytes.begin(), bytes.end());
}

std::string BinaryReader::readFixedString(size_t length) {
    Bytes bytes = readBytes(length);
    // Find null terminator if present
    auto nullPos = std::find(bytes.begin(), bytes.end(), 0);
    if (nullPos != bytes.end()) {
        return std::string(bytes.begin(), nullPos);
    }
    return std::string(bytes.begin(), bytes.end());
}

void BinaryReader::skip(size_t count) {
    if (position_ + count > size_) {
        throw DeserializationException("Attempted to skip beyond end of data");
    }
    position_ += count;
}

void BinaryReader::seek(size_t position) {
    if (position > size_) {
        throw DeserializationException("Attempted to seek beyond end of data");
    }
    position_ = position;
}

} // namespace neocpp