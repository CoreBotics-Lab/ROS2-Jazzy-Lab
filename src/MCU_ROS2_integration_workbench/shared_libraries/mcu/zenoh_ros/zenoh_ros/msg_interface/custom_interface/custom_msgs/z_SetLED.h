#ifndef CUSTOM_INTERFACE_CUSTOM_MSGS_Z_SETLED_H
#define CUSTOM_INTERFACE_CUSTOM_MSGS_Z_SETLED_H

#include <ArduinoJson.h>

namespace custom_msgs {
struct z_SetLED {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t brightness;
    uint8_t led_num;
};
} // namespace custom_msgs

// --- Topic Serialization Override ---
template <>
inline size_t serialize_msg<custom_msgs::z_SetLED>(
    const custom_msgs::z_SetLED& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["r"] = msg.r;
    doc["g"] = msg.g;
    doc["b"] = msg.b;
    doc["brightness"] = msg.brightness;
    doc["led_num"] = msg.led_num;
    return serializeMsgPack(doc, buffer, max_len);
}

// --- Topic Deserialization Override ---
template <>
inline void deserialize_msg<custom_msgs::z_SetLED>(
    const uint8_t* buffer, size_t len, custom_msgs::z_SetLED& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.r = doc["r"].as<uint8_t>();
    msg.g = doc["g"].as<uint8_t>();
    msg.b = doc["b"].as<uint8_t>();
    msg.brightness = doc["brightness"].as<uint8_t>();
    msg.led_num = doc["led_num"].as<uint8_t>();
}

using z_SetLED = custom_msgs::z_SetLED;

#endif // CUSTOM_INTERFACE_CUSTOM_MSGS_Z_SETLED_H
