#ifndef Z_BUILTIN_INTERFACES_PC_H
#define Z_BUILTIN_INTERFACES_PC_H

#include <nlohmann/json.hpp>
#include <vector>
#include <cstdint>

// Forward declarations of serialization templates
template <typename T>
std::vector<uint8_t> serialize_msg(const T& msg);

template <typename T>
void deserialize_msg(const std::vector<uint8_t>& buffer, T& msg);

// =============================================================================
// builtin_interfaces — PC C++ (nlohmann/json + MessagePack)
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
inline std::vector<uint8_t> serialize_msg<builtin_interfaces::z_Duration>(
    const builtin_interfaces::z_Duration& msg) {
    nlohmann::json j;
    j["sec"]     = msg.sec;
    j["nanosec"] = msg.nanosec;
    return nlohmann::json::to_msgpack(j);
}
template <>
inline void deserialize_msg<builtin_interfaces::z_Duration>(
    const std::vector<uint8_t>& buffer, builtin_interfaces::z_Duration& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.sec     = j["sec"].get<int32_t>();
    msg.nanosec = j["nanosec"].get<uint32_t>();
}

// --- Time Serialization ---
template <>
inline std::vector<uint8_t> serialize_msg<builtin_interfaces::z_Time>(
    const builtin_interfaces::z_Time& msg) {
    nlohmann::json j;
    j["sec"]     = msg.sec;
    j["nanosec"] = msg.nanosec;
    return nlohmann::json::to_msgpack(j);
}
template <>
inline void deserialize_msg<builtin_interfaces::z_Time>(
    const std::vector<uint8_t>& buffer, builtin_interfaces::z_Time& msg) {
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
    msg.sec     = j["sec"].get<int32_t>();
    msg.nanosec = j["nanosec"].get<uint32_t>();
}

using z_Duration = builtin_interfaces::z_Duration;
using z_Time     = builtin_interfaces::z_Time;

#endif // Z_BUILTIN_INTERFACES_PC_H
