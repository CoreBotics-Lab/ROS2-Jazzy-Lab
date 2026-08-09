#ifndef CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_MOTORSTATUS_H
#define CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_MOTORSTATUS_H

#include <nlohmann/json.hpp>
#include <vector>
#include <string>

namespace custom_msgs {
struct z_MotorStatus {
    int32_t motor_id;
    float speed;
    float temperature;
    bool is_active;
};
} // namespace custom_msgs

// --- Topic Serialization Override ---
template <>
inline std::vector<uint8_t> serialize_msg<custom_msgs::z_MotorStatus>(
    const custom_msgs::z_MotorStatus& msg) {
    nlohmann::json j;
    j["motor_id"] = msg.motor_id;
    j["speed"] = msg.speed;
    j["temperature"] = msg.temperature;
    j["is_active"] = msg.is_active;
    return nlohmann::json::to_msgpack(j);
}

// --- Topic Deserialization Override ---
template <>
inline void deserialize_msg<custom_msgs::z_MotorStatus>(
    const std::vector<uint8_t>& buffer, custom_msgs::z_MotorStatus& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.motor_id = j["motor_id"].get<int32_t>();
    msg.speed = j["speed"].get<float>();
    msg.temperature = j["temperature"].get<float>();
    msg.is_active = j["is_active"].get<bool>();
}

using z_MotorStatus = custom_msgs::z_MotorStatus;

#endif // CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_MOTORSTATUS_H
