#ifndef CUSTOM_INTERFACE_PC_CUSTOM_SRVS_Z_SETCOLOR_H
#define CUSTOM_INTERFACE_PC_CUSTOM_SRVS_Z_SETCOLOR_H

#include <nlohmann/json.hpp>
#include <vector>
#include <string>

namespace custom_srvs {
struct z_SetColor {
    struct Request {
        int32_t r;
        int32_t g;
        int32_t b;
    };

    struct Response {
        bool success;
        std::string message;
    };
};
} // namespace custom_srvs

// --- Service Request Serializer ---
template <>
inline std::vector<uint8_t> serialize_msg<custom_srvs::z_SetColor::Request>(
    const custom_srvs::z_SetColor::Request& msg) {
    nlohmann::json j;
    j["r"] = msg.r;
    j["g"] = msg.g;
    j["b"] = msg.b;
    return nlohmann::json::to_msgpack(j);
}

// --- Service Request Deserializer ---
template <>
inline void deserialize_msg<custom_srvs::z_SetColor::Request>(
    const std::vector<uint8_t>& buffer, custom_srvs::z_SetColor::Request& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.r = j["r"].get<int32_t>();
    msg.g = j["g"].get<int32_t>();
    msg.b = j["b"].get<int32_t>();
}

// --- Service Response Serializer ---
template <>
inline std::vector<uint8_t> serialize_msg<custom_srvs::z_SetColor::Response>(
    const custom_srvs::z_SetColor::Response& msg) {
    nlohmann::json j;
    j["success"] = msg.success;
    j["message"] = msg.message;
    return nlohmann::json::to_msgpack(j);
}

// --- Service Response Deserializer ---
template <>
inline void deserialize_msg<custom_srvs::z_SetColor::Response>(
    const std::vector<uint8_t>& buffer, custom_srvs::z_SetColor::Response& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.success = j["success"].get<bool>();
    msg.message = j["message"].get<std::string>();
}

using z_SetColor = custom_srvs::z_SetColor;

#endif // CUSTOM_INTERFACE_PC_CUSTOM_SRVS_Z_SETCOLOR_H
