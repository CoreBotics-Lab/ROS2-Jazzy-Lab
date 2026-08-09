/**
 * z_logger.h — ROS 2-style logger for zenoh_ros PC C++
 *
 * API matches rclcpp macro style:
 *
 *   auto logger = z_get_logger("my_node");
 *
 *   ZLOG_DEBUG(logger, "x=%.3f", x);
 *   ZLOG_INFO(logger,  "Node started");
 *   ZLOG_WARN(logger,  "Battery low: %.1fV", v);
 *   ZLOG_ERROR(logger, "Sensor failed: %s", name);
 *   ZLOG_FATAL(logger, "Out of memory");
 *
 *   ZLOG_INFO_ONCE(logger, "Connected");
 *   ZLOG_WARN_THROTTLE(logger, 1000, "Loop slow: %d ms", elapsed);
 *   ZLOG_ERROR_EXPRESSION(logger, err_cnt > 5, "Too many errors: %d", err_cnt);
 */

#ifndef Z_LOGGER_PC_H
#define Z_LOGGER_PC_H

#include <iostream>
#include <string>
#include <chrono>
#include <mutex>
#include <unordered_map>
#include <cstdarg>
#include <cstdio>
#include <memory>

enum class ZLogLevel {
    UNSET = 0,
    DEBUG = 10,
    INFO  = 20,
    WARN  = 30,
    ERROR = 40,
    FATAL = 50
};

class ZLoggerPC {
public:
    ZLoggerPC(const std::string& name, ZLogLevel level = ZLogLevel::INFO)
        : name_(name), level_(level) {}

    const std::string& get_name() const { return name_; }
    ZLogLevel get_level() const { return level_; }
    void set_level(ZLogLevel level) { level_ = level; }

    bool enabled(ZLogLevel level) const {
        return static_cast<int>(level) >= static_cast<int>(level_);
    }

    void log(ZLogLevel level, const char* level_str, const char* color, const char* fmt, ...) {
        if (!enabled(level)) return;

        char msg_buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(msg_buf, sizeof(msg_buf), fmt, args);
        va_end(args);

        std::lock_guard<std::mutex> lock(mutex_);
        std::cerr << color << "[" << level_str << "] [" << name_ << "]: " 
                  << msg_buf << "\033[0m\n";
    }

private:
    std::string name_;
    ZLogLevel level_;
    mutable std::mutex mutex_;
};

inline std::shared_ptr<ZLoggerPC> z_get_logger(const std::string& name) {
    static std::unordered_map<std::string, std::shared_ptr<ZLoggerPC>> loggers;
    static std::mutex reg_mutex;

    std::lock_guard<std::mutex> lock(reg_mutex);
    auto it = loggers.find(name);
    if (it == loggers.end()) {
        auto logger = std::make_shared<ZLoggerPC>(name);
        loggers[name] = logger;
        return logger;
    }
    return it->second;
}

#define _ZLOG_COLOR_CYAN    "\033[36m"
#define _ZLOG_COLOR_WHITE   "\033[37m"
#define _ZLOG_COLOR_YELLOW  "\033[33m"
#define _ZLOG_COLOR_RED     "\033[31m"
#define _ZLOG_COLOR_BRED    "\033[1;31m"

#define ZLOG_DEBUG(logger, fmt, ...) (logger)->log(ZLogLevel::DEBUG, "DEBUG", _ZLOG_COLOR_CYAN, fmt, ##__VA_ARGS__)
#define ZLOG_INFO(logger, fmt, ...)  (logger)->log(ZLogLevel::INFO,  "INFO",  _ZLOG_COLOR_WHITE, fmt, ##__VA_ARGS__)
#define ZLOG_WARN(logger, fmt, ...)  (logger)->log(ZLogLevel::WARN,  "WARN",  _ZLOG_COLOR_YELLOW, fmt, ##__VA_ARGS__)
#define ZLOG_ERROR(logger, fmt, ...) (logger)->log(ZLogLevel::ERROR, "ERROR", _ZLOG_COLOR_RED, fmt, ##__VA_ARGS__)
#define ZLOG_FATAL(logger, fmt, ...) (logger)->log(ZLogLevel::FATAL, "FATAL", _ZLOG_COLOR_BRED, fmt, ##__VA_ARGS__)

#define ZLOG_DEBUG_ONCE(logger, fmt, ...) do { static bool _fired = false; if (!_fired) { _fired = true; ZLOG_DEBUG(logger, fmt, ##__VA_ARGS__); } } while(0)
#define ZLOG_INFO_ONCE(logger, fmt, ...)  do { static bool _fired = false; if (!_fired) { _fired = true; ZLOG_INFO(logger, fmt, ##__VA_ARGS__); } } while(0)
#define ZLOG_WARN_ONCE(logger, fmt, ...)  do { static bool _fired = false; if (!_fired) { _fired = true; ZLOG_WARN(logger, fmt, ##__VA_ARGS__); } } while(0)
#define ZLOG_ERROR_ONCE(logger, fmt, ...) do { static bool _fired = false; if (!_fired) { _fired = true; ZLOG_ERROR(logger, fmt, ##__VA_ARGS__); } } while(0)
#define ZLOG_FATAL_ONCE(logger, fmt, ...) do { static bool _fired = false; if (!_fired) { _fired = true; ZLOG_FATAL(logger, fmt, ##__VA_ARGS__); } } while(0)

#define ZLOG_DEBUG_THROTTLE(logger, ms, fmt, ...) do { static auto _last = std::chrono::steady_clock::now(); auto _now = std::chrono::steady_clock::now(); if (std::chrono::duration_cast<std::chrono::milliseconds>(_now - _last).count() >= (ms)) { _last = _now; ZLOG_DEBUG(logger, fmt, ##__VA_ARGS__); } } while(0)
#define ZLOG_INFO_THROTTLE(logger, ms, fmt, ...)  do { static auto _last = std::chrono::steady_clock::now(); auto _now = std::chrono::steady_clock::now(); if (std::chrono::duration_cast<std::chrono::milliseconds>(_now - _last).count() >= (ms)) { _last = _now; ZLOG_INFO(logger, fmt, ##__VA_ARGS__); } } while(0)
#define ZLOG_WARN_THROTTLE(logger, ms, fmt, ...)  do { static auto _last = std::chrono::steady_clock::now(); auto _now = std::chrono::steady_clock::now(); if (std::chrono::duration_cast<std::chrono::milliseconds>(_now - _last).count() >= (ms)) { _last = _now; ZLOG_WARN(logger, fmt, ##__VA_ARGS__); } } while(0)
#define ZLOG_ERROR_THROTTLE(logger, ms, fmt, ...) do { static auto _last = std::chrono::steady_clock::now(); auto _now = std::chrono::steady_clock::now(); if (std::chrono::duration_cast<std::chrono::milliseconds>(_now - _last).count() >= (ms)) { _last = _now; ZLOG_ERROR(logger, fmt, ##__VA_ARGS__); } } while(0)
#define ZLOG_FATAL_THROTTLE(logger, ms, fmt, ...) do { static auto _last = std::chrono::steady_clock::now(); auto _now = std::chrono::steady_clock::now(); if (std::chrono::duration_cast<std::chrono::milliseconds>(_now - _last).count() >= (ms)) { _last = _now; ZLOG_FATAL(logger, fmt, ##__VA_ARGS__); } } while(0)

#define ZLOG_DEBUG_EXPRESSION(logger, expr, fmt, ...) do { if (expr) ZLOG_DEBUG(logger, fmt, ##__VA_ARGS__); } while(0)
#define ZLOG_INFO_EXPRESSION(logger, expr, fmt, ...)  do { if (expr) ZLOG_INFO(logger, fmt, ##__VA_ARGS__); } while(0)
#define ZLOG_WARN_EXPRESSION(logger, expr, fmt, ...)  do { if (expr) ZLOG_WARN(logger, fmt, ##__VA_ARGS__); } while(0)
#define ZLOG_ERROR_EXPRESSION(logger, expr, fmt, ...) do { if (expr) ZLOG_ERROR(logger, fmt, ##__VA_ARGS__); } while(0)
#define ZLOG_FATAL_EXPRESSION(logger, expr, fmt, ...) do { if (expr) ZLOG_FATAL(logger, fmt, ##__VA_ARGS__); } while(0)

#endif // Z_LOGGER_PC_H
