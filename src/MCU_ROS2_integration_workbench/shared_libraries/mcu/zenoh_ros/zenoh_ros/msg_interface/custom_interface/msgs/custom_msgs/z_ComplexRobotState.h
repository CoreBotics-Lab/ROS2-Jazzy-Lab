#ifndef CUSTOM_INTERFACE_CUSTOM_MSGS_Z_COMPLEXROBOTSTATE_H
#define CUSTOM_INTERFACE_CUSTOM_MSGS_Z_COMPLEXROBOTSTATE_H

#include <ArduinoJson.h>
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
inline size_t serialize_msg<custom_msgs::z_ComplexRobotState>(
    const custom_msgs::z_ComplexRobotState& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
      JsonObject header_obj   = doc["header"].to<JsonObject>();
  JsonObject header_stamp = header_obj["stamp"].to<JsonObject>();
  header_stamp["sec"]     = msg.header.stamp.sec;
  header_stamp["nanosec"] = msg.header.stamp.nanosec;
  header_obj["frame_id"]  = msg.header.frame_id;
      static uint8_t pose_buf[512];
  size_t pose_len = serialize_msg(msg.pose, pose_buf, sizeof(pose_buf));
  JsonDocument pose_sub_doc;
  deserializeMsgPack(pose_sub_doc, pose_buf, pose_len);
  doc["pose"] = pose_sub_doc;
      static uint8_t telemetry_buf[512];
  size_t telemetry_len = serialize_msg(msg.telemetry, telemetry_buf, sizeof(telemetry_buf));
  JsonDocument telemetry_sub_doc;
  deserializeMsgPack(telemetry_sub_doc, telemetry_buf, telemetry_len);
  doc["telemetry"] = telemetry_sub_doc;
    doc["robot_mode"] = msg.robot_mode;
    doc["cycle_count"] = msg.cycle_count;
    return serializeMsgPack(doc, buffer, max_len);
}

// --- Topic Deserialization Override ---
template <>
inline void deserialize_msg<custom_msgs::z_ComplexRobotState>(
    const uint8_t* buffer, size_t len, custom_msgs::z_ComplexRobotState& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
      msg.header.stamp.sec     = doc["header"]["stamp"]["sec"].as<int32_t>();
  msg.header.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>();
  msg.header.frame_id      = doc["header"]["frame_id"].as<std::string>();
      static uint8_t pose_buf[512];
  size_t pose_len = serializeMsgPack(doc["pose"], pose_buf, sizeof(pose_buf));
  deserialize_msg(pose_buf, pose_len, msg.pose);
      static uint8_t telemetry_buf[512];
  size_t telemetry_len = serializeMsgPack(doc["telemetry"], telemetry_buf, sizeof(telemetry_buf));
  deserialize_msg(telemetry_buf, telemetry_len, msg.telemetry);
    msg.robot_mode = doc["robot_mode"].as<std::string>();
    msg.cycle_count = doc["cycle_count"].as<uint64_t>();
}

// Convenience aliases for nested types (no extra #include needed in user code)
using z_Header = z_std_msgs::z_Header;
using z_Time = builtin_interfaces::z_Time;
using z_Duration = builtin_interfaces::z_Duration;
using z_Pose = z_geometry_msgs::z_Pose;
using z_SensorTelemetry = custom_msgs::z_SensorTelemetry;

using z_ComplexRobotState = custom_msgs::z_ComplexRobotState;

#endif // CUSTOM_INTERFACE_CUSTOM_MSGS_Z_COMPLEXROBOTSTATE_H
