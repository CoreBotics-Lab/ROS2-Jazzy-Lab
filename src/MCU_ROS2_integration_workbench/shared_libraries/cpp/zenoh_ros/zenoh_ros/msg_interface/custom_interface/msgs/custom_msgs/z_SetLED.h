#ifndef CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_SETLED_H
#define CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_SETLED_H

#include <nlohmann/json.hpp>
#include <vector>
#include <string>

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
inline std::vector<uint8_t> serialize_msg<custom_msgs::z_SetLED>(
    const custom_msgs::z_SetLED& msg) {
    nlohmann::json j;
    j["r"] = msg.r;
    j["g"] = msg.g;
    j["b"] = msg.b;
    j["brightness"] = msg.brightness;
    j["led_num"] = msg.led_num;
    return nlohmann::json::to_msgpack(j);
}

// --- Topic Deserialization Override ---
template <>
inline void deserialize_msg<custom_msgs::z_SetLED>(
    const std::vector<uint8_t>& buffer, custom_msgs::z_SetLED& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.r = j["r"].get<uint8_t>();
    msg.g = j["g"].get<uint8_t>();
    msg.b = j["b"].get<uint8_t>();
    msg.brightness = j["brightness"].get<uint8_t>();
    msg.led_num = j["led_num"].get<uint8_t>();
}

using z_SetLED = custom_msgs::z_SetLED;

#endif // CUSTOM_INTERFACE_PC_CUSTOM_MSGS_Z_SETLED_H
