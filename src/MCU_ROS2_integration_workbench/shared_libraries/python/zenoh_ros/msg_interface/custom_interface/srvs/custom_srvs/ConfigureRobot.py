import msgpack
from typing import Any, List, Optional, cast
from zenoh_ros.custom_msgs import z_MotorStatus
class z_ConfigureRobot:
    class Request:
        def __init__(self, target_status: z_MotorStatus = z_MotorStatus(), mode: int = 0) -> None:
            self.target_status = target_status
            self.mode = mode

        def serialize(self) -> bytes:
            return cast(bytes, msgpack.packb({"target_status": msgpack.unpackb(self.target_status.serialize()), "mode": self.mode}))

        @classmethod
        def deserialize(cls, payload: bytes) -> 'Request':
            data = msgpack.unpackb(payload)
            if not isinstance(data, dict):
                data = {}
            return cls(z_MotorStatus.deserialize(msgpack.packb((data.get(b"target_status", data.get("target_status", {})) or {}))), int(data.get(b"mode", data.get("mode", 0))))

        def __repr__(self) -> str:
            return f"custom_srvs.z_ConfigureRobot.Request(target_status={self.target_status}, mode={self.mode})"

    class Response:
        def __init__(self, success: bool = False, status_message: str = "") -> None:
            self.success = success
            self.status_message = status_message

        def serialize(self) -> bytes:
            return cast(bytes, msgpack.packb({"success": self.success, "status_message": self.status_message}))

        @classmethod
        def deserialize(cls, payload: bytes) -> 'Response':
            data = msgpack.unpackb(payload)
            if not isinstance(data, dict):
                data = {}
            return cls(bool(data.get(b"success", data.get("success", False))), (lambda v: v.decode("utf-8") if isinstance(v, bytes) else str(v))(data.get(b"status_message", data.get("status_message", ""))))

        def __repr__(self) -> str:
            return f"custom_srvs.z_ConfigureRobot.Response(success={self.success}, status_message={self.status_message})"
