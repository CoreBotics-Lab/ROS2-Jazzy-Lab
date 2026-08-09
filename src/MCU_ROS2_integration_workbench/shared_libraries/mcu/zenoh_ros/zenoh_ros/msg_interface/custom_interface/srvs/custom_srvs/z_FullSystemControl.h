#ifndef CUSTOM_INTERFACE_CUSTOM_SRVS_Z_FULLSYSTEMCONTROL_H
#define CUSTOM_INTERFACE_CUSTOM_SRVS_Z_FULLSYSTEMCONTROL_H

#include <ArduinoJson.h>
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
inline size_t serialize_msg<custom_srvs::z_FullSystemControl::Request>(
    const custom_srvs::z_FullSystemControl::Request& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
      static uint8_t target_state_buf[512];
  size_t target_state_len = serialize_msg(msg.target_state, target_state_buf, sizeof(target_state_buf));
  JsonDocument target_state_sub_doc;
  deserializeMsgPack(target_state_sub_doc, target_state_buf, target_state_len);
  doc["target_state"] = target_state_sub_doc;
    doc["command_code"] = msg.command_code;
    return serializeMsgPack(doc, buffer, max_len);
}

// --- Service Request Deserializer ---
template <>
inline void deserialize_msg<custom_srvs::z_FullSystemControl::Request>(
    const uint8_t* buffer, size_t len, custom_srvs::z_FullSystemControl::Request& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
      static uint8_t target_state_buf[512];
  size_t target_state_len = serializeMsgPack(doc["target_state"], target_state_buf, sizeof(target_state_buf));
  deserialize_msg(target_state_buf, target_state_len, msg.target_state);
    msg.command_code = doc["command_code"].as<int32_t>();
}

// --- Service Response Serializer ---
template <>
inline size_t serialize_msg<custom_srvs::z_FullSystemControl::Response>(
    const custom_srvs::z_FullSystemControl::Response& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
      static uint8_t current_telemetry_buf[512];
  size_t current_telemetry_len = serialize_msg(msg.current_telemetry, current_telemetry_buf, sizeof(current_telemetry_buf));
  JsonDocument current_telemetry_sub_doc;
  deserializeMsgPack(current_telemetry_sub_doc, current_telemetry_buf, current_telemetry_len);
  doc["current_telemetry"] = current_telemetry_sub_doc;
    doc["ack"] = msg.ack;
    doc["status_details"] = msg.status_details;
    return serializeMsgPack(doc, buffer, max_len);
}

// --- Service Response Deserializer ---
template <>
inline void deserialize_msg<custom_srvs::z_FullSystemControl::Response>(
    const uint8_t* buffer, size_t len, custom_srvs::z_FullSystemControl::Response& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
      static uint8_t current_telemetry_buf[512];
  size_t current_telemetry_len = serializeMsgPack(doc["current_telemetry"], current_telemetry_buf, sizeof(current_telemetry_buf));
  deserialize_msg(current_telemetry_buf, current_telemetry_len, msg.current_telemetry);
    msg.ack = doc["ack"].as<bool>();
    msg.status_details = doc["status_details"].as<std::string>();
}

// Convenience aliases for nested types (no extra #include needed in user code)
using z_ComplexRobotState = custom_msgs::z_ComplexRobotState;
using z_SensorTelemetry = custom_msgs::z_SensorTelemetry;

using z_FullSystemControl = custom_srvs::z_FullSystemControl;

#endif // CUSTOM_INTERFACE_CUSTOM_SRVS_Z_FULLSYSTEMCONTROL_H
