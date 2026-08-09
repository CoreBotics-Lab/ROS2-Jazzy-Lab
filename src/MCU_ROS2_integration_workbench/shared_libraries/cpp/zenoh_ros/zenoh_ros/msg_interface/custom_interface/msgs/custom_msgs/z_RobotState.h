#ifndef CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_ROBOTSTATE_H
#define CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_ROBOTSTATE_H

#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include "msg_interface/pre_defined_interface/z_geometry_msgs.h"

namespace custom_msgs {
struct z_RobotState {
    std::string name;
    z_geometry_msgs::z_Vector3 position;
    z_geometry_msgs::z_Twist velocity;
};
} // namespace custom_msgs

// --- Topic Serialization Override ---
template <>
inline std::vector<uint8_t> serialize_msg<custom_msgs::z_RobotState>(
    const custom_msgs::z_RobotState& msg) {
    nlohmann::json j;
    j["name"] = msg.name;
    j["position"] = {msg.position.x, msg.position.y, msg.position.z};
    j["velocity"] = {
    {msg.velocity.linear.x, msg.velocity.linear.y, msg.velocity.linear.z},
    {msg.velocity.angular.x, msg.velocity.angular.y, msg.velocity.angular.z}
  };
    return nlohmann::json::to_msgpack(j);
}

// --- Topic Deserialization Override ---
template <>
inline void deserialize_msg<custom_msgs::z_RobotState>(
    const std::vector<uint8_t>& buffer, custom_msgs::z_RobotState& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.name = j["name"].get<std::string>();
      msg.position.x = j["position"][0].get<double>();
  msg.position.y = j["position"][1].get<double>();
  msg.position.z = j["position"][2].get<double>();
      msg.velocity.linear.x = j["velocity"][0][0].get<double>();
  msg.velocity.linear.y = j["velocity"][0][1].get<double>();
  msg.velocity.linear.z = j["velocity"][0][2].get<double>();
  msg.velocity.angular.x = j["velocity"][1][0].get<double>();
  msg.velocity.angular.y = j["velocity"][1][1].get<double>();
  msg.velocity.angular.z = j["velocity"][1][2].get<double>();
}

// Convenience aliases for nested types (no extra #include needed in user code)
using z_Vector3 = z_geometry_msgs::z_Vector3;
using z_Twist = z_geometry_msgs::z_Twist;

using z_RobotState = custom_msgs::z_RobotState;

#endif // CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_ROBOTSTATE_H
