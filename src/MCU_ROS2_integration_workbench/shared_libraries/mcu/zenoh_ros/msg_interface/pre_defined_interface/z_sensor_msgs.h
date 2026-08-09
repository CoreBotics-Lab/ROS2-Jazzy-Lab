#ifndef Z_SENSOR_MSGS_H
#define Z_SENSOR_MSGS_H

#include <ArduinoJson.h>
#include <vector>
#include <string>
#include "z_std_msgs.h"
#include "z_geometry_msgs.h"
#include "z_builtin_interfaces.h"

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
inline size_t serialize_msg<z_sensor_msgs::z_Imu>(const z_sensor_msgs::z_Imu& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    JsonObject header_obj = doc["header"].to<JsonObject>();
    header_obj["stamp"]["sec"] = msg.header.stamp.sec;
    header_obj["stamp"]["nanosec"] = msg.header.stamp.nanosec;
    header_obj["frame_id"] = msg.header.frame_id;

    JsonObject ori = doc["orientation"].to<JsonObject>();
    ori["x"] = msg.orientation.x; ori["y"] = msg.orientation.y; ori["z"] = msg.orientation.z; ori["w"] = msg.orientation.w;

    JsonObject ang = doc["angular_velocity"].to<JsonObject>();
    ang["x"] = msg.angular_velocity.x; ang["y"] = msg.angular_velocity.y; ang["z"] = msg.angular_velocity.z;

    JsonObject lin = doc["linear_acceleration"].to<JsonObject>();
    lin["x"] = msg.linear_acceleration.x; lin["y"] = msg.linear_acceleration.y; lin["z"] = msg.linear_acceleration.z;

    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline void deserialize_msg<z_sensor_msgs::z_Imu>(const uint8_t* buffer, size_t len, z_sensor_msgs::z_Imu& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.header.stamp.sec = doc["header"]["stamp"]["sec"].as<int32_t>();
    msg.header.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>();
    msg.header.frame_id = doc["header"]["frame_id"].as<std::string>();

    msg.orientation.x = doc["orientation"]["x"].as<double>();
    msg.orientation.y = doc["orientation"]["y"].as<double>();
    msg.orientation.z = doc["orientation"]["z"].as<double>();
    msg.orientation.w = doc["orientation"]["w"].as<double>();

    msg.angular_velocity.x = doc["angular_velocity"]["x"].as<double>();
    msg.angular_velocity.y = doc["angular_velocity"]["y"].as<double>();
    msg.angular_velocity.z = doc["angular_velocity"]["z"].as<double>();

    msg.linear_acceleration.x = doc["linear_acceleration"]["x"].as<double>();
    msg.linear_acceleration.y = doc["linear_acceleration"]["y"].as<double>();
    msg.linear_acceleration.z = doc["linear_acceleration"]["z"].as<double>();
}

template <>
inline size_t serialize_msg<z_sensor_msgs::z_Temperature>(const z_sensor_msgs::z_Temperature& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["header"]["stamp"]["sec"] = msg.header.stamp.sec;
    doc["header"]["stamp"]["nanosec"] = msg.header.stamp.nanosec;
    doc["header"]["frame_id"] = msg.header.frame_id;
    doc["temperature"] = msg.temperature;
    doc["variance"] = msg.variance;
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline void deserialize_msg<z_sensor_msgs::z_Temperature>(const uint8_t* buffer, size_t len, z_sensor_msgs::z_Temperature& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.header.stamp.sec = doc["header"]["stamp"]["sec"].as<int32_t>();
    msg.header.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>();
    msg.header.frame_id = doc["header"]["frame_id"].as<std::string>();
    msg.temperature = doc["temperature"].as<double>();
    msg.variance = doc["variance"].as<double>();
}

template <>
inline size_t serialize_msg<z_sensor_msgs::z_Range>(const z_sensor_msgs::z_Range& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["header"]["stamp"]["sec"] = msg.header.stamp.sec;
    doc["header"]["stamp"]["nanosec"] = msg.header.stamp.nanosec;
    doc["header"]["frame_id"] = msg.header.frame_id;
    doc["radiation_type"] = msg.radiation_type;
    doc["field_of_view"] = msg.field_of_view;
    doc["min_range"] = msg.min_range;
    doc["max_range"] = msg.max_range;
    doc["range"] = msg.range;
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline void deserialize_msg<z_sensor_msgs::z_Range>(const uint8_t* buffer, size_t len, z_sensor_msgs::z_Range& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.header.stamp.sec = doc["header"]["stamp"]["sec"].as<int32_t>();
    msg.header.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>();
    msg.header.frame_id = doc["header"]["frame_id"].as<std::string>();
    msg.radiation_type = doc["radiation_type"].as<uint8_t>();
    msg.field_of_view = doc["field_of_view"].as<float>();
    msg.min_range = doc["min_range"].as<float>();
    msg.max_range = doc["max_range"].as<float>();
    msg.range = doc["range"].as<float>();
}

// ─── NEW types for ROS 2 Jazzy parity ────────────────────────────────────────

namespace z_sensor_msgs {

    struct z_RelativeHumidity {
        z_std_msgs::z_Header header;
        double relative_humidity = 0.0;
        double variance = 0.0;
    };

    struct z_RegionOfInterest {
        uint32_t x_offset = 0; uint32_t y_offset = 0;
        uint32_t height = 0;   uint32_t width = 0;
        bool do_rectify = false;
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

    struct z_BatteryState {
        z_std_msgs::z_Header header;
        float voltage = 0.0f; float temperature = 0.0f; float current = 0.0f;
        float charge = 0.0f;  float capacity = 0.0f;   float design_capacity = 0.0f;
        float percentage = 0.0f;
        uint8_t power_supply_status = 0; uint8_t power_supply_health = 0; uint8_t power_supply_technology = 0;
        bool present = true;
    };

    struct z_JoyFeedback {
        uint8_t type = 0; uint8_t id = 0; float intensity = 0.0f;
    };

    struct z_TimeReference {
        z_std_msgs::z_Header header;
        builtin_interfaces::z_Time time_ref;
        std::string source;
    };

    // Struct-only on MCU (no serialization)
    struct z_Joy {
        z_std_msgs::z_Header header;
        std::vector<float> axes;
        std::vector<int32_t> buttons;
    };

    struct z_LaserScan {
        z_std_msgs::z_Header header;
        float angle_min = 0.0f; float angle_max = 0.0f; float angle_increment = 0.0f;
        float time_increment = 0.0f; float scan_time = 0.0f;
        float range_min = 0.0f; float range_max = 0.0f;
        std::vector<float> ranges; std::vector<float> intensities;
    };

} // namespace z_sensor_msgs (extension)

// ─── Serialization for NEW types only ────────────────────────────────────────

// JointState (was struct-only before — now adding serialization)
template <>
inline size_t serialize_msg<z_sensor_msgs::z_JointState>(const z_sensor_msgs::z_JointState& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["header"]["stamp"]["sec"] = msg.header.stamp.sec; doc["header"]["stamp"]["nanosec"] = msg.header.stamp.nanosec; doc["header"]["frame_id"] = msg.header.frame_id;
    JsonArray names = doc["name"].to<JsonArray>(); for (const auto& n : msg.name) names.add(n);
    JsonArray pos = doc["position"].to<JsonArray>(); for (auto v : msg.position) pos.add(v);
    JsonArray vel = doc["velocity"].to<JsonArray>(); for (auto v : msg.velocity) vel.add(v);
    JsonArray eff = doc["effort"].to<JsonArray>();   for (auto v : msg.effort)   eff.add(v);
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_sensor_msgs::z_JointState>(const uint8_t* buffer, size_t len, z_sensor_msgs::z_JointState& msg) {
    JsonDocument doc; deserializeMsgPack(doc, buffer, len);
    msg.header.stamp.sec = doc["header"]["stamp"]["sec"].as<int32_t>(); msg.header.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>(); msg.header.frame_id = doc["header"]["frame_id"].as<std::string>();
    msg.name.clear(); for (auto n : doc["name"].as<JsonArrayConst>()) msg.name.push_back(n.as<std::string>());
    msg.position.clear(); for (auto v : doc["position"].as<JsonArrayConst>()) msg.position.push_back(v.as<double>());
    msg.velocity.clear(); for (auto v : doc["velocity"].as<JsonArrayConst>()) msg.velocity.push_back(v.as<double>());
    msg.effort.clear();   for (auto v : doc["effort"].as<JsonArrayConst>())   msg.effort.push_back(v.as<double>());
}

template <>
inline size_t serialize_msg<z_sensor_msgs::z_RelativeHumidity>(const z_sensor_msgs::z_RelativeHumidity& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["header"]["stamp"]["sec"] = msg.header.stamp.sec; doc["header"]["stamp"]["nanosec"] = msg.header.stamp.nanosec; doc["header"]["frame_id"] = msg.header.frame_id;
    doc["relative_humidity"] = msg.relative_humidity; doc["variance"] = msg.variance;
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_sensor_msgs::z_RelativeHumidity>(const uint8_t* buffer, size_t len, z_sensor_msgs::z_RelativeHumidity& msg) {
    JsonDocument doc; deserializeMsgPack(doc, buffer, len);
    msg.header.stamp.sec = doc["header"]["stamp"]["sec"].as<int32_t>(); msg.header.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>(); msg.header.frame_id = doc["header"]["frame_id"].as<std::string>();
    msg.relative_humidity = doc["relative_humidity"].as<double>(); msg.variance = doc["variance"].as<double>();
}

template <>
inline size_t serialize_msg<z_sensor_msgs::z_RegionOfInterest>(const z_sensor_msgs::z_RegionOfInterest& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["x_offset"] = msg.x_offset; doc["y_offset"] = msg.y_offset; doc["height"] = msg.height; doc["width"] = msg.width; doc["do_rectify"] = msg.do_rectify;
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_sensor_msgs::z_RegionOfInterest>(const uint8_t* buffer, size_t len, z_sensor_msgs::z_RegionOfInterest& msg) {
    JsonDocument doc; deserializeMsgPack(doc, buffer, len);
    msg.x_offset = doc["x_offset"].as<uint32_t>(); msg.y_offset = doc["y_offset"].as<uint32_t>(); msg.height = doc["height"].as<uint32_t>(); msg.width = doc["width"].as<uint32_t>(); msg.do_rectify = doc["do_rectify"].as<bool>();
}

template <>
inline size_t serialize_msg<z_sensor_msgs::z_NavSatStatus>(const z_sensor_msgs::z_NavSatStatus& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc; doc["status"] = msg.status; doc["service"] = msg.service;
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_sensor_msgs::z_NavSatStatus>(const uint8_t* buffer, size_t len, z_sensor_msgs::z_NavSatStatus& msg) {
    JsonDocument doc; deserializeMsgPack(doc, buffer, len);
    msg.status = doc["status"].as<int8_t>(); msg.service = doc["service"].as<uint16_t>();
}

template <>
inline size_t serialize_msg<z_sensor_msgs::z_NavSatFix>(const z_sensor_msgs::z_NavSatFix& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["header"]["stamp"]["sec"] = msg.header.stamp.sec; doc["header"]["stamp"]["nanosec"] = msg.header.stamp.nanosec; doc["header"]["frame_id"] = msg.header.frame_id;
    doc["status"]["status"] = msg.status.status; doc["status"]["service"] = msg.status.service;
    doc["latitude"] = msg.latitude; doc["longitude"] = msg.longitude; doc["altitude"] = msg.altitude;
    doc["position_covariance_type"] = msg.position_covariance_type;
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_sensor_msgs::z_NavSatFix>(const uint8_t* buffer, size_t len, z_sensor_msgs::z_NavSatFix& msg) {
    JsonDocument doc; deserializeMsgPack(doc, buffer, len);
    msg.header.stamp.sec = doc["header"]["stamp"]["sec"].as<int32_t>(); msg.header.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>(); msg.header.frame_id = doc["header"]["frame_id"].as<std::string>();
    msg.status.status = doc["status"]["status"].as<int8_t>(); msg.status.service = doc["status"]["service"].as<uint16_t>();
    msg.latitude = doc["latitude"].as<double>(); msg.longitude = doc["longitude"].as<double>(); msg.altitude = doc["altitude"].as<double>();
    msg.position_covariance_type = doc["position_covariance_type"].as<uint8_t>();
}

template <>
inline size_t serialize_msg<z_sensor_msgs::z_BatteryState>(const z_sensor_msgs::z_BatteryState& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["header"]["stamp"]["sec"] = msg.header.stamp.sec; doc["header"]["stamp"]["nanosec"] = msg.header.stamp.nanosec; doc["header"]["frame_id"] = msg.header.frame_id;
    doc["voltage"] = msg.voltage; doc["temperature"] = msg.temperature; doc["current"] = msg.current;
    doc["charge"] = msg.charge; doc["capacity"] = msg.capacity; doc["design_capacity"] = msg.design_capacity;
    doc["percentage"] = msg.percentage; doc["power_supply_status"] = msg.power_supply_status;
    doc["power_supply_health"] = msg.power_supply_health; doc["power_supply_technology"] = msg.power_supply_technology;
    doc["present"] = msg.present;
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_sensor_msgs::z_BatteryState>(const uint8_t* buffer, size_t len, z_sensor_msgs::z_BatteryState& msg) {
    JsonDocument doc; deserializeMsgPack(doc, buffer, len);
    msg.header.stamp.sec = doc["header"]["stamp"]["sec"].as<int32_t>(); msg.header.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>(); msg.header.frame_id = doc["header"]["frame_id"].as<std::string>();
    msg.voltage = doc["voltage"].as<float>(); msg.temperature = doc["temperature"].as<float>(); msg.current = doc["current"].as<float>();
    msg.charge = doc["charge"].as<float>(); msg.capacity = doc["capacity"].as<float>(); msg.design_capacity = doc["design_capacity"].as<float>();
    msg.percentage = doc["percentage"].as<float>(); msg.power_supply_status = doc["power_supply_status"].as<uint8_t>();
    msg.power_supply_health = doc["power_supply_health"].as<uint8_t>(); msg.power_supply_technology = doc["power_supply_technology"].as<uint8_t>();
    msg.present = doc["present"].as<bool>();
}

template <>
inline size_t serialize_msg<z_sensor_msgs::z_JoyFeedback>(const z_sensor_msgs::z_JoyFeedback& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc; doc["type"] = msg.type; doc["id"] = msg.id; doc["intensity"] = msg.intensity;
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_sensor_msgs::z_JoyFeedback>(const uint8_t* buffer, size_t len, z_sensor_msgs::z_JoyFeedback& msg) {
    JsonDocument doc; deserializeMsgPack(doc, buffer, len);
    msg.type = doc["type"].as<uint8_t>(); msg.id = doc["id"].as<uint8_t>(); msg.intensity = doc["intensity"].as<float>();
}

template <>
inline size_t serialize_msg<z_sensor_msgs::z_TimeReference>(const z_sensor_msgs::z_TimeReference& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["header"]["stamp"]["sec"] = msg.header.stamp.sec; doc["header"]["stamp"]["nanosec"] = msg.header.stamp.nanosec; doc["header"]["frame_id"] = msg.header.frame_id;
    doc["time_ref"]["sec"] = msg.time_ref.sec; doc["time_ref"]["nanosec"] = msg.time_ref.nanosec;
    doc["source"] = msg.source;
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_sensor_msgs::z_TimeReference>(const uint8_t* buffer, size_t len, z_sensor_msgs::z_TimeReference& msg) {
    JsonDocument doc; deserializeMsgPack(doc, buffer, len);
    msg.header.stamp.sec = doc["header"]["stamp"]["sec"].as<int32_t>(); msg.header.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>(); msg.header.frame_id = doc["header"]["frame_id"].as<std::string>();
    msg.time_ref.sec = doc["time_ref"]["sec"].as<int32_t>(); msg.time_ref.nanosec = doc["time_ref"]["nanosec"].as<uint32_t>();
    msg.source = doc["source"].as<std::string>();
}

// z_Joy and z_LaserScan: struct-only on MCU, no serialize_msg specialization.

#endif // Z_SENSOR_MSGS_H
