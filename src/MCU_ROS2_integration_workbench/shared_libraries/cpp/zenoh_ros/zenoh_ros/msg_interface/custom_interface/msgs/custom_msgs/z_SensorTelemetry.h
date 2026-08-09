#ifndef CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_SENSORTELEMETRY_H
#define CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_SENSORTELEMETRY_H

#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include "msg_interface/pre_defined_interface/z_std_msgs.h"
#include "msg_interface/pre_defined_interface/z_geometry_msgs.h"

namespace custom_msgs {
struct z_SensorTelemetry {
    z_std_msgs::z_Header header;
    z_geometry_msgs::z_Vector3 accel;
    z_geometry_msgs::z_Quaternion orientation;
    int32_t sensor_id;
    float temp;
    bool status_ok;
};
} // namespace custom_msgs

// --- Topic Serialization Override ---
template <>
inline std::vector<uint8_t> serialize_msg<custom_msgs::z_SensorTelemetry>(
    const custom_msgs::z_SensorTelemetry& msg) {
    nlohmann::json j;
    j["header"] = {{"stamp", {{"sec", msg.header.stamp.sec}, {"nanosec", msg.header.stamp.nanosec}}}, {"frame_id", msg.header.frame_id}};
    j["accel"] = {msg.accel.x, msg.accel.y, msg.accel.z};
    j["orientation"] = {msg.orientation.x, msg.orientation.y, msg.orientation.z, msg.orientation.w};
    j["sensor_id"] = msg.sensor_id;
    j["temp"] = msg.temp;
    j["status_ok"] = msg.status_ok;
    return nlohmann::json::to_msgpack(j);
}

// --- Topic Deserialization Override ---
template <>
inline void deserialize_msg<custom_msgs::z_SensorTelemetry>(
    const std::vector<uint8_t>& buffer, custom_msgs::z_SensorTelemetry& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
      msg.header.stamp.sec     = j["header"]["stamp"]["sec"].get<int32_t>();
  msg.header.stamp.nanosec = j["header"]["stamp"]["nanosec"].get<uint32_t>();
  msg.header.frame_id      = j["header"]["frame_id"].get<std::string>();
      msg.accel.x = j["accel"][0].get<double>();
  msg.accel.y = j["accel"][1].get<double>();
  msg.accel.z = j["accel"][2].get<double>();
      msg.orientation.x = j["orientation"][0].get<double>();
  msg.orientation.y = j["orientation"][1].get<double>();
  msg.orientation.z = j["orientation"][2].get<double>();
  msg.orientation.w = j["orientation"][3].get<double>();
    msg.sensor_id = j["sensor_id"].get<int32_t>();
    msg.temp = j["temp"].get<float>();
    msg.status_ok = j["status_ok"].get<bool>();
}

// Convenience aliases for nested types (no extra #include needed in user code)
using z_Header = z_std_msgs::z_Header;
using z_Time = builtin_interfaces::z_Time;
using z_Duration = builtin_interfaces::z_Duration;
using z_Vector3 = z_geometry_msgs::z_Vector3;
using z_Quaternion = z_geometry_msgs::z_Quaternion;

using z_SensorTelemetry = custom_msgs::z_SensorTelemetry;

#endif // CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_SENSORTELEMETRY_H
