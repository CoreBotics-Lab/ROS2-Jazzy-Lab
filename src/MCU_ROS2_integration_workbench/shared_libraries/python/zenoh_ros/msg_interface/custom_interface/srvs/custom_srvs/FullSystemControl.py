import msgpack
from typing import Any, List, Optional, cast
from zenoh_ros.custom_msgs import z_ComplexRobotState
from zenoh_ros.custom_msgs import z_SensorTelemetry
class z_FullSystemControl:
    class Request:
        def __init__(self, target_state: z_ComplexRobotState = z_ComplexRobotState(), command_code: int = 0) -> None:
            self.target_state = target_state
            self.command_code = command_code

        def serialize(self) -> bytes:
            return cast(bytes, msgpack.packb({"target_state": msgpack.unpackb(self.target_state.serialize()), "command_code": self.command_code}))

        @classmethod
        def deserialize(cls, payload: bytes) -> 'Request':
            data = msgpack.unpackb(payload)
            if not isinstance(data, dict):
                data = {}
            return cls(z_ComplexRobotState.deserialize(msgpack.packb((data.get(b"target_state", data.get("target_state", {})) or {}))), int(data.get(b"command_code", data.get("command_code", 0))))

        def __repr__(self) -> str:
            return f"custom_srvs.z_FullSystemControl.Request(target_state={self.target_state}, command_code={self.command_code})"

    class Response:
        def __init__(self, current_telemetry: z_SensorTelemetry = z_SensorTelemetry(), ack: bool = False, status_details: str = "") -> None:
            self.current_telemetry = current_telemetry
            self.ack = ack
            self.status_details = status_details

        def serialize(self) -> bytes:
            return cast(bytes, msgpack.packb({"current_telemetry": msgpack.unpackb(self.current_telemetry.serialize()), "ack": self.ack, "status_details": self.status_details}))

        @classmethod
        def deserialize(cls, payload: bytes) -> 'Response':
            data = msgpack.unpackb(payload)
            if not isinstance(data, dict):
                data = {}
            return cls(z_SensorTelemetry.deserialize(msgpack.packb((data.get(b"current_telemetry", data.get("current_telemetry", {})) or {}))), bool(data.get(b"ack", data.get("ack", False))), (lambda v: v.decode("utf-8") if isinstance(v, bytes) else str(v))(data.get(b"status_details", data.get("status_details", ""))))

        def __repr__(self) -> str:
            return f"custom_srvs.z_FullSystemControl.Response(current_telemetry={self.current_telemetry}, ack={self.ack}, status_details={self.status_details})"
