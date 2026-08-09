import msgpack
from typing import Any, List, Optional, cast
from zenoh_ros.custom_msgs import z_SensorTelemetry
from zenoh_ros.msg_interface.pre_defined_interface import z_geometry_msgs
from zenoh_ros.msg_interface.pre_defined_interface import z_std_msgs
class z_ComplexRobotState:
    def __init__(self, header: z_std_msgs.z_Header = z_std_msgs.z_Header(), pose: z_geometry_msgs.z_Pose = z_geometry_msgs.z_Pose(), telemetry: z_SensorTelemetry = z_SensorTelemetry(), robot_mode: str = "", cycle_count: int = 0) -> None:
        self.header = header
        self.pose = pose
        self.telemetry = telemetry
        self.robot_mode = robot_mode
        self.cycle_count = cycle_count

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({"header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id}, "pose": msgpack.unpackb(self.pose.serialize()), "telemetry": msgpack.unpackb(self.telemetry.serialize()), "robot_mode": self.robot_mode, "cycle_count": self.cycle_count}))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_ComplexRobotState':
        data = msgpack.unpackb(payload)
        # Handle empty/invalid payload
        if not isinstance(data, dict):
            data = {}
        return cls(z_std_msgs.z_Header.deserialize(msgpack.packb((data.get(b"header", data.get("header", {})) or {}))), z_geometry_msgs.z_Pose.deserialize(msgpack.packb((data.get(b"pose", data.get("pose", {})) or {}))), z_SensorTelemetry.deserialize(msgpack.packb((data.get(b"telemetry", data.get("telemetry", {})) or {}))), (lambda v: v.decode("utf-8") if isinstance(v, bytes) else str(v))(data.get(b"robot_mode", data.get("robot_mode", ""))), int(data.get(b"cycle_count", data.get("cycle_count", 0))))

    def __repr__(self) -> str:
        return f"custom_msgs.z_ComplexRobotState(header={self.header}, pose={self.pose}, telemetry={self.telemetry}, robot_mode={self.robot_mode}, cycle_count={self.cycle_count})"
