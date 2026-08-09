#ifndef Z_STD_MSGS_H
#define Z_STD_MSGS_H

#include <ArduinoJson.h>
#include <vector>
#include <string>
#include "z_builtin_interfaces.h"

// --- Helper macro to define primitive message types and their serialization traits ---
#define DEFINE_Z_STD_MSG_PRIMITIVE(TypeName, Type) \
    namespace z_std_msgs { \
        struct TypeName { \
            Type data; \
        }; \
    } \
    template <> \
    inline size_t serialize_msg<z_std_msgs::TypeName>(const z_std_msgs::TypeName& msg, uint8_t* buffer, size_t max_len) { \
        JsonDocument doc; \
        doc.add(msg.data); \
        return serializeMsgPack(doc, buffer, max_len); \
    } \
    template <> \
    inline void deserialize_msg<z_std_msgs::TypeName>(const uint8_t* buffer, size_t len, z_std_msgs::TypeName& msg) { \
        JsonDocument doc; \
        deserializeMsgPack(doc, buffer, len); \
        msg.data = doc[0].as<Type>(); \
    }

// --- Helper macro to define array message types and their serialization traits ---
#define DEFINE_Z_STD_MSG_ARRAY(TypeName, Type) \
    namespace z_std_msgs { \
        struct TypeName { \
            std::vector<Type> data; \
        }; \
    } \
    template <> \
    inline size_t serialize_msg<z_std_msgs::TypeName>(const z_std_msgs::TypeName& msg, uint8_t* buffer, size_t max_len) { \
        JsonDocument doc; \
        JsonArray array = doc.to<JsonArray>(); \
        for (Type val : msg.data) { \
            array.add(val); \
        } \
        return serializeMsgPack(doc, buffer, max_len); \
    } \
    template <> \
    inline void deserialize_msg<z_std_msgs::TypeName>(const uint8_t* buffer, size_t len, z_std_msgs::TypeName& msg) { \
        JsonDocument doc; \
        deserializeMsgPack(doc, buffer, len); \
        JsonArrayConst array = doc.as<JsonArrayConst>(); \
        msg.data.clear(); \
        for (JsonVariantConst val : array) { \
            msg.data.push_back(val.as<Type>()); \
        } \
    }

// Define Primitives
DEFINE_Z_STD_MSG_PRIMITIVE(z_Bool, bool)
DEFINE_Z_STD_MSG_PRIMITIVE(z_Byte, int8_t)
DEFINE_Z_STD_MSG_PRIMITIVE(z_Char, uint8_t)
DEFINE_Z_STD_MSG_PRIMITIVE(z_Int8, int8_t)
DEFINE_Z_STD_MSG_PRIMITIVE(z_UInt8, uint8_t)
DEFINE_Z_STD_MSG_PRIMITIVE(z_Int16, int16_t)
DEFINE_Z_STD_MSG_PRIMITIVE(z_UInt16, uint16_t)
DEFINE_Z_STD_MSG_PRIMITIVE(z_Int32, int32_t)
DEFINE_Z_STD_MSG_PRIMITIVE(z_UInt32, uint32_t)
DEFINE_Z_STD_MSG_PRIMITIVE(z_Int64, int64_t)
DEFINE_Z_STD_MSG_PRIMITIVE(z_UInt64, uint64_t)
DEFINE_Z_STD_MSG_PRIMITIVE(z_Float32, float)
DEFINE_Z_STD_MSG_PRIMITIVE(z_Float64, double)

// Define Arrays
DEFINE_Z_STD_MSG_ARRAY(z_ByteMultiArray, int8_t)
DEFINE_Z_STD_MSG_ARRAY(z_Int8MultiArray, int8_t)
DEFINE_Z_STD_MSG_ARRAY(z_UInt8MultiArray, uint8_t)
DEFINE_Z_STD_MSG_ARRAY(z_Int16MultiArray, int16_t)
DEFINE_Z_STD_MSG_ARRAY(z_UInt16MultiArray, uint16_t)
DEFINE_Z_STD_MSG_ARRAY(z_Int32MultiArray, int32_t)
DEFINE_Z_STD_MSG_ARRAY(z_UInt32MultiArray, uint32_t)
DEFINE_Z_STD_MSG_ARRAY(z_Int64MultiArray, int64_t)
DEFINE_Z_STD_MSG_ARRAY(z_UInt64MultiArray, uint64_t)
DEFINE_Z_STD_MSG_ARRAY(z_Float32MultiArray, float)
DEFINE_Z_STD_MSG_ARRAY(z_Float64MultiArray, double)

// --- Custom Specialization for z_std_msgs::z_String ---
namespace z_std_msgs {
    struct z_String {
        std::string data;
    };
    struct z_Header {
        builtin_interfaces::z_Time stamp;
        std::string frame_id;
    };
}
template <>
inline size_t serialize_msg<z_std_msgs::z_String>(const z_std_msgs::z_String& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc.add(msg.data);
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_std_msgs::z_String>(const uint8_t* buffer, size_t len, z_std_msgs::z_String& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.data = doc[0].as<std::string>();
}

template <>
inline size_t serialize_msg<z_std_msgs::z_Header>(const z_std_msgs::z_Header& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["stamp"]["sec"] = msg.stamp.sec;
    doc["stamp"]["nanosec"] = msg.stamp.nanosec;
    doc["frame_id"] = msg.frame_id;
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_std_msgs::z_Header>(const uint8_t* buffer, size_t len, z_std_msgs::z_Header& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.stamp.sec = doc["stamp"]["sec"].as<int32_t>();
    msg.stamp.nanosec = doc["stamp"]["nanosec"].as<uint32_t>();
    msg.frame_id = doc["frame_id"].as<std::string>();
}

// --- Custom Specialization for z_std_msgs::z_Empty ---
namespace z_std_msgs {
    struct z_Empty {};
}
template <>
inline size_t serialize_msg<z_std_msgs::z_Empty>(const z_std_msgs::z_Empty& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<z_std_msgs::z_Empty>(const uint8_t* buffer, size_t len, z_std_msgs::z_Empty& msg) {
    (void)buffer; (void)len; (void)msg;
}

// --- Passthrough Specialization for z_Raw messages ---
namespace z_std_msgs {
    struct z_Raw {
        std::vector<uint8_t> data;
    };
}
template <>
inline size_t serialize_msg<z_std_msgs::z_Raw>(const z_std_msgs::z_Raw& msg, uint8_t* buffer, size_t max_len) {
    size_t copy_len = msg.data.size() < max_len ? msg.data.size() : max_len;
    memcpy(buffer, msg.data.data(), copy_len);
    return copy_len;
}
template <>
inline void deserialize_msg<z_std_msgs::z_Raw>(const uint8_t* buffer, size_t len, z_std_msgs::z_Raw& msg) {
    msg.data.assign(buffer, buffer + len);
}

// --- Specializations for ArduinoJson::JsonDocument ---
template <>
inline size_t serialize_msg<ArduinoJson::JsonDocument>(const ArduinoJson::JsonDocument& msg, uint8_t* buffer, size_t max_len) {
    return serializeMsgPack(msg, buffer, max_len);
}
template <>
inline void deserialize_msg<ArduinoJson::JsonDocument>(const uint8_t* buffer, size_t len, ArduinoJson::JsonDocument& msg) {
    deserializeMsgPack(msg, buffer, len);
}

#endif // Z_STD_MSGS_H
