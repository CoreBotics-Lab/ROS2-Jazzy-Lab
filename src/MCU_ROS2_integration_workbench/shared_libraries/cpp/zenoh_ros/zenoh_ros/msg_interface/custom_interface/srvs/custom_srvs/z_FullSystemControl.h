#ifndef CUSTOM_INTERFACE_PC_CUSTOM_SRVS_Z_FULLSYSTEMCONTROL_H
#define CUSTOM_INTERFACE_PC_CUSTOM_SRVS_Z_FULLSYSTEMCONTROL_H

#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <zenoh_ros/custom_msgs/z_ComplexRobotState.h>
#include <zenoh_ros/custom_msgs/z_SensorTelemetry.h>

namespace custom_srvs {
struct z_FullSystemControl {
    struct Request {
        custom_msgs::z_ComplexRobotState target_state;
        int32_t command_code;
    };

    struct Response {
        custom_msgs::z_SensorTelemetry current_telemetry;
        bool ack;
        std::string status_details;
    };
};
} // namespace custom_srvs

// --- Service Request Serializer ---
template <>
inline std::vector<uint8_t> serialize_msg<custom_srvs::z_FullSystemControl::Request>(
    const custom_srvs::z_FullSystemControl::Request& msg) {
    nlohmann::json j;
    j["target_state"] = nlohmann::json::from_msgpack(serialize_msg(msg.target_state));
    j["command_code"] = msg.command_code;
    return nlohmann::json::to_msgpack(j);
}

// --- Service Request Deserializer ---
template <>
inline void deserialize_msg<custom_srvs::z_FullSystemControl::Request>(
    const std::vector<uint8_t>& buffer, custom_srvs::z_FullSystemControl::Request& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
      std::vector<uint8_t> target_state_bytes = nlohmann::json::to_msgpack(j["target_state"]);
  deserialize_msg(target_state_bytes, msg.target_state);
    msg.command_code = j["command_code"].get<int32_t>();
}

// --- Service Response Serializer ---
template <>
inline std::vector<uint8_t> serialize_msg<custom_srvs::z_FullSystemControl::Response>(
    const custom_srvs::z_FullSystemControl::Response& msg) {
    nlohmann::json j;
    j["current_telemetry"] = nlohmann::json::from_msgpack(serialize_msg(msg.current_telemetry));
    j["ack"] = msg.ack;
    j["status_details"] = msg.status_details;
    return nlohmann::json::to_msgpack(j);
}

// --- Service Response Deserializer ---
template <>
inline void deserialize_msg<custom_srvs::z_FullSystemControl::Response>(
    const std::vector<uint8_t>& buffer, custom_srvs::z_FullSystemControl::Response& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
      std::vector<uint8_t> current_telemetry_bytes = nlohmann::json::to_msgpack(j["current_telemetry"]);
  deserialize_msg(current_telemetry_bytes, msg.current_telemetry);
    msg.ack = j["ack"].get<bool>();
    msg.status_details = j["status_details"].get<std::string>();
}

// Convenience aliases for nested types (no extra #include needed in user code)
using z_ComplexRobotState = custom_msgs::z_ComplexRobotState;
using z_SensorTelemetry = custom_msgs::z_SensorTelemetry;

using z_FullSystemControl = custom_srvs::z_FullSystemControl;

#endif // CUSTOM_INTERFACE_PC_CUSTOM_SRVS_Z_FULLSYSTEMCONTROL_H
