#ifndef Z_STD_SRVS_H
#define Z_STD_SRVS_H

#include <ArduinoJson.h>
#include <string>

namespace z_std_srvs {
    struct z_Empty {
        struct Request {};
        struct Response {};
    };

    struct z_Trigger {
        struct Request {};
        struct Response {
            bool success;
            std::string message;
        };
    };

    struct z_SetBool {
        struct Request {
            bool data;
        };
        struct Response {
            bool success;
            std::string message;
        };
    };
} // namespace z_std_srvs

// --- z_Empty Serializers ---
template <>
inline size_t serialize_msg<z_std_srvs::z_Empty::Request>(
    const z_std_srvs::z_Empty::Request& msg, uint8_t* buffer, size_t max_len) {
    (void)msg;
    JsonDocument doc;
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline void deserialize_msg<z_std_srvs::z_Empty::Request>(
    const uint8_t* buffer, size_t len, z_std_srvs::z_Empty::Request& msg) {
    (void)buffer; (void)len; (void)msg;
}

template <>
inline size_t serialize_msg<z_std_srvs::z_Empty::Response>(
    const z_std_srvs::z_Empty::Response& msg, uint8_t* buffer, size_t max_len) {
    (void)msg;
    JsonDocument doc;
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline void deserialize_msg<z_std_srvs::z_Empty::Response>(
    const uint8_t* buffer, size_t len, z_std_srvs::z_Empty::Response& msg) {
    (void)buffer; (void)len; (void)msg;
}

// --- z_Trigger Serializers ---
template <>
inline size_t serialize_msg<z_std_srvs::z_Trigger::Request>(
    const z_std_srvs::z_Trigger::Request& msg, uint8_t* buffer, size_t max_len) {
    (void)msg;
    JsonDocument doc;
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline void deserialize_msg<z_std_srvs::z_Trigger::Request>(
    const uint8_t* buffer, size_t len, z_std_srvs::z_Trigger::Request& msg) {
    (void)buffer; (void)len; (void)msg;
}

template <>
inline size_t serialize_msg<z_std_srvs::z_Trigger::Response>(
    const z_std_srvs::z_Trigger::Response& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["success"] = msg.success;
    doc["message"] = msg.message;
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline void deserialize_msg<z_std_srvs::z_Trigger::Response>(
    const uint8_t* buffer, size_t len, z_std_srvs::z_Trigger::Response& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.success = doc["success"].as<bool>();
    msg.message = doc["message"].as<std::string>();
}

// --- z_SetBool Serializers ---
template <>
inline size_t serialize_msg<z_std_srvs::z_SetBool::Request>(
    const z_std_srvs::z_SetBool::Request& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["data"] = msg.data;
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline void deserialize_msg<z_std_srvs::z_SetBool::Request>(
    const uint8_t* buffer, size_t len, z_std_srvs::z_SetBool::Request& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.data = doc["data"].as<bool>();
}

template <>
inline size_t serialize_msg<z_std_srvs::z_SetBool::Response>(
    const z_std_srvs::z_SetBool::Response& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["success"] = msg.success;
    doc["message"] = msg.message;
    return serializeMsgPack(doc, buffer, max_len);
}

template <>
inline void deserialize_msg<z_std_srvs::z_SetBool::Response>(
    const uint8_t* buffer, size_t len, z_std_srvs::z_SetBool::Response& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.success = doc["success"].as<bool>();
    msg.message = doc["message"].as<std::string>();
}

#endif // Z_STD_SRVS_H
