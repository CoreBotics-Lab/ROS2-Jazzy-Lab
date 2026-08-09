#ifndef Z_STD_SRVS_PC_H
#define Z_STD_SRVS_PC_H

#include <nlohmann/json.hpp>
#include <vector>
#include <string>

// Forward declarations
template <typename T>
std::vector<uint8_t> serialize_msg(const T& msg);

template <typename T>
void deserialize_msg(const std::vector<uint8_t>& buffer, T& msg);

namespace z_std_srvs {
    struct z_Empty {
        struct Request {};
        struct Response {};
    };

    struct z_Trigger {
        struct Request {};
        struct Response {
            bool success;
            std::string message;
        };
    };

    struct z_SetBool {
        struct Request {
            bool data;
        };
        struct Response {
            bool success;
            std::string message;
        };
    };
} // namespace z_std_srvs

// --- z_Empty Serializers ---
template <>
inline std::vector<uint8_t> serialize_msg<z_std_srvs::z_Empty::Request>(
    const z_std_srvs::z_Empty::Request& msg) {
    (void)msg;
    nlohmann::json j;
    return nlohmann::json::to_msgpack(j);
}

template <>
inline void deserialize_msg<z_std_srvs::z_Empty::Request>(
    const std::vector<uint8_t>& buffer, z_std_srvs::z_Empty::Request& msg) {
    (void)buffer; (void)msg;
}

template <>
inline std::vector<uint8_t> serialize_msg<z_std_srvs::z_Empty::Response>(
    const z_std_srvs::z_Empty::Response& msg) {
    (void)msg;
    nlohmann::json j;
    return nlohmann::json::to_msgpack(j);
}

template <>
inline void deserialize_msg<z_std_srvs::z_Empty::Response>(
    const std::vector<uint8_t>& buffer, z_std_srvs::z_Empty::Response& msg) {
    (void)buffer; (void)msg;
}

// --- z_Trigger Serializers ---
template <>
inline std::vector<uint8_t> serialize_msg<z_std_srvs::z_Trigger::Request>(
    const z_std_srvs::z_Trigger::Request& msg) {
    (void)msg;
    nlohmann::json j;
    return nlohmann::json::to_msgpack(j);
}

template <>
inline void deserialize_msg<z_std_srvs::z_Trigger::Request>(
    const std::vector<uint8_t>& buffer, z_std_srvs::z_Trigger::Request& msg) {
    (void)buffer; (void)msg;
}

template <>
inline std::vector<uint8_t> serialize_msg<z_std_srvs::z_Trigger::Response>(
    const z_std_srvs::z_Trigger::Response& msg) {
    nlohmann::json j;
    j["success"] = msg.success;
    j["message"] = msg.message;
    return nlohmann::json::to_msgpack(j);
}

template <>
inline void deserialize_msg<z_std_srvs::z_Trigger::Response>(
    const std::vector<uint8_t>& buffer, z_std_srvs::z_Trigger::Response& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.success = j["success"].get<bool>();
    msg.message = j["message"].get<std::string>();
}

// --- z_SetBool Serializers ---
template <>
inline std::vector<uint8_t> serialize_msg<z_std_srvs::z_SetBool::Request>(
    const z_std_srvs::z_SetBool::Request& msg) {
    nlohmann::json j;
    j["data"] = msg.data;
    return nlohmann::json::to_msgpack(j);
}

template <>
inline void deserialize_msg<z_std_srvs::z_SetBool::Request>(
    const std::vector<uint8_t>& buffer, z_std_srvs::z_SetBool::Request& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.data = j["data"].get<bool>();
}

template <>
inline std::vector<uint8_t> serialize_msg<z_std_srvs::z_SetBool::Response>(
    const z_std_srvs::z_SetBool::Response& msg) {
    nlohmann::json j;
    j["success"] = msg.success;
    j["message"] = msg.message;
    return nlohmann::json::to_msgpack(j);
}

template <>
inline void deserialize_msg<z_std_srvs::z_SetBool::Response>(
    const std::vector<uint8_t>& buffer, z_std_srvs::z_SetBool::Response& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.success = j["success"].get<bool>();
    msg.message = j["message"].get<std::string>();
}

#endif // Z_STD_SRVS_PC_H
