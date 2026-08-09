#ifndef CUSTOM_INTERFACE_CUSTOM_SRVS_Z_SETLEDCOLOR_H
#define CUSTOM_INTERFACE_CUSTOM_SRVS_Z_SETLEDCOLOR_H

#include <ArduinoJson.h>
#include <zenoh_ros/custom_msgs/z_SetLED.h>

namespace custom_srvs {
struct z_SetLEDColor {
    struct Request {
        custom_msgs::z_SetLED led_data;
    };

    struct Response {
        bool success;
        std::string message;
    };
};
} // namespace custom_srvs

// --- Service Request Serializer ---
template <>
inline size_t serialize_msg<custom_srvs::z_SetLEDColor::Request>(
    const custom_srvs::z_SetLEDColor::Request& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
      uint8_t led_data_buf[256];
  size_t led_data_len = serialize_msg(msg.led_data, led_data_buf, sizeof(led_data_buf));
  JsonDocument led_data_doc;
  deserializeMsgPack(led_data_doc, led_data_buf, led_data_len);
  doc["led_data"] = led_data_doc;
    return serializeMsgPack(doc, buffer, max_len);
}

// --- Service Request Deserializer ---
template <>
inline void deserialize_msg<custom_srvs::z_SetLEDColor::Request>(
    const uint8_t* buffer, size_t len, custom_srvs::z_SetLEDColor::Request& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
      uint8_t led_data_buf[256];
  size_t led_data_len = serializeMsgPack(doc["led_data"], led_data_buf, sizeof(led_data_buf));
  deserialize_msg(led_data_buf, led_data_len, msg.led_data);
}

// --- Service Response Serializer ---
template <>
inline size_t serialize_msg<custom_srvs::z_SetLEDColor::Response>(
    const custom_srvs::z_SetLEDColor::Response& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["success"] = msg.success;
    doc["message"] = msg.message;
    return serializeMsgPack(doc, buffer, max_len);
}

// --- Service Response Deserializer ---
template <>
inline void deserialize_msg<custom_srvs::z_SetLEDColor::Response>(
    const uint8_t* buffer, size_t len, custom_srvs::z_SetLEDColor::Response& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.success = doc["success"].as<bool>();
    msg.message = doc["message"].as<std::string>();
}

using z_SetLEDColor = custom_srvs::z_SetLEDColor;

#endif // CUSTOM_INTERFACE_CUSTOM_SRVS_Z_SETLEDCOLOR_H
