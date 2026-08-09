"""
zenoh_ros.logging  —  ROS 2-style logger for zenoh_ros.

API mirrors rclpy.Logger exactly:

    logger.debug(msg, *, throttle_duration_sec=None, once=False, skip_first=False)
    logger.info(...)
    logger.warn(...)
    logger.error(...)
    logger.fatal(...)
    logger.set_level(LogLevel.DEBUG)
    child = logger.get_child("sub")

Global helpers (mirrors rclpy.logging):
    get_logger(name)               -> ZenohLogger
    set_logger_level(name, level)  -> None
    get_logger_effective_level(name) -> LogLevel

Every log line is also published to the Zenoh topic 'zenoh_ros/log'
(msgpack-serialized) so any remote subscriber can monitor logs live.
"""

from __future__ import annotations

import os
import sys
import time
import threading
import logging
import msgpack
from enum import IntEnum
from typing import Optional, Dict

# ─── Severity levels (identical to rcutils / rclpy) ──────────────────────────

class LogLevel(IntEnum):
    UNSET  = 0
    DEBUG  = 10
    INFO   = 20
    WARN   = 30
    ERROR  = 40
    FATAL  = 50

_LEVEL_NAME: Dict[int, str] = {
    LogLevel.DEBUG: "DEBUG",
    LogLevel.INFO:  "INFO",
    LogLevel.WARN:  "WARN",
    LogLevel.ERROR: "ERROR",
    LogLevel.FATAL: "FATAL",
}

# ─── ANSI colors (exactly matching ROS 2 / rclcpp terminal colors) ────────────

class _Color:
    RESET    = "\033[0m"
    CYAN     = "\033[36m"      # DEBUG
    WHITE    = ""              # INFO  (default terminal — no code needed)
    YELLOW   = "\033[33m"      # WARN
    RED      = "\033[31m"      # ERROR
    BOLD_RED = "\033[1;31m"    # FATAL

_LEVEL_COLOR: Dict[int, str] = {
    LogLevel.DEBUG: _Color.CYAN,
    LogLevel.INFO:  _Color.WHITE,
    LogLevel.WARN:  _Color.YELLOW,
    LogLevel.ERROR: _Color.RED,
    LogLevel.FATAL: _Color.BOLD_RED,
}

_USE_COLORS: bool = (
    os.getenv("RCUTILS_COLORIZED_OUTPUT", "") == "1"
    or (
        os.getenv("RCUTILS_COLORIZED_OUTPUT", "") != "0"
        and hasattr(sys.stderr, "isatty")
        and sys.stderr.isatty()
    )
)

# ─── Global logger registry ───────────────────────────────────────────────────

_registry_lock  = threading.Lock()
_registry: Dict[str, "ZenohLogger"] = {}

# ─── Zenoh log publisher (optional, wired by ZenohNode.init()) ───────────────

_zenoh_session = None
_publisher_cache: Dict[str, Any] = {}
_publisher_lock = threading.Lock()


def _set_zenoh_session(session) -> None:
    """Called by ZenohNode.init() to attach a Zenoh session for log publishing."""
    global _zenoh_session
    with _publisher_lock:
        _zenoh_session = session


def _publish_log(level: int, name: str, message: str,
                 stamp_ns: int, file: str, line: int, func: str) -> None:
    global _zenoh_session, _publisher_cache
    if _zenoh_session is None:
        return
    try:
        topic = f"{name}/log"
        with _publisher_lock:
            if topic not in _publisher_cache:
                _publisher_cache[topic] = _zenoh_session.declare_publisher(topic)
            pub = _publisher_cache[topic]

        lv_str = _LEVEL_NAME.get(level, "INFO")
        formatted = f"[{lv_str}] [{name}]: {message}"
        pub.put(formatted.encode("utf-8"))
    except Exception:
        pass  # logging must never crash the caller


# ─── File logging helper ──────────────────────────────────────────────────────

def _get_log_dir() -> str:
    ros_log_dir = os.getenv("ROS_LOG_DIR", "")
    if ros_log_dir:
        return os.path.expanduser(ros_log_dir)
    ros_home = os.getenv("ROS_HOME", os.path.expanduser("~/.ros"))
    return os.path.join(ros_home, "log")


# ─── ZenohLogger ─────────────────────────────────────────────────────────────

class ZenohLogger:
    """
    ROS 2-compatible logger for zenoh_ros. API is identical to rclpy.Logger.

    Quick start::

        from zenoh_ros.logging import get_logger, LogLevel
        logger = get_logger("my_node")
        logger.info("Node started")
        logger.warn("Low battery: %.1fV", 3.1, throttle_duration_sec=5.0)
        logger.error("Sensor dropout", once=True)
        logger.fatal("Out of memory")
        child = logger.get_child("imu")   # name -> "my_node.imu"
    """

    def __init__(self, name: str,
                 level: LogLevel = LogLevel.INFO,
                 log_to_file: bool = False) -> None:
        self._name:         str       = name
        self._level:        LogLevel  = level
        self._log_to_file:  bool      = log_to_file
        self._lock:         threading.Lock = threading.Lock()

        # Per-callsite state: throttle timestamps, once flags, skip_first flags
        self._throttle_t:   Dict[str, float] = {}
        self._once_fired:   Dict[str, bool]  = {}
        self._skip_fired:   Dict[str, bool]  = {}

        # Optional file handler
        self._fh: Optional[logging.FileHandler] = None
        if log_to_file:
            log_dir = _get_log_dir()
            os.makedirs(log_dir, exist_ok=True)
            safe = name.replace("/", "_").replace(".", "_")
            self._fh = logging.FileHandler(
                os.path.join(log_dir, f"{safe}.log"), encoding="utf-8")
            self._fh.setFormatter(logging.Formatter("%(message)s"))

    # ── Properties ────────────────────────────────────────────────────────────

    @property
    def name(self) -> str:
        return self._name

    # ── Level control ─────────────────────────────────────────────────────────

    def set_level(self, level: LogLevel) -> None:
        """Set minimum severity. Mirrors rclpy.Logger.set_level()."""
        with self._lock:
            self._level = level

    def get_level(self) -> LogLevel:
        """Return current minimum severity."""
        return self._level

    # ── Child loggers ─────────────────────────────────────────────────────────

    def get_child(self, suffix: str) -> "ZenohLogger":
        """
        Return a child logger named '{name}.{suffix}'.
        Inherits this logger's level and file-logging setting.
        """
        return get_logger(f"{self._name}.{suffix}",
                          default_level=self._level,
                          log_to_file=self._log_to_file)

    # ── Severity methods (identical signature to rclpy) ───────────────────────

    def debug(self, msg: str, *args,
              throttle_duration_sec: Optional[float] = None,
              once: bool = False, skip_first: bool = False) -> None:
        self._log(LogLevel.DEBUG, msg, args, throttle_duration_sec, once, skip_first)

    def info(self, msg: str, *args,
             throttle_duration_sec: Optional[float] = None,
             once: bool = False, skip_first: bool = False) -> None:
        self._log(LogLevel.INFO, msg, args, throttle_duration_sec, once, skip_first)

    def warn(self, msg: str, *args,
             throttle_duration_sec: Optional[float] = None,
             once: bool = False, skip_first: bool = False) -> None:
        self._log(LogLevel.WARN, msg, args, throttle_duration_sec, once, skip_first)

    def error(self, msg: str, *args,
              throttle_duration_sec: Optional[float] = None,
              once: bool = False, skip_first: bool = False) -> None:
        self._log(LogLevel.ERROR, msg, args, throttle_duration_sec, once, skip_first)

    def fatal(self, msg: str, *args,
              throttle_duration_sec: Optional[float] = None,
              once: bool = False, skip_first: bool = False) -> None:
        self._log(LogLevel.FATAL, msg, args, throttle_duration_sec, once, skip_first)

    # ── Internal ──────────────────────────────────────────────────────────────

    def _log(self, level: int, msg: str, args: tuple,
             throttle_duration_sec: Optional[float],
             once: bool, skip_first: bool) -> None:
        if level < self._level:
            return

        # Capture call-site info (frame 2 = caller of debug/info/warn/...)
        frame    = sys._getframe(2)
        site_id  = f"{frame.f_code.co_filename}:{frame.f_lineno}"
        file_name = os.path.basename(frame.f_code.co_filename)
        func_name = frame.f_code.co_name
        line_no   = frame.f_lineno

        now     = time.monotonic()
        wall_ns = int(time.time_ns())

        with self._lock:
            # once: print only on first call at this site
            if once:
                if self._once_fired.get(site_id, False):
                    return
                self._once_fired[site_id] = True

            # skip_first: suppress the very first call at this site
            if skip_first:
                if not self._skip_fired.get(site_id, False):
                    self._skip_fired[site_id] = True
                    return

            # throttle: rate-limit per call site
            if throttle_duration_sec is not None:
                if (now - self._throttle_t.get(site_id, 0.0)) < throttle_duration_sec:
                    return
                self._throttle_t[site_id] = now

        # Format the message (supports printf-style %)
        try:
            formatted = msg % args if args else str(msg)
        except Exception:
            formatted = str(msg)

        # Build log line — matches ROS 2 default format
        lv_str   = _LEVEL_NAME.get(level, "UNKNOWN")
        line_out = f"[{lv_str}] [{self._name}]: {formatted}"

        # Print to stderr with optional ANSI color
        if _USE_COLORS:
            c = _LEVEL_COLOR.get(level, "")
            print(f"{c}{line_out}{_Color.RESET}", file=sys.stderr, flush=True)
        else:
            print(line_out, file=sys.stderr, flush=True)

        # Write to file
        if self._fh is not None:
            record = logging.LogRecord(
                name=self._name, level=level,
                pathname=frame.f_code.co_filename, lineno=line_no,
                msg=line_out, args=(), exc_info=None)
            self._fh.emit(record)

        # Publish to Zenoh topic
        _publish_log(level, self._name, formatted,
                     wall_ns, file_name, line_no, func_name)


# ─── Module-level API (mirrors rclpy.logging) ────────────────────────────────

def get_logger(name: str,
               default_level: LogLevel = LogLevel.INFO,
               log_to_file: bool = False) -> ZenohLogger:
    """
    Return existing logger by name, or create a new one.
    Mirrors rclpy.logging.get_logger().
    """
    with _registry_lock:
        if name not in _registry:
            _registry[name] = ZenohLogger(name, level=default_level,
                                          log_to_file=log_to_file)
        return _registry[name]


def set_logger_level(name: str, level: LogLevel) -> None:
    """
    Set level for a named logger and all its children.
    Mirrors rclpy.logging.set_logger_level().
    """
    with _registry_lock:
        for logger_name, logger in _registry.items():
            if logger_name == name or logger_name.startswith(name + "."):
                logger.set_level(level)


def get_logger_effective_level(name: str) -> LogLevel:
    """
    Walk up the logger hierarchy to find the effective level.
    Mirrors rclpy.logging.get_logger_effective_level().
    """
    with _registry_lock:
        parts = name.split(".")
        for length in range(len(parts), 0, -1):
            candidate = ".".join(parts[:length])
            if candidate in _registry:
                lvl = _registry[candidate].get_level()
                if lvl != LogLevel.UNSET:
                    return lvl
    return LogLevel.INFO


def run_log_viewer() -> None:
    """Launch the internal log viewer to monitor /zenoh_ros/log live."""
    from .viewer import main
    main()


__all__ = [
    "ZenohLogger",
    "LogLevel",
    "get_logger",
    "set_logger_level",
    "get_logger_effective_level",
    "_set_zenoh_session",
]
