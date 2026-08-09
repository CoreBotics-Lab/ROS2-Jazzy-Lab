#ifndef CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_COMPLEXROBOTSTATE_H
#define CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_COMPLEXROBOTSTATE_H

#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include "msg_interface/pre_defined_interface/z_std_msgs.h"
#include "msg_interface/pre_defined_interface/z_geometry_msgs.h"
#include "z_SensorTelemetry.h"

namespace custom_msgs {
struct z_ComplexRobotState {
    z_std_msgs::z_Header header;
    z_geometry_msgs::z_Pose pose;
    custom_msgs::z_SensorTelemetry telemetry;
    std::string robot_mode;
    uint64_t cycle_count;
};
} // namespace custom_msgs

// --- Topic Serialization Override ---
template <>
inline std::vector<uint8_t> serialize_msg<custom_msgs::z_ComplexRobotState>(
    const custom_msgs::z_ComplexRobotState& msg) {
    nlohmann::json j;
    j["header"] = {{"stamp", {{"sec", msg.header.stamp.sec}, {"nanosec", msg.header.stamp.nanosec}}}, {"frame_id", msg.header.frame_id}};
    j["pose"] = nlohmann::json::from_msgpack(serialize_msg(msg.pose));
    j["telemetry"] = nlohmann::json::from_msgpack(serialize_msg(msg.telemetry));
    j["robot_mode"] = msg.robot_mode;
    j["cycle_count"] = msg.cycle_count;
    return nlohmann::json::to_msgpack(j);
}

// --- Topic Deserialization Override ---
template <>
inline void deserialize_msg<custom_msgs::z_ComplexRobotState>(
    const std::vector<uint8_t>& buffer, custom_msgs::z_ComplexRobotState& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
      msg.header.stamp.sec     = j["header"]["stamp"]["sec"].get<int32_t>();
  msg.header.stamp.nanosec = j["header"]["stamp"]["nanosec"].get<uint32_t>();
  msg.header.frame_id      = j["header"]["frame_id"].get<std::string>();
      std::vector<uint8_t> pose_bytes = nlohmann::json::to_msgpack(j["pose"]);
  deserialize_msg(pose_bytes, msg.pose);
      std::vector<uint8_t> telemetry_bytes = nlohmann::json::to_msgpack(j["telemetry"]);
  deserialize_msg(telemetry_bytes, msg.telemetry);
    msg.robot_mode = j["robot_mode"].get<std::string>();
    msg.cycle_count = j["cycle_count"].get<uint64_t>();
}

// Convenience aliases for nested types (no extra #include needed in user code)
using z_Header = z_std_msgs::z_Header;
using z_Time = builtin_interfaces::z_Time;
using z_Duration = builtin_interfaces::z_Duration;
using z_Pose = z_geometry_msgs::z_Pose;
using z_SensorTelemetry = custom_msgs::z_SensorTelemetry;

using z_ComplexRobotState = custom_msgs::z_ComplexRobotState;

#endif // CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_COMPLEXROBOTSTATE_H
