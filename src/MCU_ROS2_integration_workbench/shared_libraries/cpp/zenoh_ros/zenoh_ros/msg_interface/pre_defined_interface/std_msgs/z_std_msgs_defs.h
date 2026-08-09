#ifndef Z_STD_MSGS_PC_H
#define Z_STD_MSGS_PC_H

#include <nlohmann/json.hpp>
#include <vector>
#include <string>

// Forward declarations of serialization templates
template <typename T>
std::vector<uint8_t> serialize_msg(const T& msg);

template <typename T>
void deserialize_msg(const std::vector<uint8_t>& buffer, T& msg);

// --- Helper macro to define primitive message types and their serialization traits ---
#define DEFINE_Z_STD_MSG_PRIMITIVE_PC(TypeName, Type) \
    namespace z_std_msgs { \
        struct TypeName { \
            Type data; \
        }; \
    } \
    template <> \
    inline std::vector<uint8_t> serialize_msg<z_std_msgs::TypeName>(const z_std_msgs::TypeName& msg) { \
        nlohmann::json j = nlohmann::json::array(); \
        j.push_back(msg.data); \
        return nlohmann::json::to_msgpack(j); \
    } \
    template <> \
    inline void deserialize_msg<z_std_msgs::TypeName>(const std::vector<uint8_t>& buffer, z_std_msgs::TypeName& msg) { \
        nlohmann::json j = nlohmann::json::from_msgpack(buffer); \
        msg.data = j[0].get<Type>(); \
    }

// --- Helper macro to define array message types and their serialization traits ---
#define DEFINE_Z_STD_MSG_ARRAY_PC(TypeName, Type) \
    namespace z_std_msgs { \
        struct TypeName { \
            std::vector<Type> data; \
        }; \
    } \
    template <> \
    inline std::vector<uint8_t> serialize_msg<z_std_msgs::TypeName>(const z_std_msgs::TypeName& msg) { \
        nlohmann::json j = msg.data; \
        return nlohmann::json::to_msgpack(j); \
    } \
    template <> \
    inline void deserialize_msg<z_std_msgs::TypeName>(const std::vector<uint8_t>& buffer, z_std_msgs::TypeName& msg) { \
        nlohmann::json j = nlohmann::json::from_msgpack(buffer); \
        msg.data = j.get<std::vector<Type>>(); \
    }

// Define Primitives
DEFINE_Z_STD_MSG_PRIMITIVE_PC(z_Bool, bool)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(z_Byte, int8_t)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(z_Char, uint8_t)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(z_Int8, int8_t)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(z_UInt8, uint8_t)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(z_Int16, int16_t)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(z_UInt16, uint16_t)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(z_Int32, int32_t)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(z_UInt32, uint32_t)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(z_Int64, int64_t)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(z_UInt64, uint64_t)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(z_Float32, float)
DEFINE_Z_STD_MSG_PRIMITIVE_PC(z_Float64, double)

// Define Arrays
DEFINE_Z_STD_MSG_ARRAY_PC(z_ByteMultiArray, int8_t)
DEFINE_Z_STD_MSG_ARRAY_PC(z_Int8MultiArray, int8_t)
DEFINE_Z_STD_MSG_ARRAY_PC(z_UInt8MultiArray, uint8_t)
DEFINE_Z_STD_MSG_ARRAY_PC(z_Int16MultiArray, int16_t)
DEFINE_Z_STD_MSG_ARRAY_PC(z_UInt16MultiArray, uint16_t)
DEFINE_Z_STD_MSG_ARRAY_PC(z_Int32MultiArray, int32_t)
DEFINE_Z_STD_MSG_ARRAY_PC(z_UInt32MultiArray, uint32_t)
DEFINE_Z_STD_MSG_ARRAY_PC(z_Int64MultiArray, int64_t)
DEFINE_Z_STD_MSG_ARRAY_PC(z_UInt64MultiArray, uint64_t)
DEFINE_Z_STD_MSG_ARRAY_PC(z_Float32MultiArray, float)
DEFINE_Z_STD_MSG_ARRAY_PC(z_Float64MultiArray, double)

// --- Custom Specialization for z_std_msgs::z_String ---
namespace z_std_msgs {
    struct z_String {
        std::string data;
    };
}
template <>
inline std::vector<uint8_t> serialize_msg<z_std_msgs::z_String>(const z_std_msgs::z_String& msg) {
    nlohmann::json j = nlohmann::json::array();
    j.push_back(msg.data);
    return nlohmann::json::to_msgpack(j);
}
template <>
inline void deserialize_msg<z_std_msgs::z_String>(const std::vector<uint8_t>& buffer, z_std_msgs::z_String& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.data = j[0].get<std::string>();
}

// --- Custom Specialization for z_std_msgs::z_Empty ---
namespace z_std_msgs {
    struct z_Empty {};
}
template <>
inline std::vector<uint8_t> serialize_msg<z_std_msgs::z_Empty>(const z_std_msgs::z_Empty& msg) {
    nlohmann::json j = nlohmann::json::array();
    return nlohmann::json::to_msgpack(j);
}
template <>
inline void deserialize_msg<z_std_msgs::z_Empty>(const std::vector<uint8_t>& buffer, z_std_msgs::z_Empty& msg) {
    (void)buffer; (void)msg;
}

// --- Passthrough Specialization for z_Raw messages ---
namespace z_std_msgs {
    struct z_Raw {
        std::vector<uint8_t> data;
    };
}
template <>
inline std::vector<uint8_t> serialize_msg<z_std_msgs::z_Raw>(const z_std_msgs::z_Raw& msg) {
    return msg.data;
}
template <>
inline void deserialize_msg<z_std_msgs::z_Raw>(const std::vector<uint8_t>& buffer, z_std_msgs::z_Raw& msg) {
    msg.data = buffer;
}

// --- Specializations for nlohmann::json ---
template <>
inline std::vector<uint8_t> serialize_msg<nlohmann::json>(const nlohmann::json& msg) {
    return nlohmann::json::to_msgpack(msg);
}
template <>
inline void deserialize_msg<nlohmann::json>(const std::vector<uint8_t>& buffer, nlohmann::json& msg) {
    msg = nlohmann::json::from_msgpack(buffer);
}

#endif // Z_STD_MSGS_PC_H
