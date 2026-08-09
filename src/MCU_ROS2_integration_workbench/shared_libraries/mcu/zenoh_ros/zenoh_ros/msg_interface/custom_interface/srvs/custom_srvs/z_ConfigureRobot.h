#ifndef CUSTOM_INTERFACE_CUSTOM_SRVS_Z_CONFIGUREROBOT_H
#define CUSTOM_INTERFACE_CUSTOM_SRVS_Z_CONFIGUREROBOT_H

#include <ArduinoJson.h>
#include <zenoh_ros/custom_msgs/z_MotorStatus.h>

namespace custom_srvs {
struct z_ConfigureRobot {
    struct Request {
        custom_msgs::z_MotorStatus target_status;
        int32_t mode;
    };

    struct Response {
        bool success;
        std::string status_message;
    };
};
} // namespace custom_srvs

// --- Service Request Serializer ---
template <>
inline size_t serialize_msg<custom_srvs::z_ConfigureRobot::Request>(
    const custom_srvs::z_ConfigureRobot::Request& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
      static uint8_t target_status_buf[512];
  size_t target_status_len = serialize_msg(msg.target_status, target_status_buf, sizeof(target_status_buf));
  JsonDocument target_status_sub_doc;
  deserializeMsgPack(target_status_sub_doc, target_status_buf, target_status_len);
  doc["target_status"] = target_status_sub_doc;
    doc["mode"] = msg.mode;
    return serializeMsgPack(doc, buffer, max_len);
}

// --- Service Request Deserializer ---
template <>
inline void deserialize_msg<custom_srvs::z_ConfigureRobot::Request>(
    const uint8_t* buffer, size_t len, custom_srvs::z_ConfigureRobot::Request& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
      static uint8_t target_status_buf[512];
  size_t target_status_len = serializeMsgPack(doc["target_status"], target_status_buf, sizeof(target_status_buf));
  deserialize_msg(target_status_buf, target_status_len, msg.target_status);
    msg.mode = doc["mode"].as<int32_t>();
}

// --- Service Response Serializer ---
template <>
inline size_t serialize_msg<custom_srvs::z_ConfigureRobot::Response>(
    const custom_srvs::z_ConfigureRobot::Response& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["success"] = msg.success;
    doc["status_message"] = msg.status_message;
    return serializeMsgPack(doc, buffer, max_len);
}

// --- Service Response Deserializer ---
template <>
inline void deserialize_msg<custom_srvs::z_ConfigureRobot::Response>(
    const uint8_t* buffer, size_t len, custom_srvs::z_ConfigureRobot::Response& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.success = doc["success"].as<bool>();
    msg.status_message = doc["status_message"].as<std::string>();
}

// Convenience aliases for nested types (no extra #include needed in user code)
using z_MotorStatus = custom_msgs::z_MotorStatus;

using z_ConfigureRobot = custom_srvs::z_ConfigureRobot;

#endif // CUSTOM_INTERFACE_CUSTOM_SRVS_Z_CONFIGUREROBOT_H
