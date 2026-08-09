import msgpack
from typing import Any, List, Optional, cast
class z_SetColor:
    class Request:
        def __init__(self, r: int = 0, g: int = 0, b: int = 0) -> None:
            self.r = r
            self.g = g
            self.b = b

        def serialize(self) -> bytes:
            return cast(bytes, msgpack.packb({"r": self.r, "g": self.g, "b": self.b}))

        @classmethod
        def deserialize(cls, payload: bytes) -> 'Request':
            data = msgpack.unpackb(payload)
            if not isinstance(data, dict):
                data = {}
            return cls(int(data.get(b"r", data.get("r", 0))), int(data.get(b"g", data.get("g", 0))), int(data.get(b"b", data.get("b", 0))))

        def __repr__(self) -> str:
            return f"custom_srvs.z_SetColor.Request(r={self.r}, g={self.g}, b={self.b})"

    class Response:
        def __init__(self, success: bool = False, message: str = "") -> None:
            self.success = success
            self.message = message

        def serialize(self) -> bytes:
            return cast(bytes, msgpack.packb({"success": self.success, "message": self.message}))

        @classmethod
        def deserialize(cls, payload: bytes) -> 'Response':
            data = msgpack.unpackb(payload)
            if not isinstance(data, dict):
                data = {}
            return cls(bool(data.get(b"success", data.get("success", False))), (lambda v: v.decode("utf-8") if isinstance(v, bytes) else str(v))(data.get(b"message", data.get("message", ""))))

        def __repr__(self) -> str:
            return f"custom_srvs.z_SetColor.Response(success={self.success}, message={self.message})"
