#include "neocpp/types/contract_parameter.hpp"
#include "neocpp/exceptions.hpp"

namespace neocpp {

ContractParameter::ContractParameter(ContractParameterType type) : type_(type) {
}

ContractParameter ContractParameter::boolean(bool value) {
    ContractParameter param(ContractParameterType::BOOLEAN);
    param.value_ = value;
    return param;
}

ContractParameter ContractParameter::integer(int64_t value) {
    ContractParameter param(ContractParameterType::INTEGER);
    param.value_ = value;
    return param;
}

ContractParameter ContractParameter::byteArray(const Bytes& value) {
    ContractParameter param(ContractParameterType::BYTE_ARRAY);
    param.value_ = value;
    return param;
}

ContractParameter ContractParameter::string(const std::string& value) {
    ContractParameter param(ContractParameterType::STRING);
    param.value_ = value;
    return param;
}

ContractParameter ContractParameter::hash160(const Hash160& value) {
    ContractParameter param(ContractParameterType::HASH160);
    param.value_ = value;
    return param;
}

ContractParameter ContractParameter::hash256(const Hash256& value) {
    ContractParameter param(ContractParameterType::HASH256);
    param.value_ = value;
    return param;
}

ContractParameter ContractParameter::publicKey(const SharedPtr<ECPublicKey>& value) {
    ContractParameter param(ContractParameterType::PUBLIC_KEY);
    param.value_ = value;
    return param;
}

ContractParameter ContractParameter::signature(const Bytes& value) {
    ContractParameter param(ContractParameterType::SIGNATURE);
    param.value_ = value;
    return param;
}

ContractParameter ContractParameter::array(const std::vector<ContractParameter>& value) {
    ContractParameter param(ContractParameterType::ARRAY);
    param.value_ = value;
    return param;
}

ContractParameter ContractParameter::map(const std::map<ContractParameter, ContractParameter>& value) {
    ContractParameter param(ContractParameterType::MAP);
    param.value_ = value;
    return param;
}

bool ContractParameter::getBoolean() const {
    if (type_ != ContractParameterType::BOOLEAN) {
        throw IllegalArgumentException("Parameter is not a boolean");
    }
    return std::get<bool>(value_);
}

int64_t ContractParameter::getInteger() const {
    if (type_ != ContractParameterType::INTEGER) {
        throw IllegalArgumentException("Parameter is not an integer");
    }
    return std::get<int64_t>(value_);
}

Bytes ContractParameter::getByteArray() const {
    if (type_ != ContractParameterType::BYTE_ARRAY && type_ != ContractParameterType::SIGNATURE) {
        throw IllegalArgumentException("Parameter is not a byte array");
    }
    return std::get<Bytes>(value_);
}

std::string ContractParameter::getString() const {
    if (type_ != ContractParameterType::STRING) {
        throw IllegalArgumentException("Parameter is not a string");
    }
    return std::get<std::string>(value_);
}

Hash160 ContractParameter::getHash160() const {
    if (type_ != ContractParameterType::HASH160) {
        throw IllegalArgumentException("Parameter is not a Hash160");
    }
    return std::get<Hash160>(value_);
}

Hash256 ContractParameter::getHash256() const {
    if (type_ != ContractParameterType::HASH256) {
        throw IllegalArgumentException("Parameter is not a Hash256");
    }
    return std::get<Hash256>(value_);
}

SharedPtr<ECPublicKey> ContractParameter::getPublicKey() const {
    if (type_ != ContractParameterType::PUBLIC_KEY) {
        throw IllegalArgumentException("Parameter is not a public key");
    }
    return std::get<SharedPtr<ECPublicKey>>(value_);
}

std::vector<ContractParameter> ContractParameter::getArray() const {
    if (type_ != ContractParameterType::ARRAY) {
        throw IllegalArgumentException("Parameter is not an array");
    }
    return std::get<std::vector<ContractParameter>>(value_);
}

std::map<ContractParameter, ContractParameter> ContractParameter::getMap() const {
    if (type_ != ContractParameterType::MAP) {
        throw IllegalArgumentException("Parameter is not a map");
    }
    return std::get<std::map<ContractParameter, ContractParameter>>(value_);
}

bool ContractParameter::operator<(const ContractParameter& other) const {
    if (type_ != other.type_) {
        return type_ < other.type_;
    }
    return value_ < other.value_;
}

bool ContractParameter::operator==(const ContractParameter& other) const {
    return type_ == other.type_ && value_ == other.value_;
}

} // namespace neocpp