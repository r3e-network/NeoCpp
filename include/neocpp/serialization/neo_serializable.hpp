#pragma once

#include <cstddef>

namespace neocpp {

// Forward declarations
class BinaryWriter;
class BinaryReader;

/// Interface for Neo serializable objects
class NeoSerializable {
public:
    virtual ~NeoSerializable() = default;
    
    /// Get the size of the serialized data in bytes
    /// @return The size in bytes
    virtual size_t getSize() const = 0;
    
    /// Serialize the object to a binary writer
    /// @param writer The binary writer to serialize to
    virtual void serialize(BinaryWriter& writer) const = 0;
};

} // namespace neocpp