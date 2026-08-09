import msgpack
from typing import Any, List, Optional, cast

class z_Empty:
    class Request:
        def __init__(self) -> None:
            pass

        def serialize(self) -> bytes:
            return cast(bytes, msgpack.packb({}))

        @classmethod
        def deserialize(cls, payload: bytes) -> 'Request':
            return cls()

        def __repr__(self) -> str:
            return "z_std_srvs.z_Empty.Request()"

    class Response:
        def __init__(self) -> None:
            pass

        def serialize(self) -> bytes:
            return cast(bytes, msgpack.packb({}))

        @classmethod
        def deserialize(cls, payload: bytes) -> 'Response':
            return cls()

        def __repr__(self) -> str:
            return "z_std_srvs.z_Empty.Response()"


class z_Trigger:
    class Request:
        def __init__(self) -> None:
            pass

        def serialize(self) -> bytes:
            return cast(bytes, msgpack.packb({}))

        @classmethod
        def deserialize(cls, payload: bytes) -> 'Request':
            return cls()

        def __repr__(self) -> str:
            return "z_std_srvs.z_Trigger.Request()"

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
            success = bool(data.get(b"success", data.get("success", False)))
            raw_msg = data.get(b"message", data.get("message", ""))
            message = raw_msg.decode('utf-8') if isinstance(raw_msg, bytes) else str(raw_msg)
            return cls(success, message)

        def __repr__(self) -> str:
            return f"z_std_srvs.z_Trigger.Response(success={self.success}, message={self.message})"


class z_SetBool:
    class Request:
        def __init__(self, data: bool = False) -> None:
            self.data = data

        def serialize(self) -> bytes:
            return cast(bytes, msgpack.packb({"data": self.data}))

        @classmethod
        def deserialize(cls, payload: bytes) -> 'Request':
            data = msgpack.unpackb(payload)
            if not isinstance(data, dict):
                data = {}
            val = bool(data.get(b"data", data.get("data", False)))
            return cls(val)

        def __repr__(self) -> str:
            return f"z_std_srvs.z_SetBool.Request(data={self.data})"

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
            success = bool(data.get(b"success", data.get("success", False)))
            raw_msg = data.get(b"message", data.get("message", ""))
            message = raw_msg.decode('utf-8') if isinstance(raw_msg, bytes) else str(raw_msg)
            return cls(success, message)

        def __repr__(self) -> str:
            return f"z_std_srvs.z_SetBool.Response(success={self.success}, message={self.message})"
