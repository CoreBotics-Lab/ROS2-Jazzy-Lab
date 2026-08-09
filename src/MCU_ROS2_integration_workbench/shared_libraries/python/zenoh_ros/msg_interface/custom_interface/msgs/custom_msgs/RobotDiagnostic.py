import msgpack
from typing import Any, List, Optional, cast
from zenoh_ros.custom_msgs import z_MotorStatus
from zenoh_ros.msg_interface.pre_defined_interface import z_geometry_msgs
from zenoh_ros.msg_interface.pre_defined_interface import z_std_msgs
class z_RobotDiagnostic:
    def __init__(self, header: z_std_msgs.z_Header = z_std_msgs.z_Header(), velocity: z_geometry_msgs.z_Vector3 = z_geometry_msgs.z_Vector3(), left_motor: z_MotorStatus = z_MotorStatus(), right_motor: z_MotorStatus = z_MotorStatus(), robot_name: str = "") -> None:
        self.header = header
        self.velocity = velocity
        self.left_motor = left_motor
        self.right_motor = right_motor
        self.robot_name = robot_name

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({"header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id}, "velocity": [self.velocity.x, self.velocity.y, self.velocity.z], "left_motor": msgpack.unpackb(self.left_motor.serialize()), "right_motor": msgpack.unpackb(self.right_motor.serialize()), "robot_name": self.robot_name}))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_RobotDiagnostic':
        data = msgpack.unpackb(payload)
        # Handle empty/invalid payload
        if not isinstance(data, dict):
            data = {}
        return cls(z_std_msgs.z_Header.deserialize(msgpack.packb((data.get(b"header", data.get("header", {})) or {}))), z_geometry_msgs.z_Vector3(
                float(data.get(b"velocity", data.get("velocity", [0,0,0]))[0]),
                float(data.get(b"velocity", data.get("velocity", [0,0,0]))[1]),
                float(data.get(b"velocity", data.get("velocity", [0,0,0]))[2])
            ), z_MotorStatus.deserialize(msgpack.packb((data.get(b"left_motor", data.get("left_motor", {})) or {}))), z_MotorStatus.deserialize(msgpack.packb((data.get(b"right_motor", data.get("right_motor", {})) or {}))), (lambda v: v.decode("utf-8") if isinstance(v, bytes) else str(v))(data.get(b"robot_name", data.get("robot_name", ""))))

    def __repr__(self) -> str:
        return f"custom_msgs.z_RobotDiagnostic(header={self.header}, velocity={self.velocity}, left_motor={self.left_motor}, right_motor={self.right_motor}, robot_name={self.robot_name})"
