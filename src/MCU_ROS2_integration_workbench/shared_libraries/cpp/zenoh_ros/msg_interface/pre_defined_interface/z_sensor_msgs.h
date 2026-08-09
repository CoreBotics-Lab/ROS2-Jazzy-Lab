#ifndef Z_SENSOR_MSGS_H
#define Z_SENSOR_MSGS_H

#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include "z_std_msgs.h"
#include "z_geometry_msgs.h"
#include "z_builtin_interfaces.h"

// Forward declarations
template <typename T>
std::vector<uint8_t> serialize_msg(const T& msg);

template <typename T>
void deserialize_msg(const std::vector<uint8_t>& buffer, T& msg);

namespace z_sensor_msgs {

    struct z_Imu {
        z_std_msgs::z_Header header;
        z_geometry_msgs::z_Quaternion orientation;
        double orientation_covariance[9] = {0};
        z_geometry_msgs::z_Vector3 angular_velocity;
        double angular_velocity_covariance[9] = {0};
        z_geometry_msgs::z_Vector3 linear_acceleration;
        double linear_acceleration_covariance[9] = {0};
    };

    struct z_JointState {
        z_std_msgs::z_Header header;
        std::vector<std::string> name;
        std::vector<double> position;
        std::vector<double> velocity;
        std::vector<double> effort;
    };

    struct z_Temperature {
        z_std_msgs::z_Header header;
        double temperature = 0.0;
        double variance = 0.0;
    };

    struct z_FluidPressure {
        z_std_msgs::z_Header header;
        double fluid_pressure = 0.0;
        double variance = 0.0;
    };

    struct z_Illuminance {
        z_std_msgs::z_Header header;
        double illuminance = 0.0;
        double variance = 0.0;
    };

    struct z_MagneticField {
        z_std_msgs::z_Header header;
        z_geometry_msgs::z_Vector3 magnetic_field;
        double magnetic_field_covariance[9] = {0};
    };

    struct z_Range {
        z_std_msgs::z_Header header;
        uint8_t radiation_type = 0;
        float field_of_view = 0.0f;
        float min_range = 0.0f;
        float max_range = 0.0f;
        float range = 0.0f;
    };

} // namespace z_sensor_msgs

// --- Serialization Specializations ---

template <>
inline std::vector<uint8_t> serialize_msg<z_sensor_msgs::z_Imu>(const z_sensor_msgs::z_Imu& msg) {
    nlohmann::json j;
    j["header"] = {{"stamp", {{"sec", msg.header.stamp.sec}, {"nanosec", msg.header.stamp.nanosec}}}, {"frame_id", msg.header.frame_id}};
    j["orientation"] = {{"x", msg.orientation.x}, {"y", msg.orientation.y}, {"z", msg.orientation.z}, {"w", msg.orientation.w}};
    j["angular_velocity"] = {{"x", msg.angular_velocity.x}, {"y", msg.angular_velocity.y}, {"z", msg.angular_velocity.z}};
    j["linear_acceleration"] = {{"x", msg.linear_acceleration.x}, {"y", msg.linear_acceleration.y}, {"z", msg.linear_acceleration.z}};
    return nlohmann::json::to_msgpack(j);
}

template <>
inline void deserialize_msg<z_sensor_msgs::z_Imu>(const std::vector<uint8_t>& buffer, z_sensor_msgs::z_Imu& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.header.stamp.sec = j["header"]["stamp"]["sec"].get<int32_t>();
    msg.header.stamp.nanosec = j["header"]["stamp"]["nanosec"].get<uint32_t>();
    msg.header.frame_id = j["header"]["frame_id"].get<std::string>();

    msg.orientation.x = j["orientation"]["x"].get<double>();
    msg.orientation.y = j["orientation"]["y"].get<double>();
    msg.orientation.z = j["orientation"]["z"].get<double>();
    msg.orientation.w = j["orientation"]["w"].get<double>();

    msg.angular_velocity.x = j["angular_velocity"]["x"].get<double>();
    msg.angular_velocity.y = j["angular_velocity"]["y"].get<double>();
    msg.angular_velocity.z = j["angular_velocity"]["z"].get<double>();

    msg.linear_acceleration.x = j["linear_acceleration"]["x"].get<double>();
    msg.linear_acceleration.y = j["linear_acceleration"]["y"].get<double>();
    msg.linear_acceleration.z = j["linear_acceleration"]["z"].get<double>();
}

template <>
inline std::vector<uint8_t> serialize_msg<z_sensor_msgs::z_Temperature>(const z_sensor_msgs::z_Temperature& msg) {
    nlohmann::json j;
    j["header"] = {{"stamp", {{"sec", msg.header.stamp.sec}, {"nanosec", msg.header.stamp.nanosec}}}, {"frame_id", msg.header.frame_id}};
    j["temperature"] = msg.temperature;
    j["variance"] = msg.variance;
    return nlohmann::json::to_msgpack(j);
}

template <>
inline void deserialize_msg<z_sensor_msgs::z_Temperature>(const std::vector<uint8_t>& buffer, z_sensor_msgs::z_Temperature& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.header.stamp.sec = j["header"]["stamp"]["sec"].get<int32_t>();
    msg.header.stamp.nanosec = j["header"]["stamp"]["nanosec"].get<uint32_t>();
    msg.header.frame_id = j["header"]["frame_id"].get<std::string>();
    msg.temperature = j["temperature"].get<double>();
    msg.variance = j["variance"].get<double>();
}

template <>
inline std::vector<uint8_t> serialize_msg<z_sensor_msgs::z_Range>(const z_sensor_msgs::z_Range& msg) {
    nlohmann::json j;
    j["header"] = {{"stamp", {{"sec", msg.header.stamp.sec}, {"nanosec", msg.header.stamp.nanosec}}}, {"frame_id", msg.header.frame_id}};
    j["radiation_type"] = msg.radiation_type;
    j["field_of_view"] = msg.field_of_view;
    j["min_range"] = msg.min_range;
    j["max_range"] = msg.max_range;
    j["range"] = msg.range;
    return nlohmann::json::to_msgpack(j);
}

template <>
inline void deserialize_msg<z_sensor_msgs::z_Range>(const std::vector<uint8_t>& buffer, z_sensor_msgs::z_Range& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.header.stamp.sec = j["header"]["stamp"]["sec"].get<int32_t>();
    msg.header.stamp.nanosec = j["header"]["stamp"]["nanosec"].get<uint32_t>();
    msg.header.frame_id = j["header"]["frame_id"].get<std::string>();
    msg.radiation_type = j["radiation_type"].get<uint8_t>();
    msg.field_of_view = j["field_of_view"].get<float>();
    msg.min_range = j["min_range"].get<float>();
    msg.max_range = j["max_range"].get<float>();
    msg.range = j["range"].get<float>();
}


namespace z_sensor_msgs {
    struct z_RelativeHumidity {
        z_std_msgs::z_Header header;
        double relative_humidity = 0.0;
        double variance = 0.0;
    };

    struct z_BatteryState {
        z_std_msgs::z_Header header;
        float voltage = 0.0f; float temperature = 0.0f; float current = 0.0f;
        float charge = 0.0f;  float capacity = 0.0f;   float design_capacity = 0.0f;
        float percentage = 0.0f;
        uint8_t power_supply_status = 0; uint8_t power_supply_health = 0; uint8_t power_supply_technology = 0;
        bool present = true;
    };

    struct z_NavSatStatus {
        int8_t   status  = -1;
        uint16_t service = 1;
    };

    struct z_NavSatFix {
        z_std_msgs::z_Header header;
        z_NavSatStatus status;
        double latitude = 0.0; double longitude = 0.0; double altitude = 0.0;
        double position_covariance[9] = {0};
        uint8_t position_covariance_type = 0;
    };

    struct z_JoyFeedback {
        uint8_t type = 0; uint8_t id = 0; float intensity = 0.0f;
    };
}

template <>
inline std::vector<uint8_t> serialize_msg<z_sensor_msgs::z_JointState>(const z_sensor_msgs::z_JointState& msg) {
    nlohmann::json j;
    j["header"] = {{"stamp", {{"sec", msg.header.stamp.sec}, {"nanosec", msg.header.stamp.nanosec}}}, {"frame_id", msg.header.frame_id}};
    j["name"] = msg.name; j["position"] = msg.position; j["velocity"] = msg.velocity; j["effort"] = msg.effort;
    return nlohmann::json::to_msgpack(j);
}
template <>
inline void deserialize_msg<z_sensor_msgs::z_JointState>(const std::vector<uint8_t>& buffer, z_sensor_msgs::z_JointState& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.header.stamp.sec = j["header"]["stamp"]["sec"].get<int32_t>();
    msg.header.stamp.nanosec = j["header"]["stamp"]["nanosec"].get<uint32_t>();
    msg.header.frame_id = j["header"]["frame_id"].get<std::string>();
    msg.name = j["name"].get<std::vector<std::string>>();
    msg.position = j["position"].get<std::vector<double>>();
    msg.velocity = j["velocity"].get<std::vector<double>>();
    msg.effort = j["effort"].get<std::vector<double>>();
}

template <>
inline std::vector<uint8_t> serialize_msg<z_sensor_msgs::z_RelativeHumidity>(const z_sensor_msgs::z_RelativeHumidity& msg) {
    nlohmann::json j;
    j["header"] = {{"stamp", {{"sec", msg.header.stamp.sec}, {"nanosec", msg.header.stamp.nanosec}}}, {"frame_id", msg.header.frame_id}};
    j["relative_humidity"] = msg.relative_humidity; j["variance"] = msg.variance;
    return nlohmann::json::to_msgpack(j);
}
template <>
inline void deserialize_msg<z_sensor_msgs::z_RelativeHumidity>(const std::vector<uint8_t>& buffer, z_sensor_msgs::z_RelativeHumidity& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.header.stamp.sec = j["header"]["stamp"]["sec"].get<int32_t>();
    msg.header.stamp.nanosec = j["header"]["stamp"]["nanosec"].get<uint32_t>();
    msg.header.frame_id = j["header"]["frame_id"].get<std::string>();
    msg.relative_humidity = j["relative_humidity"].get<double>();
    msg.variance = j["variance"].get<double>();
}

template <>
inline std::vector<uint8_t> serialize_msg<z_sensor_msgs::z_BatteryState>(const z_sensor_msgs::z_BatteryState& msg) {
    nlohmann::json j;
    j["header"] = {{"stamp", {{"sec", msg.header.stamp.sec}, {"nanosec", msg.header.stamp.nanosec}}}, {"frame_id", msg.header.frame_id}};
    j["voltage"] = msg.voltage; j["percentage"] = msg.percentage; j["power_supply_status"] = msg.power_supply_status; j["present"] = msg.present;
    return nlohmann::json::to_msgpack(j);
}
template <>
inline void deserialize_msg<z_sensor_msgs::z_BatteryState>(const std::vector<uint8_t>& buffer, z_sensor_msgs::z_BatteryState& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.header.stamp.sec = j["header"]["stamp"]["sec"].get<int32_t>();
    msg.header.stamp.nanosec = j["header"]["stamp"]["nanosec"].get<uint32_t>();
    msg.header.frame_id = j["header"]["frame_id"].get<std::string>();
    msg.voltage = j["voltage"].get<float>();
    msg.percentage = j["percentage"].get<float>();
    msg.power_supply_status = j["power_supply_status"].get<uint8_t>();
    msg.present = j["present"].get<bool>();
}

template <>
inline std::vector<uint8_t> serialize_msg<z_sensor_msgs::z_NavSatFix>(const z_sensor_msgs::z_NavSatFix& msg) {
    nlohmann::json j;
    j["header"] = {{"stamp", {{"sec", msg.header.stamp.sec}, {"nanosec", msg.header.stamp.nanosec}}}, {"frame_id", msg.header.frame_id}};
    j["status"] = {{"status", msg.status.status}, {"service", msg.status.service}};
    j["latitude"] = msg.latitude; j["longitude"] = msg.longitude; j["altitude"] = msg.altitude;
    return nlohmann::json::to_msgpack(j);
}
template <>
inline void deserialize_msg<z_sensor_msgs::z_NavSatFix>(const std::vector<uint8_t>& buffer, z_sensor_msgs::z_NavSatFix& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.header.stamp.sec = j["header"]["stamp"]["sec"].get<int32_t>();
    msg.header.stamp.nanosec = j["header"]["stamp"]["nanosec"].get<uint32_t>();
    msg.header.frame_id = j["header"]["frame_id"].get<std::string>();
    msg.status.status = j["status"]["status"].get<int8_t>();
    msg.status.service = j["status"]["service"].get<uint16_t>();
    msg.latitude = j["latitude"].get<double>();
    msg.longitude = j["longitude"].get<double>();
    msg.altitude = j["altitude"].get<double>();
}

template <>
inline std::vector<uint8_t> serialize_msg<z_sensor_msgs::z_JoyFeedback>(const z_sensor_msgs::z_JoyFeedback& msg) {
    nlohmann::json j; j["type"] = msg.type; j["id"] = msg.id; j["intensity"] = msg.intensity;
    return nlohmann::json::to_msgpack(j);
}
template <>
inline void deserialize_msg<z_sensor_msgs::z_JoyFeedback>(const std::vector<uint8_t>& buffer, z_sensor_msgs::z_JoyFeedback& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.type = j["type"].get<uint8_t>();
    msg.id = j["id"].get<uint8_t>();
    msg.intensity = j["intensity"].get<float>();
}

#endif // Z_SENSOR_MSGS_H
