#ifndef Z_NAV_MSGS_H
#define Z_NAV_MSGS_H

#include <nlohmann/json.hpp>
#include <vector>
#include "z_std_msgs.h"
#include "z_geometry_msgs.h"

namespace z_nav_msgs {

    struct z_Odometry {
        z_std_msgs::z_Header header;
        std::string child_frame_id;
        z_geometry_msgs::z_Pose pose;
        double pose_covariance[36] = {0};
        z_geometry_msgs::z_Twist twist;
        double twist_covariance[36] = {0};
    };

    struct z_Path {
        z_std_msgs::z_Header header;
        std::vector<z_geometry_msgs::z_Pose> poses;
    };

} // namespace z_nav_msgs

// --- Serialization Specializations ---

template <>
inline size_t serialize_msg<z_nav_msgs::z_Odometry>(const z_nav_msgs::z_Odometry& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["header"]["stamp"]["sec"] = msg.header.stamp.sec;
    doc["header"]["stamp"]["nanosec"] = msg.header.stamp.nanosec;
    doc["header"]["frame_id"] = msg.header.frame_id;
    doc["child_frame_id"] = msg.child_frame_id;

    JsonObject pos = doc["pose"]["pose"]["position"].to<JsonObject>();
    pos["x"] = msg.pose.position.x; pos["y"] = msg.pose.position.y; pos["z"] = msg.pose.position.z;
    JsonObject ori = doc["pose"]["pose"]["orientation"].to<JsonObject>();
    ori["x"] = msg.pose.orientation.x; ori["y"] = msg.pose.orientation.y; ori["z"] = msg.pose.orientation.z; ori["w"] = msg.pose.orientation.w;

    JsonObject lin = doc["twist"]["twist"]["linear"].to<JsonObject>();
    lin["x"] = msg.twist.linear.x; lin["y"] = msg.twist.linear.y; lin["z"] = msg.twist.linear.z;
    JsonObject ang = doc["twist"]["twist"]["angular"].to<JsonObject>();
    ang["x"] = msg.twist.angular.x; ang["y"] = msg.twist.angular.y; ang["z"] = msg.twist.angular.z;

    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline void deserialize_msg<z_nav_msgs::z_Odometry>(const uint8_t* buffer, size_t len, z_nav_msgs::z_Odometry& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.header.stamp.sec = doc["header"]["stamp"]["sec"].as<int32_t>();
    msg.header.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>();
    msg.header.frame_id = doc["header"]["frame_id"].as<std::string>();
    msg.child_frame_id = doc["child_frame_id"].as<std::string>();

    msg.pose.position.x = doc["pose"]["pose"]["position"]["x"].as<double>();
    msg.pose.position.y = doc["pose"]["pose"]["position"]["y"].as<double>();
    msg.pose.position.z = doc["pose"]["pose"]["position"]["z"].as<double>();
    msg.pose.orientation.x = doc["pose"]["pose"]["orientation"]["x"].as<double>();
    msg.pose.orientation.y = doc["pose"]["pose"]["orientation"]["y"].as<double>();
    msg.pose.orientation.z = doc["pose"]["pose"]["orientation"]["z"].as<double>();
    msg.pose.orientation.w = doc["pose"]["pose"]["orientation"]["w"].as<double>();

    msg.twist.linear.x = doc["twist"]["twist"]["linear"]["x"].as<double>();
    msg.twist.linear.y = doc["twist"]["twist"]["linear"]["y"].as<double>();
    msg.twist.linear.z = doc["twist"]["twist"]["linear"]["z"].as<double>();
    msg.twist.angular.x = doc["twist"]["twist"]["angular"]["x"].as<double>();
    msg.twist.angular.y = doc["twist"]["twist"]["angular"]["y"].as<double>();
    msg.twist.angular.z = doc["twist"]["twist"]["angular"]["z"].as<double>();
}

// ─── NEW types for ROS 2 Jazzy parity ────────────────────────────────────────
namespace z_nav_msgs {

    struct z_MapMetaData {
        builtin_interfaces::z_Time map_load_time;
        float resolution = 0.0f;
        uint32_t width = 0;
        uint32_t height = 0;
        z_geometry_msgs::z_Pose origin;
    };

    struct z_GridCells {
        z_std_msgs::z_Header header;
        float cell_width = 0.0f;
        float cell_height = 0.0f;
        std::vector<z_geometry_msgs::z_Point> cells;
    };

} // namespace z_nav_msgs (extension)

template <>
inline size_t serialize_msg<z_nav_msgs::z_Path>(const z_nav_msgs::z_Path& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["header"]["stamp"]["sec"] = msg.header.stamp.sec; doc["header"]["stamp"]["nanosec"] = msg.header.stamp.nanosec; doc["header"]["frame_id"] = msg.header.frame_id;
    JsonArray poses = doc["poses"].to<JsonArray>();
    for (const auto& p : msg.poses) {
        JsonArray pose_arr = poses.add<JsonArray>();
        JsonArray pos_arr = pose_arr.add<JsonArray>(); pos_arr.add(p.position.x); pos_arr.add(p.position.y); pos_arr.add(p.position.z);
        JsonArray ori_arr = pose_arr.add<JsonArray>(); ori_arr.add(p.orientation.x); ori_arr.add(p.orientation.y); ori_arr.add(p.orientation.z); ori_arr.add(p.orientation.w);
    }
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_nav_msgs::z_Path>(const uint8_t* buffer, size_t len, z_nav_msgs::z_Path& msg) {
    JsonDocument doc; deserializeMsgPack(doc, buffer, len);
    msg.header.stamp.sec = doc["header"]["stamp"]["sec"].as<int32_t>(); msg.header.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>(); msg.header.frame_id = doc["header"]["frame_id"].as<std::string>();
    msg.poses.clear();
    for (auto pose_arr : doc["poses"].as<JsonArrayConst>()) {
        z_geometry_msgs::z_Pose p;
        p.position.x = pose_arr[0][0].as<double>(); p.position.y = pose_arr[0][1].as<double>(); p.position.z = pose_arr[0][2].as<double>();
        p.orientation.x = pose_arr[1][0].as<double>(); p.orientation.y = pose_arr[1][1].as<double>(); p.orientation.z = pose_arr[1][2].as<double>(); p.orientation.w = pose_arr[1][3].as<double>();
        msg.poses.push_back(p);
    }
}

template <>
inline size_t serialize_msg<z_nav_msgs::z_MapMetaData>(const z_nav_msgs::z_MapMetaData& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["map_load_time"]["sec"] = msg.map_load_time.sec; doc["map_load_time"]["nanosec"] = msg.map_load_time.nanosec;
    doc["resolution"] = msg.resolution; doc["width"] = msg.width; doc["height"] = msg.height;
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_nav_msgs::z_MapMetaData>(const uint8_t* buffer, size_t len, z_nav_msgs::z_MapMetaData& msg) {
    JsonDocument doc; deserializeMsgPack(doc, buffer, len);
    msg.map_load_time.sec = doc["map_load_time"]["sec"].as<int32_t>(); msg.map_load_time.nanosec = doc["map_load_time"]["nanosec"].as<uint32_t>();
    msg.resolution = doc["resolution"].as<float>(); msg.width = doc["width"].as<uint32_t>(); msg.height = doc["height"].as<uint32_t>();
}

#endif // Z_NAV_MSGS_H
