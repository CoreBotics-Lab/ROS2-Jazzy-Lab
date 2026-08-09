#ifndef CUSTOM_INTERFACE_CUSTOM_MSGS_Z_ROBOTDIAGNOSTIC_H
#define CUSTOM_INTERFACE_CUSTOM_MSGS_Z_ROBOTDIAGNOSTIC_H

#include <ArduinoJson.h>
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
inline size_t serialize_msg<custom_msgs::z_RobotDiagnostic>(
    const custom_msgs::z_RobotDiagnostic& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
      JsonObject header_obj   = doc["header"].to<JsonObject>();
  JsonObject header_stamp = header_obj["stamp"].to<JsonObject>();
  header_stamp["sec"]     = msg.header.stamp.sec;
  header_stamp["nanosec"] = msg.header.stamp.nanosec;
  header_obj["frame_id"]  = msg.header.frame_id;
      JsonArray velocity_arr = doc["velocity"].to<JsonArray>();
  velocity_arr.add(msg.velocity.x);
  velocity_arr.add(msg.velocity.y);
  velocity_arr.add(msg.velocity.z);
      static uint8_t left_motor_buf[512];
  size_t left_motor_len = serialize_msg(msg.left_motor, left_motor_buf, sizeof(left_motor_buf));
  JsonDocument left_motor_sub_doc;
  deserializeMsgPack(left_motor_sub_doc, left_motor_buf, left_motor_len);
  doc["left_motor"] = left_motor_sub_doc;
      static uint8_t right_motor_buf[512];
  size_t right_motor_len = serialize_msg(msg.right_motor, right_motor_buf, sizeof(right_motor_buf));
  JsonDocument right_motor_sub_doc;
  deserializeMsgPack(right_motor_sub_doc, right_motor_buf, right_motor_len);
  doc["right_motor"] = right_motor_sub_doc;
    doc["robot_name"] = msg.robot_name;
    return serializeMsgPack(doc, buffer, max_len);
}

// --- Topic Deserialization Override ---
template <>
inline void deserialize_msg<custom_msgs::z_RobotDiagnostic>(
    const uint8_t* buffer, size_t len, custom_msgs::z_RobotDiagnostic& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
      msg.header.stamp.sec     = doc["header"]["stamp"]["sec"].as<int32_t>();
  msg.header.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>();
  msg.header.frame_id      = doc["header"]["frame_id"].as<std::string>();
      msg.velocity.x = doc["velocity"][0].as<double>();
  msg.velocity.y = doc["velocity"][1].as<double>();
  msg.velocity.z = doc["velocity"][2].as<double>();
      static uint8_t left_motor_buf[512];
  size_t left_motor_len = serializeMsgPack(doc["left_motor"], left_motor_buf, sizeof(left_motor_buf));
  deserialize_msg(left_motor_buf, left_motor_len, msg.left_motor);
      static uint8_t right_motor_buf[512];
  size_t right_motor_len = serializeMsgPack(doc["right_motor"], right_motor_buf, sizeof(right_motor_buf));
  deserialize_msg(right_motor_buf, right_motor_len, msg.right_motor);
    msg.robot_name = doc["robot_name"].as<std::string>();
}

// Convenience aliases for nested types (no extra #include needed in user code)
using z_Header = z_std_msgs::z_Header;
using z_Time = builtin_interfaces::z_Time;
using z_Duration = builtin_interfaces::z_Duration;
using z_Vector3 = z_geometry_msgs::z_Vector3;
using z_MotorStatus = custom_msgs::z_MotorStatus;

using z_RobotDiagnostic = custom_msgs::z_RobotDiagnostic;

#endif // CUSTOM_INTERFACE_CUSTOM_MSGS_Z_ROBOTDIAGNOSTIC_H
