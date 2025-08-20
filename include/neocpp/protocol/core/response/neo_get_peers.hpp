#pragma once

#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "neocpp/types/types.hpp"

namespace neocpp {

/// NeoGetPeers response type
class NeoGetPeers {
private:
    nlohmann::json data_;
    
public:
    /// Constructor
    NeoGetPeers() = default;
    
    /// Constructor from JSON
    explicit NeoGetPeers(const nlohmann::json& json) : data_(json) {}
    
    /// Get raw JSON data
    const nlohmann::json& getRawJson() const { return data_; }
    
    /// Convert to JSON
    nlohmann::json toJson() const { return data_; }
    
    /// Create from JSON
    static SharedPtr<NeoGetPeers> fromJson(const nlohmann::json& json) {
        return std::make_shared<NeoGetPeers>(json);
    }
};

} // namespace neocpp
