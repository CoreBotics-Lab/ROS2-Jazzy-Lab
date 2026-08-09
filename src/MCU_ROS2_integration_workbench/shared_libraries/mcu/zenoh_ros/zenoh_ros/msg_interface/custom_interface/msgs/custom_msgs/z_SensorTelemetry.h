#ifndef CUSTOM_INTERFACE_CUSTOM_MSGS_Z_SENSORTELEMETRY_H
#define CUSTOM_INTERFACE_CUSTOM_MSGS_Z_SENSORTELEMETRY_H

#include <ArduinoJson.h>
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
inline size_t serialize_msg<custom_msgs::z_SensorTelemetry>(
    const custom_msgs::z_SensorTelemetry& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
      JsonObject header_obj   = doc["header"].to<JsonObject>();
  JsonObject header_stamp = header_obj["stamp"].to<JsonObject>();
  header_stamp["sec"]     = msg.header.stamp.sec;
  header_stamp["nanosec"] = msg.header.stamp.nanosec;
  header_obj["frame_id"]  = msg.header.frame_id;
      JsonArray accel_arr = doc["accel"].to<JsonArray>();
  accel_arr.add(msg.accel.x);
  accel_arr.add(msg.accel.y);
  accel_arr.add(msg.accel.z);
      JsonArray orientation_arr = doc["orientation"].to<JsonArray>();
  orientation_arr.add(msg.orientation.x);
  orientation_arr.add(msg.orientation.y);
  orientation_arr.add(msg.orientation.z);
  orientation_arr.add(msg.orientation.w);
    doc["sensor_id"] = msg.sensor_id;
    doc["temp"] = msg.temp;
    doc["status_ok"] = msg.status_ok;
    return serializeMsgPack(doc, buffer, max_len);
}

// --- Topic Deserialization Override ---
template <>
inline void deserialize_msg<custom_msgs::z_SensorTelemetry>(
    const uint8_t* buffer, size_t len, custom_msgs::z_SensorTelemetry& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
      msg.header.stamp.sec     = doc["header"]["stamp"]["sec"].as<int32_t>();
  msg.header.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>();
  msg.header.frame_id      = doc["header"]["frame_id"].as<std::string>();
      msg.accel.x = doc["accel"][0].as<double>();
  msg.accel.y = doc["accel"][1].as<double>();
  msg.accel.z = doc["accel"][2].as<double>();
      msg.orientation.x = doc["orientation"][0].as<double>();
  msg.orientation.y = doc["orientation"][1].as<double>();
  msg.orientation.z = doc["orientation"][2].as<double>();
  msg.orientation.w = doc["orientation"][3].as<double>();
    msg.sensor_id = doc["sensor_id"].as<int32_t>();
    msg.temp = doc["temp"].as<float>();
    msg.status_ok = doc["status_ok"].as<bool>();
}

// Convenience aliases for nested types (no extra #include needed in user code)
using z_Header = z_std_msgs::z_Header;
using z_Time = builtin_interfaces::z_Time;
using z_Duration = builtin_interfaces::z_Duration;
using z_Vector3 = z_geometry_msgs::z_Vector3;
using z_Quaternion = z_geometry_msgs::z_Quaternion;

using z_SensorTelemetry = custom_msgs::z_SensorTelemetry;

#endif // CUSTOM_INTERFACE_CUSTOM_MSGS_Z_SENSORTELEMETRY_H
