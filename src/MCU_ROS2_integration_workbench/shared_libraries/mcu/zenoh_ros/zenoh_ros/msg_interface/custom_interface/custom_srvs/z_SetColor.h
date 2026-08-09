#ifndef CUSTOM_INTERFACE_CUSTOM_SRVS_Z_SETCOLOR_H
#define CUSTOM_INTERFACE_CUSTOM_SRVS_Z_SETCOLOR_H

#include <ArduinoJson.h>

namespace custom_srvs {
struct z_SetColor {
    struct Request {
        int32_t r;
        int32_t g;
        int32_t b;
    };

    struct Response {
        bool success;
        std::string message;
    };
};
} // namespace custom_srvs

// --- Service Request Serializer ---
template <>
inline size_t serialize_msg<custom_srvs::z_SetColor::Request>(
    const custom_srvs::z_SetColor::Request& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["r"] = msg.r;
    doc["g"] = msg.g;
    doc["b"] = msg.b;
    return serializeMsgPack(doc, buffer, max_len);
}

// --- Service Request Deserializer ---
template <>
inline void deserialize_msg<custom_srvs::z_SetColor::Request>(
    const uint8_t* buffer, size_t len, custom_srvs::z_SetColor::Request& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.r = doc["r"].as<int32_t>();
    msg.g = doc["g"].as<int32_t>();
    msg.b = doc["b"].as<int32_t>();
}

// --- Service Response Serializer ---
template <>
inline size_t serialize_msg<custom_srvs::z_SetColor::Response>(
    const custom_srvs::z_SetColor::Response& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["success"] = msg.success;
    doc["message"] = msg.message;
    return serializeMsgPack(doc, buffer, max_len);
}

// --- Service Response Deserializer ---
template <>
inline void deserialize_msg<custom_srvs::z_SetColor::Response>(
    const uint8_t* buffer, size_t len, custom_srvs::z_SetColor::Response& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.success = doc["success"].as<bool>();
    msg.message = doc["message"].as<std::string>();
}

using z_SetColor = custom_srvs::z_SetColor;

#endif // CUSTOM_INTERFACE_CUSTOM_SRVS_Z_SETCOLOR_H
