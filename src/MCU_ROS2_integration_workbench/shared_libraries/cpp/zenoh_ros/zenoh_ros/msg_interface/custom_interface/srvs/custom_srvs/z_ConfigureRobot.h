#ifndef CUSTOM_INTERFACE_PC_CUSTOM_SRVS_Z_CONFIGUREROBOT_H
#define CUSTOM_INTERFACE_PC_CUSTOM_SRVS_Z_CONFIGUREROBOT_H

#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <zenoh_ros/custom_msgs/z_MotorStatus.h>

namespace custom_srvs {
struct z_ConfigureRobot {
    struct Request {
        custom_msgs::z_MotorStatus target_status;
        int32_t mode;
    };

    struct Response {
        bool success;
        std::string status_message;
    };
};
} // namespace custom_srvs

// --- Service Request Serializer ---
template <>
inline std::vector<uint8_t> serialize_msg<custom_srvs::z_ConfigureRobot::Request>(
    const custom_srvs::z_ConfigureRobot::Request& msg) {
    nlohmann::json j;
    j["target_status"] = nlohmann::json::from_msgpack(serialize_msg(msg.target_status));
    j["mode"] = msg.mode;
    return nlohmann::json::to_msgpack(j);
}

// --- Service Request Deserializer ---
template <>
inline void deserialize_msg<custom_srvs::z_ConfigureRobot::Request>(
    const std::vector<uint8_t>& buffer, custom_srvs::z_ConfigureRobot::Request& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
      std::vector<uint8_t> target_status_bytes = nlohmann::json::to_msgpack(j["target_status"]);
  deserialize_msg(target_status_bytes, msg.target_status);
    msg.mode = j["mode"].get<int32_t>();
}

// --- Service Response Serializer ---
template <>
inline std::vector<uint8_t> serialize_msg<custom_srvs::z_ConfigureRobot::Response>(
    const custom_srvs::z_ConfigureRobot::Response& msg) {
    nlohmann::json j;
    j["success"] = msg.success;
    j["status_message"] = msg.status_message;
    return nlohmann::json::to_msgpack(j);
}

// --- Service Response Deserializer ---
template <>
inline void deserialize_msg<custom_srvs::z_ConfigureRobot::Response>(
    const std::vector<uint8_t>& buffer, custom_srvs::z_ConfigureRobot::Response& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.success = j["success"].get<bool>();
    msg.status_message = j["status_message"].get<std::string>();
}

// Convenience aliases for nested types (no extra #include needed in user code)
using z_MotorStatus = custom_msgs::z_MotorStatus;

using z_ConfigureRobot = custom_srvs::z_ConfigureRobot;

#endif // CUSTOM_INTERFACE_PC_CUSTOM_SRVS_Z_CONFIGUREROBOT_H
