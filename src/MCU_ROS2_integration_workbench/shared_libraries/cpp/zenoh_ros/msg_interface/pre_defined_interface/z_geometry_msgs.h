#ifndef Z_GEOMETRY_MSGS_PC_H
#define Z_GEOMETRY_MSGS_PC_H

#include <nlohmann/json.hpp>
#include <vector>

// Forward declarations of serialization templates
template <typename T>
std::vector<uint8_t> serialize_msg(const T& msg);

template <typename T>
void deserialize_msg(const std::vector<uint8_t>& buffer, T& msg);

namespace z_geometry_msgs {

    struct z_Vector3 {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
    };

    struct z_Point {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
    };

    struct z_Quaternion {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
        double w = 1.0;
    };

    struct z_Pose {
        z_Point position;
        z_Quaternion orientation;
    };

    struct z_Twist {
        z_Vector3 linear;
        z_Vector3 angular;
    };

    struct z_Wrench {
        z_Vector3 force;
        z_Vector3 torque;
    };

    struct z_Transform {
        z_Vector3 translation;
        z_Quaternion rotation;
    };

    struct z_Pose2D {
        double x = 0.0;
        double y = 0.0;
        double theta = 0.0;
    };

} // namespace z_geometry_msgs

// --- Serialization Specializations ---

template <>
inline std::vector<uint8_t> serialize_msg<z_geometry_msgs::z_Vector3>(const z_geometry_msgs::z_Vector3& msg) {
    nlohmann::json j = {msg.x, msg.y, msg.z};
    return nlohmann::json::to_msgpack(j);
}

template <>
inline std::vector<uint8_t> serialize_msg<z_geometry_msgs::z_Point>(const z_geometry_msgs::z_Point& msg) {
    nlohmann::json j = {msg.x, msg.y, msg.z};
    return nlohmann::json::to_msgpack(j);
}

template <>
inline std::vector<uint8_t> serialize_msg<z_geometry_msgs::z_Quaternion>(const z_geometry_msgs::z_Quaternion& msg) {
    nlohmann::json j = {msg.x, msg.y, msg.z, msg.w};
    return nlohmann::json::to_msgpack(j);
}

template <>
inline std::vector<uint8_t> serialize_msg<z_geometry_msgs::z_Pose>(const z_geometry_msgs::z_Pose& msg) {
    nlohmann::json j = {
        {msg.position.x, msg.position.y, msg.position.z},
        {msg.orientation.x, msg.orientation.y, msg.orientation.z, msg.orientation.w}
    };
    return nlohmann::json::to_msgpack(j);
}

template <>
inline std::vector<uint8_t> serialize_msg<z_geometry_msgs::z_Twist>(const z_geometry_msgs::z_Twist& msg) {
    nlohmann::json j = {
        {msg.linear.x, msg.linear.y, msg.linear.z},
        {msg.angular.x, msg.angular.y, msg.angular.z}
    };
    return nlohmann::json::to_msgpack(j);
}

template <>
inline std::vector<uint8_t> serialize_msg<z_geometry_msgs::z_Wrench>(const z_geometry_msgs::z_Wrench& msg) {
    nlohmann::json j = {
        {msg.force.x, msg.force.y, msg.force.z},
        {msg.torque.x, msg.torque.y, msg.torque.z}
    };
    return nlohmann::json::to_msgpack(j);
}

template <>
inline std::vector<uint8_t> serialize_msg<z_geometry_msgs::z_Transform>(const z_geometry_msgs::z_Transform& msg) {
    nlohmann::json j = {
        {msg.translation.x, msg.translation.y, msg.translation.z},
        {msg.rotation.x, msg.rotation.y, msg.rotation.z, msg.rotation.w}
    };
    return nlohmann::json::to_msgpack(j);
}

template <>
inline std::vector<uint8_t> serialize_msg<z_geometry_msgs::z_Pose2D>(const z_geometry_msgs::z_Pose2D& msg) {
    nlohmann::json j = {msg.x, msg.y, msg.theta};
    return nlohmann::json::to_msgpack(j);
}

// --- Deserialization Specializations ---

template <>
inline void deserialize_msg<z_geometry_msgs::z_Vector3>(const std::vector<uint8_t>& buffer, z_geometry_msgs::z_Vector3& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.x = j[0].get<double>();
    msg.y = j[1].get<double>();
    msg.z = j[2].get<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::z_Point>(const std::vector<uint8_t>& buffer, z_geometry_msgs::z_Point& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.x = j[0].get<double>();
    msg.y = j[1].get<double>();
    msg.z = j[2].get<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::z_Quaternion>(const std::vector<uint8_t>& buffer, z_geometry_msgs::z_Quaternion& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.x = j[0].get<double>();
    msg.y = j[1].get<double>();
    msg.z = j[2].get<double>();
    msg.w = j[3].get<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::z_Pose>(const std::vector<uint8_t>& buffer, z_geometry_msgs::z_Pose& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.position.x = j[0][0].get<double>();
    msg.position.y = j[0][1].get<double>();
    msg.position.z = j[0][2].get<double>();
    
    msg.orientation.x = j[1][0].get<double>();
    msg.orientation.y = j[1][1].get<double>();
    msg.orientation.z = j[1][2].get<double>();
    msg.orientation.w = j[1][3].get<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::z_Twist>(const std::vector<uint8_t>& buffer, z_geometry_msgs::z_Twist& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.linear.x = j[0][0].get<double>();
    msg.linear.y = j[0][1].get<double>();
    msg.linear.z = j[0][2].get<double>();
    
    msg.angular.x = j[1][0].get<double>();
    msg.angular.y = j[1][1].get<double>();
    msg.angular.z = j[1][2].get<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::z_Wrench>(const std::vector<uint8_t>& buffer, z_geometry_msgs::z_Wrench& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.force.x = j[0][0].get<double>();
    msg.force.y = j[0][1].get<double>();
    msg.force.z = j[0][2].get<double>();
    
    msg.torque.x = j[1][0].get<double>();
    msg.torque.y = j[1][1].get<double>();
    msg.torque.z = j[1][2].get<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::z_Transform>(const std::vector<uint8_t>& buffer, z_geometry_msgs::z_Transform& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.translation.x = j[0][0].get<double>();
    msg.translation.y = j[0][1].get<double>();
    msg.translation.z = j[0][2].get<double>();
    
    msg.rotation.x = j[1][0].get<double>();
    msg.rotation.y = j[1][1].get<double>();
    msg.rotation.z = j[1][2].get<double>();
    msg.rotation.w = j[1][3].get<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::z_Pose2D>(const std::vector<uint8_t>& buffer, z_geometry_msgs::z_Pose2D& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.x = j[0].get<double>();
    msg.y = j[1].get<double>();
    msg.theta = j[2].get<double>();
}

// ─── NEW types for ROS 2 Jazzy parity ────────────────────────────────────────
#include <string>
#include "z_builtin_interfaces.h"

namespace z_geometry_msgs {

    struct z_Point32   { float  x = 0; float  y = 0; float  z = 0; };
    struct z_Accel     { z_Vector3 linear; z_Vector3 angular; };
    struct z_PoseWithCovariance         { z_Pose pose; std::vector<double> covariance = std::vector<double>(36, 0.0); };
    struct z_TwistWithCovariance        { z_Twist twist; std::vector<double> covariance = std::vector<double>(36, 0.0); };
    struct z_AccelWithCovariance        { z_Accel accel; std::vector<double> covariance = std::vector<double>(36, 0.0); };
    struct z_Inertia   { double m=0; z_Vector3 com; double ixx=0,ixy=0,ixz=0,iyy=0,iyz=0,izz=0; };
    struct z_Polygon   { std::vector<z_Point32> points; };
    struct z_PoseStamped            { builtin_interfaces::z_Time stamp; std::string frame_id; z_Pose pose; };
    struct z_PointStamped           { builtin_interfaces::z_Time stamp; std::string frame_id; z_Point point; };
    struct z_TwistStamped           { builtin_interfaces::z_Time stamp; std::string frame_id; z_Twist twist; };
    struct z_WrenchStamped          { builtin_interfaces::z_Time stamp; std::string frame_id; z_Wrench wrench; };
    struct z_AccelStamped           { builtin_interfaces::z_Time stamp; std::string frame_id; z_Accel accel; };
    struct z_TransformStamped       { builtin_interfaces::z_Time stamp; std::string frame_id; std::string child_frame_id; z_Transform transform; };
    struct z_PolygonStamped         { builtin_interfaces::z_Time stamp; std::string frame_id; z_Polygon polygon; };
    struct z_InertiaStamped         { builtin_interfaces::z_Time stamp; std::string frame_id; z_Inertia inertia; };
    struct z_VelocityStamped        { builtin_interfaces::z_Time stamp; std::string frame_id; std::string reference_frame_id; z_Twist velocity; };
    struct z_PoseWithCovarianceStamped  { builtin_interfaces::z_Time stamp; std::string frame_id; z_PoseWithCovariance pose; };
    struct z_TwistWithCovarianceStamped { builtin_interfaces::z_Time stamp; std::string frame_id; z_TwistWithCovariance twist; };
    struct z_AccelWithCovarianceStamped { builtin_interfaces::z_Time stamp; std::string frame_id; z_AccelWithCovariance accel; };

} // namespace z_geometry_msgs (extension)

// Serialization for new stamped types (PC C++)
template <> inline std::vector<uint8_t> serialize_msg<z_geometry_msgs::z_PoseStamped>(const z_geometry_msgs::z_PoseStamped& msg) {
    nlohmann::json j;
    j["header"] = {{"stamp", {{"sec", msg.stamp.sec}, {"nanosec", msg.stamp.nanosec}}}, {"frame_id", msg.frame_id}};
    j["pose"] = {{"position", {{"x", msg.pose.position.x}, {"y", msg.pose.position.y}, {"z", msg.pose.position.z}}},
                 {"orientation", {{"x", msg.pose.orientation.x}, {"y", msg.pose.orientation.y}, {"z", msg.pose.orientation.z}, {"w", msg.pose.orientation.w}}}};
    auto b = nlohmann::json::to_msgpack(j); return {b.begin(), b.end()};
}
template <> inline void deserialize_msg<z_geometry_msgs::z_PoseStamped>(const std::vector<uint8_t>& buf, z_geometry_msgs::z_PoseStamped& msg) {
    auto j = nlohmann::json::from_msgpack(buf);
    msg.stamp.sec = j["header"]["stamp"]["sec"]; msg.stamp.nanosec = j["header"]["stamp"]["nanosec"]; msg.frame_id = j["header"]["frame_id"].get<std::string>();
    msg.pose.position.x = j["pose"]["position"]["x"]; msg.pose.position.y = j["pose"]["position"]["y"]; msg.pose.position.z = j["pose"]["position"]["z"];
    msg.pose.orientation.x = j["pose"]["orientation"]["x"]; msg.pose.orientation.y = j["pose"]["orientation"]["y"]; msg.pose.orientation.z = j["pose"]["orientation"]["z"]; msg.pose.orientation.w = j["pose"]["orientation"]["w"];
}
template <> inline std::vector<uint8_t> serialize_msg<z_geometry_msgs::z_TwistStamped>(const z_geometry_msgs::z_TwistStamped& msg) {
    nlohmann::json j;
    j["header"] = {{"stamp", {{"sec", msg.stamp.sec}, {"nanosec", msg.stamp.nanosec}}}, {"frame_id", msg.frame_id}};
    j["twist"] = {{"linear", {{"x", msg.twist.linear.x}, {"y", msg.twist.linear.y}, {"z", msg.twist.linear.z}}},
                  {"angular", {{"x", msg.twist.angular.x}, {"y", msg.twist.angular.y}, {"z", msg.twist.angular.z}}}};
    auto b = nlohmann::json::to_msgpack(j); return {b.begin(), b.end()};
}
template <> inline void deserialize_msg<z_geometry_msgs::z_TwistStamped>(const std::vector<uint8_t>& buf, z_geometry_msgs::z_TwistStamped& msg) {
    auto j = nlohmann::json::from_msgpack(buf);
    msg.stamp.sec = j["header"]["stamp"]["sec"]; msg.stamp.nanosec = j["header"]["stamp"]["nanosec"]; msg.frame_id = j["header"]["frame_id"].get<std::string>();
    msg.twist.linear.x = j["twist"]["linear"]["x"]; msg.twist.linear.y = j["twist"]["linear"]["y"]; msg.twist.linear.z = j["twist"]["linear"]["z"];
    msg.twist.angular.x = j["twist"]["angular"]["x"]; msg.twist.angular.y = j["twist"]["angular"]["y"]; msg.twist.angular.z = j["twist"]["angular"]["z"];
}
template <> inline std::vector<uint8_t> serialize_msg<z_geometry_msgs::z_TransformStamped>(const z_geometry_msgs::z_TransformStamped& msg) {
    nlohmann::json j;
    j["header"] = {{"stamp", {{"sec", msg.stamp.sec}, {"nanosec", msg.stamp.nanosec}}}, {"frame_id", msg.frame_id}};
    j["child_frame_id"] = msg.child_frame_id;
    j["transform"] = {{"translation", {{"x", msg.transform.translation.x}, {"y", msg.transform.translation.y}, {"z", msg.transform.translation.z}}},
                      {"rotation", {{"x", msg.transform.rotation.x}, {"y", msg.transform.rotation.y}, {"z", msg.transform.rotation.z}, {"w", msg.transform.rotation.w}}}};
    auto b = nlohmann::json::to_msgpack(j); return {b.begin(), b.end()};
}
template <> inline void deserialize_msg<z_geometry_msgs::z_TransformStamped>(const std::vector<uint8_t>& buf, z_geometry_msgs::z_TransformStamped& msg) {
    auto j = nlohmann::json::from_msgpack(buf);
    msg.stamp.sec = j["header"]["stamp"]["sec"]; msg.stamp.nanosec = j["header"]["stamp"]["nanosec"]; msg.frame_id = j["header"]["frame_id"].get<std::string>();
    msg.child_frame_id = j["child_frame_id"].get<std::string>();
    msg.transform.translation.x = j["transform"]["translation"]["x"]; msg.transform.translation.y = j["transform"]["translation"]["y"]; msg.transform.translation.z = j["transform"]["translation"]["z"];
    msg.transform.rotation.x = j["transform"]["rotation"]["x"]; msg.transform.rotation.y = j["transform"]["rotation"]["y"]; msg.transform.rotation.z = j["transform"]["rotation"]["z"]; msg.transform.rotation.w = j["transform"]["rotation"]["w"];
}

#endif // Z_GEOMETRY_MSGS_PC_H
