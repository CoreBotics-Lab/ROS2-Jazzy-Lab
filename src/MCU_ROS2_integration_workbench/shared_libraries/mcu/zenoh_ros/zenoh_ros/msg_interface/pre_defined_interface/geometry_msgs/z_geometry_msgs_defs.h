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

#endif // Z_GEOMETRY_MSGS_H
