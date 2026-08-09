import msgpack
from typing import Optional, cast

# =============================================================================
# builtin_interfaces — Python
#
# Mirrors the ROS 2 builtin_interfaces package:
#   Duration: { int32 sec, uint32 nanosec }
#   Time    : { int32 sec, uint32 nanosec }
# =============================================================================


class z_Duration:
    """
    Equivalent of builtin_interfaces/msg/Duration.

    ROS 2 definition:
        int32  sec      # Seconds component (may be negative)
        uint32 nanosec  # Nanoseconds component [0, 10^9)
    """
    def __init__(self, sec: int = 0, nanosec: int = 0) -> None:
        self.sec: int = sec
        self.nanosec: int = nanosec

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({"sec": self.sec, "nanosec": self.nanosec}))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Duration':
        data = msgpack.unpackb(payload)
        if not isinstance(data, dict):
            data = {}
        sec     = int(data.get(b"sec",     data.get("sec",     0)))
        nanosec = int(data.get(b"nanosec", data.get("nanosec", 0)))
        return cls(sec, nanosec)

    def __repr__(self) -> str:
        return f"builtin_interfaces.z_Duration(sec={self.sec}, nanosec={self.nanosec})"


class z_Time:
    """
    Equivalent of builtin_interfaces/msg/Time.

    ROS 2 definition:
        int32  sec      # Seconds since epoch (may be negative before Unix epoch)
        uint32 nanosec  # Nanoseconds component [0, 10^9)
    """
    def __init__(self, sec: int = 0, nanosec: int = 0) -> None:
        self.sec: int = sec
        self.nanosec: int = nanosec

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({"sec": self.sec, "nanosec": self.nanosec}))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Time':
        data = msgpack.unpackb(payload)
        if not isinstance(data, dict):
            data = {}
        sec     = int(data.get(b"sec",     data.get("sec",     0)))
        nanosec = int(data.get(b"nanosec", data.get("nanosec", 0)))
        return cls(sec, nanosec)

    def __repr__(self) -> str:
        return f"builtin_interfaces.z_Time(sec={self.sec}, nanosec={self.nanosec})"
