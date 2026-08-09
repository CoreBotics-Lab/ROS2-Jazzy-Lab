#ifndef CUSTOM_INTERFACE_CUSTOM_MSGS_Z_ROBOTSTATE_H
#define CUSTOM_INTERFACE_CUSTOM_MSGS_Z_ROBOTSTATE_H

#include <ArduinoJson.h>
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
inline size_t serialize_msg<custom_msgs::z_RobotState>(
    const custom_msgs::z_RobotState& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["name"] = msg.name;
      JsonArray position_arr = doc.createNestedArray("position");
  position_arr.add(msg.position.x);
  position_arr.add(msg.position.y);
  position_arr.add(msg.position.z);
      JsonArray velocity_arr = doc.createNestedArray("velocity");
  JsonArray velocity_lin = velocity_arr.createNestedArray();
  velocity_lin.add(msg.velocity.linear.x);
  velocity_lin.add(msg.velocity.linear.y);
  velocity_lin.add(msg.velocity.linear.z);
  JsonArray velocity_ang = velocity_arr.createNestedArray();
  velocity_ang.add(msg.velocity.angular.x);
  velocity_ang.add(msg.velocity.angular.y);
  velocity_ang.add(msg.velocity.angular.z);
    return serializeMsgPack(doc, buffer, max_len);
}

// --- Topic Deserialization Override ---
template <>
inline void deserialize_msg<custom_msgs::z_RobotState>(
    const uint8_t* buffer, size_t len, custom_msgs::z_RobotState& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.name = doc["name"].as<std::string>();
      msg.position.x = doc["position"][0].as<double>();
  msg.position.y = doc["position"][1].as<double>();
  msg.position.z = doc["position"][2].as<double>();
      msg.velocity.linear.x = doc["velocity"][0][0].as<double>();
  msg.velocity.linear.y = doc["velocity"][0][1].as<double>();
  msg.velocity.linear.z = doc["velocity"][0][2].as<double>();
  msg.velocity.angular.x = doc["velocity"][1][0].as<double>();
  msg.velocity.angular.y = doc["velocity"][1][1].as<double>();
  msg.velocity.angular.z = doc["velocity"][1][2].as<double>();
}

using z_RobotState = custom_msgs::z_RobotState;

#endif // CUSTOM_INTERFACE_CUSTOM_MSGS_Z_ROBOTSTATE_H
