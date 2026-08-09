#ifndef CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_ROBOTDIAGNOSTIC_H
#define CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_ROBOTDIAGNOSTIC_H

#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include "msg_interface/pre_defined_interface/z_std_msgs.h"
#include "msg_interface/pre_defined_interface/z_geometry_msgs.h"
#include "z_MotorStatus.h"

namespace custom_msgs {
struct z_RobotDiagnostic {
    z_std_msgs::z_Header header;
    z_geometry_msgs::z_Vector3 velocity;
    custom_msgs::z_MotorStatus left_motor;
    custom_msgs::z_MotorStatus right_motor;
    std::string robot_name;
};
} // namespace custom_msgs

// --- Topic Serialization Override ---
template <>
inline std::vector<uint8_t> serialize_msg<custom_msgs::z_RobotDiagnostic>(
    const custom_msgs::z_RobotDiagnostic& msg) {
    nlohmann::json j;
    j["header"] = {{"stamp", {{"sec", msg.header.stamp.sec}, {"nanosec", msg.header.stamp.nanosec}}}, {"frame_id", msg.header.frame_id}};
    j["velocity"] = {msg.velocity.x, msg.velocity.y, msg.velocity.z};
    j["left_motor"] = nlohmann::json::from_msgpack(serialize_msg(msg.left_motor));
    j["right_motor"] = nlohmann::json::from_msgpack(serialize_msg(msg.right_motor));
    j["robot_name"] = msg.robot_name;
    return nlohmann::json::to_msgpack(j);
}

// --- Topic Deserialization Override ---
template <>
inline void deserialize_msg<custom_msgs::z_RobotDiagnostic>(
    const std::vector<uint8_t>& buffer, custom_msgs::z_RobotDiagnostic& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
      msg.header.stamp.sec     = j["header"]["stamp"]["sec"].get<int32_t>();
  msg.header.stamp.nanosec = j["header"]["stamp"]["nanosec"].get<uint32_t>();
  msg.header.frame_id      = j["header"]["frame_id"].get<std::string>();
      msg.velocity.x = j["velocity"][0].get<double>();
  msg.velocity.y = j["velocity"][1].get<double>();
  msg.velocity.z = j["velocity"][2].get<double>();
      std::vector<uint8_t> left_motor_bytes = nlohmann::json::to_msgpack(j["left_motor"]);
  deserialize_msg(left_motor_bytes, msg.left_motor);
      std::vector<uint8_t> right_motor_bytes = nlohmann::json::to_msgpack(j["right_motor"]);
  deserialize_msg(right_motor_bytes, msg.right_motor);
    msg.robot_name = j["robot_name"].get<std::string>();
}

// Convenience aliases for nested types (no extra #include needed in user code)
using z_Header = z_std_msgs::z_Header;
using z_Time = builtin_interfaces::z_Time;
using z_Duration = builtin_interfaces::z_Duration;
using z_Vector3 = z_geometry_msgs::z_Vector3;
using z_MotorStatus = custom_msgs::z_MotorStatus;

using z_RobotDiagnostic = custom_msgs::z_RobotDiagnostic;

#endif // CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_ROBOTDIAGNOSTIC_H
