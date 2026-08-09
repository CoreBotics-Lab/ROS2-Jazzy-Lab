#ifndef CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_MPU6050DATA_H
#define CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_MPU6050DATA_H

#include <nlohmann/json.hpp>
#include <vector>
#include <string>

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
inline std::vector<uint8_t> serialize_msg<custom_msgs::z_MPU6050Data>(
    const custom_msgs::z_MPU6050Data& msg) {
    nlohmann::json j;
    j["accel_x"] = msg.accel_x;
    j["accel_y"] = msg.accel_y;
    j["accel_z"] = msg.accel_z;
    j["gyro_x"] = msg.gyro_x;
    j["gyro_y"] = msg.gyro_y;
    j["gyro_z"] = msg.gyro_z;
    j["temperature"] = msg.temperature;
    return nlohmann::json::to_msgpack(j);
}

// --- Topic Deserialization Override ---
template <>
inline void deserialize_msg<custom_msgs::z_MPU6050Data>(
    const std::vector<uint8_t>& buffer, custom_msgs::z_MPU6050Data& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.accel_x = j["accel_x"].get<float>();
    msg.accel_y = j["accel_y"].get<float>();
    msg.accel_z = j["accel_z"].get<float>();
    msg.gyro_x = j["gyro_x"].get<float>();
    msg.gyro_y = j["gyro_y"].get<float>();
    msg.gyro_z = j["gyro_z"].get<float>();
    msg.temperature = j["temperature"].get<float>();
}

using z_MPU6050Data = custom_msgs::z_MPU6050Data;

#endif // CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_MPU6050DATA_H
