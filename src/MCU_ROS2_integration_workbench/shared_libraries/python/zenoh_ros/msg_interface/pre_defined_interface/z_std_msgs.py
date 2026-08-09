import msgpack
from typing import Any, List, Optional, cast

class z_Empty:
    def __init__(self) -> None:
        pass

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Empty':
        return cls()


class z_Raw:
    def __init__(self, data: bytes = b"") -> None:
        self.data: bytes = data

    def serialize(self) -> bytes:
        return self.data

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Raw':
        return cls(payload)


# --- Helper class factory for primitive message types ---
def _make_primitive_msg(cls_name: str, default_val: Any):
    class PrimitiveMessage:
        def __init__(self, data: Any = default_val) -> None:
            self.data = data

        def serialize(self) -> bytes:
            return cast(bytes, msgpack.packb([self.data]))

        @classmethod
        def deserialize(cls, payload: bytes) -> 'PrimitiveMessage':
            unpacked = msgpack.unpackb(payload)
            val = unpacked[0] if isinstance(unpacked, (list, tuple)) else unpacked
            return cls(val)

        def __repr__(self) -> str:
            return f"z_std_msgs.{cls_name}(data={self.data})"

    PrimitiveMessage.__name__ = cls_name
    PrimitiveMessage.__qualname__ = cls_name
    return PrimitiveMessage


# --- Helper class factory for array message types ---
def _make_array_msg(cls_name: str):
    class ArrayMessage:
        def __init__(self, data: Optional[List[Any]] = None) -> None:
            self.data = data if data is not None else []

        def serialize(self) -> bytes:
            return cast(bytes, msgpack.packb(self.data))

        @classmethod
        def deserialize(cls, payload: bytes) -> 'ArrayMessage':
            val = msgpack.unpackb(payload)
            if not isinstance(val, list):
                val = list(val) if isinstance(val, tuple) else [val]
            return cls(val)

        def __repr__(self) -> str:
            return f"z_std_msgs.{cls_name}(data={self.data})"

    ArrayMessage.__name__ = cls_name
    ArrayMessage.__qualname__ = cls_name
    return ArrayMessage


# Create all standard primitives
z_Bool = _make_primitive_msg("z_Bool", False)
z_Byte = _make_primitive_msg("z_Byte", 0)
z_Char = _make_primitive_msg("z_Char", 0)
z_Int8 = _make_primitive_msg("z_Int8", 0)
z_UInt8 = _make_primitive_msg("z_UInt8", 0)
z_Int16 = _make_primitive_msg("z_Int16", 0)
z_UInt16 = _make_primitive_msg("z_UInt16", 0)
z_Int32 = _make_primitive_msg("z_Int32", 0)
z_UInt32 = _make_primitive_msg("z_UInt32", 0)
z_Int64 = _make_primitive_msg("z_Int64", 0)
z_UInt64 = _make_primitive_msg("z_UInt64", 0)
z_Float32 = _make_primitive_msg("z_Float32", 0.0)
z_Float64 = _make_primitive_msg("z_Float64", 0.0)
z_String = _make_primitive_msg("z_String", "")

# Create all standard arrays
z_ByteMultiArray = _make_array_msg("z_ByteMultiArray")
z_Int8MultiArray = _make_array_msg("z_Int8MultiArray")
z_UInt8MultiArray = _make_array_msg("z_UInt8MultiArray")
z_Int16MultiArray = _make_array_msg("z_Int16MultiArray")
z_UInt16MultiArray = _make_array_msg("z_UInt16MultiArray")
z_Int32MultiArray = _make_array_msg("z_Int32MultiArray")
z_UInt32MultiArray = _make_array_msg("z_UInt32MultiArray")
z_Int64MultiArray = _make_array_msg("z_Int64MultiArray")
z_UInt64MultiArray = _make_array_msg("z_UInt64MultiArray")
z_Float32MultiArray = _make_array_msg("z_Float32MultiArray")
z_Float64MultiArray = _make_array_msg("z_Float64MultiArray")


# =============================================================================
# std_msgs extensions that depend on builtin_interfaces
# =============================================================================
from .z_builtin_interfaces import z_Time


class z_ColorRGBA:
    """
    Equivalent of std_msgs/msg/ColorRGBA.

    ROS 2 definition:
        float32 r
        float32 g
        float32 b
        float32 a
    """
    def __init__(self, r: float = 0.0, g: float = 0.0,
                 b: float = 0.0, a: float = 1.0) -> None:
        self.r: float = r
        self.g: float = g
        self.b: float = b
        self.a: float = a

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({"r": self.r, "g": self.g,
                                          "b": self.b, "a": self.a}))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_ColorRGBA':
        data = msgpack.unpackb(payload)
        if not isinstance(data, dict):
            data = {}
        return cls(
            r=float(data.get(b"r", data.get("r", 0.0))),
            g=float(data.get(b"g", data.get("g", 0.0))),
            b=float(data.get(b"b", data.get("b", 0.0))),
            a=float(data.get(b"a", data.get("a", 1.0))),
        )

    def __repr__(self) -> str:
        return f"z_std_msgs.z_ColorRGBA(r={self.r}, g={self.g}, b={self.b}, a={self.a})"


class z_Header:
    """
    Equivalent of std_msgs/msg/Header.

    ROS 2 definition:
        builtin_interfaces/Time stamp   # Timestamp of the associated data
        string frame_id                 # Coordinate frame this data is associated with
    """
    def __init__(self,
                 stamp: Optional[z_Time] = None,
                 frame_id: str = "") -> None:
        self.stamp: z_Time = stamp if stamp is not None else z_Time()
        self.frame_id: str = frame_id

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({
            "stamp": {"sec": self.stamp.sec, "nanosec": self.stamp.nanosec},
            "frame_id": self.frame_id
        }))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Header':
        data = msgpack.unpackb(payload)
        if not isinstance(data, dict):
            data = {}
        stamp_raw = data.get(b"stamp", data.get("stamp", {}))
        if not isinstance(stamp_raw, dict):
            stamp_raw = {}
        sec     = int(stamp_raw.get(b"sec",     stamp_raw.get("sec",     0)))
        nanosec = int(stamp_raw.get(b"nanosec", stamp_raw.get("nanosec", 0)))
        frame_id = str(data.get(b"frame_id", data.get("frame_id", "")))
        return cls(stamp=z_Time(sec, nanosec), frame_id=frame_id)

    def __repr__(self) -> str:
        return f"z_std_msgs.z_Header(stamp={self.stamp}, frame_id='{self.frame_id}')"

