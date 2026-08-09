#ifndef CUSTOM_INTERFACE_PC_CUSTOM_SRVS_Z_SETLEDCOLOR_H
#define CUSTOM_INTERFACE_PC_CUSTOM_SRVS_Z_SETLEDCOLOR_H

#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <zenoh_ros/custom_msgs/z_SetLED.h>

namespace custom_srvs {
struct z_SetLEDColor {
    struct Request {
        custom_msgs::z_SetLED led_data;
    };

    struct Response {
        bool success;
        std::string message;
    };
};
} // namespace custom_srvs

// --- Service Request Serializer ---
template <>
inline std::vector<uint8_t> serialize_msg<custom_srvs::z_SetLEDColor::Request>(
    const custom_srvs::z_SetLEDColor::Request& msg) {
    nlohmann::json j;
    j["led_data"] = nlohmann::json::from_msgpack(serialize_msg(msg.led_data));
    return nlohmann::json::to_msgpack(j);
}

// --- Service Request Deserializer ---
template <>
inline void deserialize_msg<custom_srvs::z_SetLEDColor::Request>(
    const std::vector<uint8_t>& buffer, custom_srvs::z_SetLEDColor::Request& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
      std::vector<uint8_t> led_data_bytes = nlohmann::json::to_msgpack(j["led_data"]);
  deserialize_msg(led_data_bytes, msg.led_data);
}

// --- Service Response Serializer ---
template <>
inline std::vector<uint8_t> serialize_msg<custom_srvs::z_SetLEDColor::Response>(
    const custom_srvs::z_SetLEDColor::Response& msg) {
    nlohmann::json j;
    j["success"] = msg.success;
    j["message"] = msg.message;
    return nlohmann::json::to_msgpack(j);
}

// --- Service Response Deserializer ---
template <>
inline void deserialize_msg<custom_srvs::z_SetLEDColor::Response>(
    const std::vector<uint8_t>& buffer, custom_srvs::z_SetLEDColor::Response& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.success = j["success"].get<bool>();
    msg.message = j["message"].get<std::string>();
}

// Convenience aliases for nested types (no extra #include needed in user code)
using z_SetLED = custom_msgs::z_SetLED;

using z_SetLEDColor = custom_srvs::z_SetLEDColor;

#endif // CUSTOM_INTERFACE_PC_CUSTOM_SRVS_Z_SETLEDCOLOR_H
