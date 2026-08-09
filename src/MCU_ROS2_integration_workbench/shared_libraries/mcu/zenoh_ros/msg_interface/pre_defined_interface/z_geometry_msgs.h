#ifndef Z_GEOMETRY_MSGS_H
#define Z_GEOMETRY_MSGS_H

#include <ArduinoJson.h>

namespace z_geometry_msgs {

    struct z_Vector3 {
        double x;
        double y;
        double z;
    };

    struct z_Point {
        double x;
        double y;
        double z;
    };

    struct z_Quaternion {
        double x;
        double y;
        double z;
        double w;
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
        double x;
        double y;
        double theta;
    };

} // namespace z_geometry_msgs

// --- Serialization Specializations ---

template <>
inline size_t serialize_msg<z_geometry_msgs::z_Vector3>(const z_geometry_msgs::z_Vector3& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(msg.x);
    array.add(msg.y);
    array.add(msg.z);
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline size_t serialize_msg<z_geometry_msgs::z_Point>(const z_geometry_msgs::z_Point& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(msg.x);
    array.add(msg.y);
    array.add(msg.z);
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline size_t serialize_msg<z_geometry_msgs::z_Quaternion>(const z_geometry_msgs::z_Quaternion& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(msg.x);
    array.add(msg.y);
    array.add(msg.z);
    array.add(msg.w);
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline size_t serialize_msg<z_geometry_msgs::z_Pose>(const z_geometry_msgs::z_Pose& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    
    JsonArray pos = array.add<JsonArray>();
    pos.add(msg.position.x);
    pos.add(msg.position.y);
    pos.add(msg.position.z);
    
    JsonArray ori = array.add<JsonArray>();
    ori.add(msg.orientation.x);
    ori.add(msg.orientation.y);
    ori.add(msg.orientation.z);
    ori.add(msg.orientation.w);
    
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline size_t serialize_msg<z_geometry_msgs::z_Twist>(const z_geometry_msgs::z_Twist& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    
    JsonArray lin = array.add<JsonArray>();
    lin.add(msg.linear.x);
    lin.add(msg.linear.y);
    lin.add(msg.linear.z);
    
    JsonArray ang = array.add<JsonArray>();
    ang.add(msg.angular.x);
    ang.add(msg.angular.y);
    ang.add(msg.angular.z);
    
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline size_t serialize_msg<z_geometry_msgs::z_Wrench>(const z_geometry_msgs::z_Wrench& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    
    JsonArray force = array.add<JsonArray>();
    force.add(msg.force.x);
    force.add(msg.force.y);
    force.add(msg.force.z);
    
    JsonArray torque = array.add<JsonArray>();
    torque.add(msg.torque.x);
    torque.add(msg.torque.y);
    torque.add(msg.torque.z);
    
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline size_t serialize_msg<z_geometry_msgs::z_Transform>(const z_geometry_msgs::z_Transform& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    
    JsonArray trans = array.add<JsonArray>();
    trans.add(msg.translation.x);
    trans.add(msg.translation.y);
    trans.add(msg.translation.z);
    
    JsonArray rot = array.add<JsonArray>();
    rot.add(msg.rotation.x);
    rot.add(msg.rotation.y);
    rot.add(msg.rotation.z);
    rot.add(msg.rotation.w);
    
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline size_t serialize_msg<z_geometry_msgs::z_Pose2D>(const z_geometry_msgs::z_Pose2D& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    array.add(msg.x);
    array.add(msg.y);
    array.add(msg.theta);
    return serializeMsgPack(doc, buffer, max_len);
}

// --- Deserialization Specializations ---

template <>
inline void deserialize_msg<z_geometry_msgs::z_Vector3>(const uint8_t* buffer, size_t len, z_geometry_msgs::z_Vector3& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.x = doc[0].as<double>();
    msg.y = doc[1].as<double>();
    msg.z = doc[2].as<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::z_Point>(const uint8_t* buffer, size_t len, z_geometry_msgs::z_Point& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.x = doc[0].as<double>();
    msg.y = doc[1].as<double>();
    msg.z = doc[2].as<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::z_Quaternion>(const uint8_t* buffer, size_t len, z_geometry_msgs::z_Quaternion& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.x = doc[0].as<double>();
    msg.y = doc[1].as<double>();
    msg.z = doc[2].as<double>();
    msg.w = doc[3].as<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::z_Pose>(const uint8_t* buffer, size_t len, z_geometry_msgs::z_Pose& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    
    msg.position.x = doc[0][0].as<double>();
    msg.position.y = doc[0][1].as<double>();
    msg.position.z = doc[0][2].as<double>();
    
    msg.orientation.x = doc[1][0].as<double>();
    msg.orientation.y = doc[1][1].as<double>();
    msg.orientation.z = doc[1][2].as<double>();
    msg.orientation.w = doc[1][3].as<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::z_Twist>(const uint8_t* buffer, size_t len, z_geometry_msgs::z_Twist& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    
    msg.linear.x = doc[0][0].as<double>();
    msg.linear.y = doc[0][1].as<double>();
    msg.linear.z = doc[0][2].as<double>();
    
    msg.angular.x = doc[1][0].as<double>();
    msg.angular.y = doc[1][1].as<double>();
    msg.angular.z = doc[1][2].as<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::z_Wrench>(const uint8_t* buffer, size_t len, z_geometry_msgs::z_Wrench& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    
    msg.force.x = doc[0][0].as<double>();
    msg.force.y = doc[0][1].as<double>();
    msg.force.z = doc[0][2].as<double>();
    
    msg.torque.x = doc[1][0].as<double>();
    msg.torque.y = doc[1][1].as<double>();
    msg.torque.z = doc[1][2].as<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::z_Transform>(const uint8_t* buffer, size_t len, z_geometry_msgs::z_Transform& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    
    msg.translation.x = doc[0][0].as<double>();
    msg.translation.y = doc[0][1].as<double>();
    msg.translation.z = doc[0][2].as<double>();
    
    msg.rotation.x = doc[1][0].as<double>();
    msg.rotation.y = doc[1][1].as<double>();
    msg.rotation.z = doc[1][2].as<double>();
    msg.rotation.w = doc[1][3].as<double>();
}

template <>
inline void deserialize_msg<z_geometry_msgs::z_Pose2D>(const uint8_t* buffer, size_t len, z_geometry_msgs::z_Pose2D& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.x = doc[0].as<double>();
    msg.y = doc[1].as<double>();
    msg.theta = doc[2].as<double>();
}

// ─── NEW types added for ROS 2 Jazzy parity ──────────────────────────────────
#include <string>
#include "z_builtin_interfaces.h"

namespace z_geometry_msgs {

    struct z_Point32   { float  x = 0; float  y = 0; float  z = 0; };
    struct z_Accel     { z_Vector3 linear; z_Vector3 angular; };

    struct z_PoseWithCovariance {
        z_Pose pose;
        double covariance[36] = {0};
    };
    struct z_TwistWithCovariance {
        z_Twist twist;
        double covariance[36] = {0};
    };
    struct z_AccelWithCovariance {
        z_Accel accel;
        double covariance[36] = {0};
    };
    struct z_Inertia {
        double m = 0;
        z_Vector3 com;
        double ixx=0, ixy=0, ixz=0, iyy=0, iyz=0, izz=0;
    };
    struct z_Polygon { std::vector<z_Point32> points; };

    // Stamped variants
    struct z_AccelStamped           { builtin_interfaces::z_Time stamp; std::string frame_id; z_Accel accel; };
    struct z_PoseStamped            { builtin_interfaces::z_Time stamp; std::string frame_id; z_Pose pose; };
    struct z_PointStamped           { builtin_interfaces::z_Time stamp; std::string frame_id; z_Point point; };
    struct z_PolygonStamped         { builtin_interfaces::z_Time stamp; std::string frame_id; z_Polygon polygon; };
    struct z_TransformStamped       { builtin_interfaces::z_Time stamp; std::string frame_id; std::string child_frame_id; z_Transform transform; };
    struct z_TwistStamped           { builtin_interfaces::z_Time stamp; std::string frame_id; z_Twist twist; };
    struct z_WrenchStamped          { builtin_interfaces::z_Time stamp; std::string frame_id; z_Wrench wrench; };
    struct z_VelocityStamped        { builtin_interfaces::z_Time stamp; std::string frame_id; std::string reference_frame_id; z_Twist velocity; };
    struct z_InertiaStamped         { builtin_interfaces::z_Time stamp; std::string frame_id; z_Inertia inertia; };
    struct z_PoseWithCovarianceStamped  { builtin_interfaces::z_Time stamp; std::string frame_id; z_PoseWithCovariance pose; };
    struct z_TwistWithCovarianceStamped { builtin_interfaces::z_Time stamp; std::string frame_id; z_TwistWithCovariance twist; };
    struct z_AccelWithCovarianceStamped { builtin_interfaces::z_Time stamp; std::string frame_id; z_AccelWithCovariance accel; };

} // namespace z_geometry_msgs (extension)

// ─── Serialization for NEW types only ────────────────────────────────────────

template <>
inline size_t serialize_msg<z_geometry_msgs::z_Point32>(const z_geometry_msgs::z_Point32& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc; JsonArray a = doc.to<JsonArray>(); a.add(msg.x); a.add(msg.y); a.add(msg.z);
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_geometry_msgs::z_Point32>(const uint8_t* buffer, size_t len, z_geometry_msgs::z_Point32& msg) {
    JsonDocument doc; deserializeMsgPack(doc, buffer, len);
    msg.x = doc[0].as<float>(); msg.y = doc[1].as<float>(); msg.z = doc[2].as<float>();
}

template <>
inline size_t serialize_msg<z_geometry_msgs::z_Accel>(const z_geometry_msgs::z_Accel& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc; JsonArray a = doc.to<JsonArray>();
    JsonArray lin = a.add<JsonArray>(); lin.add(msg.linear.x); lin.add(msg.linear.y); lin.add(msg.linear.z);
    JsonArray ang = a.add<JsonArray>(); ang.add(msg.angular.x); ang.add(msg.angular.y); ang.add(msg.angular.z);
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_geometry_msgs::z_Accel>(const uint8_t* buffer, size_t len, z_geometry_msgs::z_Accel& msg) {
    JsonDocument doc; deserializeMsgPack(doc, buffer, len);
    msg.linear.x = doc[0][0].as<double>(); msg.linear.y = doc[0][1].as<double>(); msg.linear.z = doc[0][2].as<double>();
    msg.angular.x = doc[1][0].as<double>(); msg.angular.y = doc[1][1].as<double>(); msg.angular.z = doc[1][2].as<double>();
}

template <>
inline size_t serialize_msg<z_geometry_msgs::z_PoseStamped>(const z_geometry_msgs::z_PoseStamped& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["header"]["stamp"]["sec"] = msg.stamp.sec; doc["header"]["stamp"]["nanosec"] = msg.stamp.nanosec; doc["header"]["frame_id"] = msg.frame_id;
    JsonArray pos_arr = doc["pose"]["position"].to<JsonArray>(); pos_arr.add(msg.pose.position.x); pos_arr.add(msg.pose.position.y); pos_arr.add(msg.pose.position.z);
    JsonArray ori_arr = doc["pose"]["orientation"].to<JsonArray>(); ori_arr.add(msg.pose.orientation.x); ori_arr.add(msg.pose.orientation.y); ori_arr.add(msg.pose.orientation.z); ori_arr.add(msg.pose.orientation.w);
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_geometry_msgs::z_PoseStamped>(const uint8_t* buffer, size_t len, z_geometry_msgs::z_PoseStamped& msg) {
    JsonDocument doc; deserializeMsgPack(doc, buffer, len);
    msg.stamp.sec = doc["header"]["stamp"]["sec"].as<int32_t>(); msg.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>(); msg.frame_id = doc["header"]["frame_id"].as<std::string>();
    msg.pose.position.x = doc["pose"]["position"][0].as<double>(); msg.pose.position.y = doc["pose"]["position"][1].as<double>(); msg.pose.position.z = doc["pose"]["position"][2].as<double>();
    msg.pose.orientation.x = doc["pose"]["orientation"][0].as<double>(); msg.pose.orientation.y = doc["pose"]["orientation"][1].as<double>(); msg.pose.orientation.z = doc["pose"]["orientation"][2].as<double>(); msg.pose.orientation.w = doc["pose"]["orientation"][3].as<double>();
}

template <>
inline size_t serialize_msg<z_geometry_msgs::z_TwistStamped>(const z_geometry_msgs::z_TwistStamped& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["header"]["stamp"]["sec"] = msg.stamp.sec; doc["header"]["stamp"]["nanosec"] = msg.stamp.nanosec; doc["header"]["frame_id"] = msg.frame_id;
    JsonArray lin = doc["twist"]["linear"].to<JsonArray>(); lin.add(msg.twist.linear.x); lin.add(msg.twist.linear.y); lin.add(msg.twist.linear.z);
    JsonArray ang = doc["twist"]["angular"].to<JsonArray>(); ang.add(msg.twist.angular.x); ang.add(msg.twist.angular.y); ang.add(msg.twist.angular.z);
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_geometry_msgs::z_TwistStamped>(const uint8_t* buffer, size_t len, z_geometry_msgs::z_TwistStamped& msg) {
    JsonDocument doc; deserializeMsgPack(doc, buffer, len);
    msg.stamp.sec = doc["header"]["stamp"]["sec"].as<int32_t>(); msg.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>(); msg.frame_id = doc["header"]["frame_id"].as<std::string>();
    msg.twist.linear.x = doc["twist"]["linear"][0].as<double>(); msg.twist.linear.y = doc["twist"]["linear"][1].as<double>(); msg.twist.linear.z = doc["twist"]["linear"][2].as<double>();
    msg.twist.angular.x = doc["twist"]["angular"][0].as<double>(); msg.twist.angular.y = doc["twist"]["angular"][1].as<double>(); msg.twist.angular.z = doc["twist"]["angular"][2].as<double>();
}

template <>
inline size_t serialize_msg<z_geometry_msgs::z_PointStamped>(const z_geometry_msgs::z_PointStamped& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["header"]["stamp"]["sec"] = msg.stamp.sec; doc["header"]["stamp"]["nanosec"] = msg.stamp.nanosec; doc["header"]["frame_id"] = msg.frame_id;
    JsonArray pt = doc["point"].to<JsonArray>(); pt.add(msg.point.x); pt.add(msg.point.y); pt.add(msg.point.z);
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_geometry_msgs::z_PointStamped>(const uint8_t* buffer, size_t len, z_geometry_msgs::z_PointStamped& msg) {
    JsonDocument doc; deserializeMsgPack(doc, buffer, len);
    msg.stamp.sec = doc["header"]["stamp"]["sec"].as<int32_t>(); msg.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>(); msg.frame_id = doc["header"]["frame_id"].as<std::string>();
    msg.point.x = doc["point"][0].as<double>(); msg.point.y = doc["point"][1].as<double>(); msg.point.z = doc["point"][2].as<double>();
}

template <>
inline size_t serialize_msg<z_geometry_msgs::z_TransformStamped>(const z_geometry_msgs::z_TransformStamped& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["header"]["stamp"]["sec"] = msg.stamp.sec; doc["header"]["stamp"]["nanosec"] = msg.stamp.nanosec; doc["header"]["frame_id"] = msg.frame_id;
    doc["child_frame_id"] = msg.child_frame_id;
    JsonArray tr = doc["transform"]["translation"].to<JsonArray>(); tr.add(msg.transform.translation.x); tr.add(msg.transform.translation.y); tr.add(msg.transform.translation.z);
    JsonArray rt = doc["transform"]["rotation"].to<JsonArray>(); rt.add(msg.transform.rotation.x); rt.add(msg.transform.rotation.y); rt.add(msg.transform.rotation.z); rt.add(msg.transform.rotation.w);
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_geometry_msgs::z_TransformStamped>(const uint8_t* buffer, size_t len, z_geometry_msgs::z_TransformStamped& msg) {
    JsonDocument doc; deserializeMsgPack(doc, buffer, len);
    msg.stamp.sec = doc["header"]["stamp"]["sec"].as<int32_t>(); msg.stamp.nanosec = doc["header"]["stamp"]["nanosec"].as<uint32_t>(); msg.frame_id = doc["header"]["frame_id"].as<std::string>();
    msg.child_frame_id = doc["child_frame_id"].as<std::string>();
    msg.transform.translation.x = doc["transform"]["translation"][0].as<double>(); msg.transform.translation.y = doc["transform"]["translation"][1].as<double>(); msg.transform.translation.z = doc["transform"]["translation"][2].as<double>();
    msg.transform.rotation.x = doc["transform"]["rotation"][0].as<double>(); msg.transform.rotation.y = doc["transform"]["rotation"][1].as<double>(); msg.transform.rotation.z = doc["transform"]["rotation"][2].as<double>(); msg.transform.rotation.w = doc["transform"]["rotation"][3].as<double>();
}

#endif // Z_GEOMETRY_MSGS_H
