/**
 * z_logger.h — ROS 2-style logger for zenoh_ros MCU (Arduino / ESP32)
 *
 * API mirrors rclcpp macros exactly:
 *
 *   ZLogger logger("my_node");                // declare once in your class
 *
 *   ZLOG_DEBUG(logger, "ax=%.3f", ax);
 *   ZLOG_INFO(logger,  "Node started");
 *   ZLOG_WARN(logger,  "Battery low: %.1fV", v);
 *   ZLOG_ERROR(logger, "Sensor failed: %s", name);
 *   ZLOG_FATAL(logger, "Out of memory");
 *
 *   // Once (only first call at this line is printed — like RCLCPP_INFO_ONCE)
 *   ZLOG_INFO_ONCE(logger,  "Init complete");
 *   ZLOG_WARN_ONCE(logger,  "Deprecated API");
 *
 *   // Throttle (print at most every N milliseconds — like RCLCPP_INFO_THROTTLE)
 *   ZLOG_DEBUG_THROTTLE(logger, 1000, "Tick %lu", millis());
 *   ZLOG_WARN_THROTTLE(logger,  5000, "High temp: %.1f", t);
 *
 *   // Conditional (like RCLCPP_INFO_EXPRESSION)
 *   ZLOG_WARN_EXPRESSION(logger, temp > 80.0f, "Overheating: %.1f", temp);
 *
 * To reduce flash usage, define before including this header:
 *   #define ZLOG_MIN_LEVEL ZLOG_LEVEL_WARN   // strips DEBUG + INFO from flash
 *
 * To publish logs over Zenoh (so PC can subscribe to 'zenoh_ros/log'):
 *   Call logger.z_attach(this) inside your ZenohNode subclass constructor,
 *   where 'this' is the ZenohNode instance.
 */

#ifndef Z_LOGGER_MCU_H
#define Z_LOGGER_MCU_H

#include <Arduino.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

// ─── Severity levels (identical to rcutils) ───────────────────────────────────

#define ZLOG_LEVEL_DEBUG  10
#define ZLOG_LEVEL_INFO   20
#define ZLOG_LEVEL_WARN   30
#define ZLOG_LEVEL_ERROR  40
#define ZLOG_LEVEL_FATAL  50

// Compile-time minimum level (strips lower levels from flash entirely)
#ifndef ZLOG_MIN_LEVEL
#define ZLOG_MIN_LEVEL ZLOG_LEVEL_DEBUG
#endif

// ─── ANSI color codes ─────────────────────────────────────────────────────────
// Works in ESP-IDF monitor and most serial terminals with ANSI support.

#define _ZLOG_COLOR_RESET   "\033[0m"
#define _ZLOG_COLOR_CYAN    "\033[36m"      // DEBUG
#define _ZLOG_COLOR_WHITE   "\033[37m"      // INFO
#define _ZLOG_COLOR_YELLOW  "\033[33m"      // WARN
#define _ZLOG_COLOR_RED     "\033[31m"      // ERROR
#define _ZLOG_COLOR_BRED    "\033[1;31m"    // FATAL

// ─── Internal format helper ───────────────────────────────────────────────────

// Writes a formatted log line to Serial, matching ROS 2 format:
//   [LEVEL] [sec.ns] [name]: message
#define _ZLOG_PRINT(color, level_str, name, fmt, ...)                           \
    do {                                                                         \
        uint32_t _t = (uint32_t)(esp_timer_get_time() / 1000ULL);              \
        uint32_t _s = _t / 1000; uint32_t _ms = _t % 1000;                     \
        Serial.printf(color "[" level_str "] [%lu.%03lu] [%s]: " fmt           \
                      _ZLOG_COLOR_RESET "\n", (unsigned long)_s,                \
                      (unsigned long)_ms, name, ##__VA_ARGS__);                 \
    } while(0)

// ─── ZLogger class ────────────────────────────────────────────────────────────

class ZLogger {
public:
    /**
     * Construct a logger with a node name.
     * @param name   Node or component name (stored as a pointer — must be a string literal or outlive this object)
     * @param level  Minimum severity level (can be changed at runtime)
     */
    explicit ZLogger(const char* name, uint8_t level = ZLOG_LEVEL_DEBUG)
        : _name(name), _level(level), _node(nullptr) {}

    // ── Properties ────────────────────────────────────────────────────────────

    inline const char* get_name()  const { return _name; }
    inline uint8_t     get_level() const { return _level; }

    /** Set minimum severity at runtime. Matches rclcpp::Logger::set_level(). */
    inline void set_level(uint8_t level) { _level = level; }

    /**
     * Attach a ZenohNode to publish log records over Zenoh topic 'zenoh_ros/log'.
     * Call this in your ZenohNode constructor:
     *   logger.z_attach(this);
     */
    template<typename NodeT>
    inline void z_attach(NodeT* node) { _node = (void*)node; _node_publish = &_publish_fn<NodeT>; }

    // ── Internal print (called by macros only — do not call directly) ─────────

    inline bool _enabled(uint8_t level) const { return level >= _level; }

    void _print(const char* color, const char* level_str, const char* fmt, ...) const {
        char msg_buf[128];
        va_list args;
        va_start(args, fmt);
        vsnprintf(msg_buf, sizeof(msg_buf), fmt, args);
        va_end(args);

        uint64_t t_us = (uint64_t)esp_timer_get_time();
        uint32_t s    = (uint32_t)(t_us / 1000000ULL);
        uint32_t ms   = (uint32_t)((t_us / 1000ULL) % 1000ULL);

        Serial.printf("[%s] [%s]: %s\n", level_str, _name, msg_buf);

        // Publish to Zenoh if a node is attached
        if (_node != nullptr && _node_publish != nullptr) {
            _node_publish(_node, level_str, _name, msg_buf, t_us);
        }
    }

private:
    const char* _name;
    uint8_t     _level;
    void*       _node = nullptr;
    // Function pointer for type-erased Zenoh publish
    void (*_node_publish)(void*, const char*, const char*, const char*, uint64_t) = nullptr;

    template<typename NodeT>
    static void _publish_fn(void* node_ptr, const char* level_str,
                             const char* name, const char* message, uint64_t ts_us) {
        (void)ts_us;
        NodeT* node = reinterpret_cast<NodeT*>(node_ptr);
        char log_buf[256];
        snprintf(log_buf, sizeof(log_buf), "[%s] [%s]: %s", level_str, name, message);
        
        char log_topic[64];
        snprintf(log_topic, sizeof(log_topic), "%s/log", name);
        node->z_publish_raw(log_topic, (const uint8_t*)log_buf, strlen(log_buf));
    }
};

/** Global helper to construct a ZLogger for a given name (e.g. z_get_logger("system")) */
inline ZLogger z_get_logger(const char* name) {
    return ZLogger(name);
}

// ─── Logging macros (matching RCLCPP naming convention) ───────────────────────

// Plain — every call
#if ZLOG_MIN_LEVEL <= ZLOG_LEVEL_DEBUG
  #define ZLOG_DEBUG(logger, fmt, ...)  \
      do { if ((logger)._enabled(ZLOG_LEVEL_DEBUG)) \
          (logger)._print(_ZLOG_COLOR_CYAN, "DEBUG", fmt, ##__VA_ARGS__); } while(0)
#else
  #define ZLOG_DEBUG(logger, fmt, ...)  do {} while(0)
#endif

#if ZLOG_MIN_LEVEL <= ZLOG_LEVEL_INFO
  #define ZLOG_INFO(logger, fmt, ...)   \
      do { if ((logger)._enabled(ZLOG_LEVEL_INFO)) \
          (logger)._print(_ZLOG_COLOR_WHITE, "INFO", fmt, ##__VA_ARGS__); } while(0)
#else
  #define ZLOG_INFO(logger, fmt, ...)   do {} while(0)
#endif

#define ZLOG_WARN(logger, fmt, ...)   \
    do { if ((logger)._enabled(ZLOG_LEVEL_WARN)) \
        (logger)._print(_ZLOG_COLOR_YELLOW, "WARN", fmt, ##__VA_ARGS__); } while(0)

#define ZLOG_ERROR(logger, fmt, ...)  \
    do { if ((logger)._enabled(ZLOG_LEVEL_ERROR)) \
        (logger)._print(_ZLOG_COLOR_RED, "ERROR", fmt, ##__VA_ARGS__); } while(0)

#define ZLOG_FATAL(logger, fmt, ...)  \
    do { if ((logger)._enabled(ZLOG_LEVEL_FATAL)) \
        (logger)._print(_ZLOG_COLOR_BRED, "FATAL", fmt, ##__VA_ARGS__); } while(0)

// ─── _ONCE variants — only print the first time this line is hit ──────────────
// Uses static bool per call site (zero heap, same as RCLCPP_INFO_ONCE)

#define _ZLOG_ONCE(logger, color, level_int, level_str, fmt, ...)           \
    do {                                                                      \
        static bool _fired = false;                                           \
        if (!_fired && (logger)._enabled(level_int)) {                        \
            _fired = true;                                                    \
            (logger)._print(color, level_str, fmt, ##__VA_ARGS__);           \
        }                                                                     \
    } while(0)

#if ZLOG_MIN_LEVEL <= ZLOG_LEVEL_DEBUG
  #define ZLOG_DEBUG_ONCE(logger, fmt, ...)  _ZLOG_ONCE(logger, _ZLOG_COLOR_CYAN,   ZLOG_LEVEL_DEBUG, "DEBUG", fmt, ##__VA_ARGS__)
#else
  #define ZLOG_DEBUG_ONCE(logger, fmt, ...)  do {} while(0)
#endif

#if ZLOG_MIN_LEVEL <= ZLOG_LEVEL_INFO
  #define ZLOG_INFO_ONCE(logger, fmt, ...)   _ZLOG_ONCE(logger, _ZLOG_COLOR_WHITE,  ZLOG_LEVEL_INFO,  "INFO",  fmt, ##__VA_ARGS__)
#else
  #define ZLOG_INFO_ONCE(logger, fmt, ...)   do {} while(0)
#endif

#define ZLOG_WARN_ONCE(logger, fmt, ...)   _ZLOG_ONCE(logger, _ZLOG_COLOR_YELLOW, ZLOG_LEVEL_WARN,  "WARN",  fmt, ##__VA_ARGS__)
#define ZLOG_ERROR_ONCE(logger, fmt, ...)  _ZLOG_ONCE(logger, _ZLOG_COLOR_RED,    ZLOG_LEVEL_ERROR, "ERROR", fmt, ##__VA_ARGS__)
#define ZLOG_FATAL_ONCE(logger, fmt, ...)  _ZLOG_ONCE(logger, _ZLOG_COLOR_BRED,   ZLOG_LEVEL_FATAL, "FATAL", fmt, ##__VA_ARGS__)

// ─── _THROTTLE variants — print at most every N milliseconds ─────────────────
// Uses static uint32_t per call site (zero heap, same as RCLCPP_INFO_THROTTLE)

#define _ZLOG_THROTTLE(logger, ms_interval, color, level_int, level_str, fmt, ...) \
    do {                                                                             \
        static uint32_t _last = 0;                                                  \
        uint32_t _now = (uint32_t)(esp_timer_get_time() / 1000ULL);                \
        if ((logger)._enabled(level_int) && (_now - _last) >= (uint32_t)(ms_interval)) { \
            _last = _now;                                                            \
            (logger)._print(color, level_str, fmt, ##__VA_ARGS__);                 \
        }                                                                           \
    } while(0)

#if ZLOG_MIN_LEVEL <= ZLOG_LEVEL_DEBUG
  #define ZLOG_DEBUG_THROTTLE(logger, ms, fmt, ...)  _ZLOG_THROTTLE(logger, ms, _ZLOG_COLOR_CYAN,   ZLOG_LEVEL_DEBUG, "DEBUG", fmt, ##__VA_ARGS__)
#else
  #define ZLOG_DEBUG_THROTTLE(logger, ms, fmt, ...)  do {} while(0)
#endif

#if ZLOG_MIN_LEVEL <= ZLOG_LEVEL_INFO
  #define ZLOG_INFO_THROTTLE(logger, ms, fmt, ...)   _ZLOG_THROTTLE(logger, ms, _ZLOG_COLOR_WHITE,  ZLOG_LEVEL_INFO,  "INFO",  fmt, ##__VA_ARGS__)
#else
  #define ZLOG_INFO_THROTTLE(logger, ms, fmt, ...)   do {} while(0)
#endif

#define ZLOG_WARN_THROTTLE(logger, ms, fmt, ...)   _ZLOG_THROTTLE(logger, ms, _ZLOG_COLOR_YELLOW, ZLOG_LEVEL_WARN,  "WARN",  fmt, ##__VA_ARGS__)
#define ZLOG_ERROR_THROTTLE(logger, ms, fmt, ...)  _ZLOG_THROTTLE(logger, ms, _ZLOG_COLOR_RED,    ZLOG_LEVEL_ERROR, "ERROR", fmt, ##__VA_ARGS__)
#define ZLOG_FATAL_THROTTLE(logger, ms, fmt, ...)  _ZLOG_THROTTLE(logger, ms, _ZLOG_COLOR_BRED,   ZLOG_LEVEL_FATAL, "FATAL", fmt, ##__VA_ARGS__)

// ─── _EXPRESSION variants — conditional print ─────────────────────────────────

#define _ZLOG_EXPR(logger, expr, color, level_int, level_str, fmt, ...)      \
    do { if ((expr) && (logger)._enabled(level_int))                          \
        (logger)._print(color, level_str, fmt, ##__VA_ARGS__); } while(0)

#if ZLOG_MIN_LEVEL <= ZLOG_LEVEL_DEBUG
  #define ZLOG_DEBUG_EXPRESSION(logger, expr, fmt, ...)  _ZLOG_EXPR(logger, expr, _ZLOG_COLOR_CYAN,   ZLOG_LEVEL_DEBUG, "DEBUG", fmt, ##__VA_ARGS__)
#else
  #define ZLOG_DEBUG_EXPRESSION(logger, expr, fmt, ...)  do {} while(0)
#endif

#if ZLOG_MIN_LEVEL <= ZLOG_LEVEL_INFO
  #define ZLOG_INFO_EXPRESSION(logger, expr, fmt, ...)   _ZLOG_EXPR(logger, expr, _ZLOG_COLOR_WHITE,  ZLOG_LEVEL_INFO,  "INFO",  fmt, ##__VA_ARGS__)
#else
  #define ZLOG_INFO_EXPRESSION(logger, expr, fmt, ...)   do {} while(0)
#endif

#define ZLOG_WARN_EXPRESSION(logger, expr, fmt, ...)   _ZLOG_EXPR(logger, expr, _ZLOG_COLOR_YELLOW, ZLOG_LEVEL_WARN,  "WARN",  fmt, ##__VA_ARGS__)
#define ZLOG_ERROR_EXPRESSION(logger, expr, fmt, ...)  _ZLOG_EXPR(logger, expr, _ZLOG_COLOR_RED,    ZLOG_LEVEL_ERROR, "ERROR", fmt, ##__VA_ARGS__)
#define ZLOG_FATAL_EXPRESSION(logger, expr, fmt, ...)  _ZLOG_EXPR(logger, expr, _ZLOG_COLOR_BRED,   ZLOG_LEVEL_FATAL, "FATAL", fmt, ##__VA_ARGS__)

#endif // Z_LOGGER_MCU_H
