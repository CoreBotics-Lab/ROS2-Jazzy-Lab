#ifndef CUSTOM_INTERFACE_CUSTOM_MSGS_Z_MPU6050DATA_H
#define CUSTOM_INTERFACE_CUSTOM_MSGS_Z_MPU6050DATA_H

#include <ArduinoJson.h>

namespace custom_msgs {
struct z_MPU6050Data {
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
    float temperature;
};
} // namespace custom_msgs

// --- Topic Serialization Override ---
template <>
inline size_t serialize_msg<custom_msgs::z_MPU6050Data>(
    const custom_msgs::z_MPU6050Data& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["accel_x"] = msg.accel_x;
    doc["accel_y"] = msg.accel_y;
    doc["accel_z"] = msg.accel_z;
    doc["gyro_x"] = msg.gyro_x;
    doc["gyro_y"] = msg.gyro_y;
    doc["gyro_z"] = msg.gyro_z;
    doc["temperature"] = msg.temperature;
    return serializeMsgPack(doc, buffer, max_len);
}

// --- Topic Deserialization Override ---
template <>
inline void deserialize_msg<custom_msgs::z_MPU6050Data>(
    const uint8_t* buffer, size_t len, custom_msgs::z_MPU6050Data& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.accel_x = doc["accel_x"].as<float>();
    msg.accel_y = doc["accel_y"].as<float>();
    msg.accel_z = doc["accel_z"].as<float>();
    msg.gyro_x = doc["gyro_x"].as<float>();
    msg.gyro_y = doc["gyro_y"].as<float>();
    msg.gyro_z = doc["gyro_z"].as<float>();
    msg.temperature = doc["temperature"].as<float>();
}

using z_MPU6050Data = custom_msgs::z_MPU6050Data;

#endif // CUSTOM_INTERFACE_CUSTOM_MSGS_Z_MPU6050DATA_H
