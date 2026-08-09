#ifndef CUSTOM_INTERFACE_CUSTOM_MSGS_Z_MOTORSTATUS_H
#define CUSTOM_INTERFACE_CUSTOM_MSGS_Z_MOTORSTATUS_H

#include <ArduinoJson.h>

namespace custom_msgs {
struct z_MotorStatus {
    int32_t motor_id;
    float speed;
    float temperature;
    bool is_active;
};
} // namespace custom_msgs

// --- Topic Serialization Override ---
template <>
inline size_t serialize_msg<custom_msgs::z_MotorStatus>(
    const custom_msgs::z_MotorStatus& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["motor_id"] = msg.motor_id;
    doc["speed"] = msg.speed;
    doc["temperature"] = msg.temperature;
    doc["is_active"] = msg.is_active;
    return serializeMsgPack(doc, buffer, max_len);
}

// --- Topic Deserialization Override ---
template <>
inline void deserialize_msg<custom_msgs::z_MotorStatus>(
    const uint8_t* buffer, size_t len, custom_msgs::z_MotorStatus& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.motor_id = doc["motor_id"].as<int32_t>();
    msg.speed = doc["speed"].as<float>();
    msg.temperature = doc["temperature"].as<float>();
    msg.is_active = doc["is_active"].as<bool>();
}

using z_MotorStatus = custom_msgs::z_MotorStatus;

#endif // CUSTOM_INTERFACE_CUSTOM_MSGS_Z_MOTORSTATUS_H
