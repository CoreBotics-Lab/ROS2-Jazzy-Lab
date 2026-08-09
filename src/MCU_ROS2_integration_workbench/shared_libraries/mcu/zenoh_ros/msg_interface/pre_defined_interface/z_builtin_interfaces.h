#ifndef Z_BUILTIN_INTERFACES_H
#define Z_BUILTIN_INTERFACES_H

#include <ArduinoJson.h>

// =============================================================================
// builtin_interfaces — MCU (ArduinoJson/MessagePack)
//
// Mirrors the ROS 2 builtin_interfaces package:
//   Duration: { int32 sec, uint32 nanosec }
//   Time    : { int32 sec, uint32 nanosec }
// =============================================================================

namespace builtin_interfaces {

    // ---- Duration -------------------------------------------------------
    // Equivalent of builtin_interfaces/msg/Duration
    // ROS 2 definition:
    //   int32  sec      # Seconds component (may be negative)
    //   uint32 nanosec  # Nanoseconds component [0, 10^9)
    struct z_Duration {
        int32_t  sec    = 0;
        uint32_t nanosec = 0;
    };

    // ---- Time -----------------------------------------------------------
    // Equivalent of builtin_interfaces/msg/Time
    // ROS 2 definition:
    //   int32  sec      # Seconds since epoch (may be negative before Unix epoch)
    //   uint32 nanosec  # Nanoseconds component [0, 10^9)
    struct z_Time {
        int32_t  sec    = 0;
        uint32_t nanosec = 0;
    };

} // namespace builtin_interfaces

// --- Duration Serialization ---
template <>
inline size_t serialize_msg<builtin_interfaces::z_Duration>(
    const builtin_interfaces::z_Duration& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["sec"]     = msg.sec;
    doc["nanosec"] = msg.nanosec;
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<builtin_interfaces::z_Duration>(
    const uint8_t* buffer, size_t len, builtin_interfaces::z_Duration& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.sec     = doc["sec"].as<int32_t>();
    msg.nanosec = doc["nanosec"].as<uint32_t>();
}

// --- Time Serialization ---
template <>
inline size_t serialize_msg<builtin_interfaces::z_Time>(
    const builtin_interfaces::z_Time& msg, uint8_t* buffer, size_t max_len) {
    JsonDocument doc;
    doc["sec"]     = msg.sec;
    doc["nanosec"] = msg.nanosec;
    return serializeMsgPack(doc, buffer, max_len);
}
template <>
inline void deserialize_msg<builtin_interfaces::z_Time>(
    const uint8_t* buffer, size_t len, builtin_interfaces::z_Time& msg) {
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
    msg.sec     = doc["sec"].as<int32_t>();
    msg.nanosec = doc["nanosec"].as<uint32_t>();
}

using z_Duration = builtin_interfaces::z_Duration;
using z_Time     = builtin_interfaces::z_Time;

#endif // Z_BUILTIN_INTERFACES_H
