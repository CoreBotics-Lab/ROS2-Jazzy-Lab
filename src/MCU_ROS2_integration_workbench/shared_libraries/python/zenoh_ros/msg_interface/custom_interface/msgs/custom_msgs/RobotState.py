import msgpack
from typing import Any, List, Optional, cast
from zenoh_ros.msg_interface.pre_defined_interface import z_geometry_msgs
class z_RobotState:
    def __init__(self, name: str = "", position: z_geometry_msgs.z_Vector3 = z_geometry_msgs.z_Vector3(), velocity: z_geometry_msgs.z_Twist = z_geometry_msgs.z_Twist()) -> None:
        self.name = name
        self.position = position
        self.velocity = velocity

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({"name": self.name, "position": [self.position.x, self.position.y, self.position.z], "velocity": [
                [self.velocity.linear.x, self.velocity.linear.y, self.velocity.linear.z],
                [self.velocity.angular.x, self.velocity.angular.y, self.velocity.angular.z]
            ]}))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_RobotState':
        data = msgpack.unpackb(payload)
        # Handle empty/invalid payload
        if not isinstance(data, dict):
            data = {}
        return cls((lambda v: v.decode("utf-8") if isinstance(v, bytes) else str(v))(data.get(b"name", data.get("name", ""))), z_geometry_msgs.z_Vector3(
                float(data.get(b"position", data.get("position", [0,0,0]))[0]),
                float(data.get(b"position", data.get("position", [0,0,0]))[1]),
                float(data.get(b"position", data.get("position", [0,0,0]))[2])
            ), z_geometry_msgs.z_Twist(
                linear=z_geometry_msgs.z_Vector3(
                    float(data.get(b"velocity", data.get("velocity", [[0,0,0],[0,0,0]]))[0][0]),
                    float(data.get(b"velocity", data.get("velocity", [[0,0,0],[0,0,0]]))[0][1]),
                    float(data.get(b"velocity", data.get("velocity", [[0,0,0],[0,0,0]]))[0][2])
                ),
                angular=z_geometry_msgs.z_Vector3(
                    float(data.get(b"velocity", data.get("velocity", [[0,0,0],[0,0,0]]))[1][0]),
                    float(data.get(b"velocity", data.get("velocity", [[0,0,0],[0,0,0]]))[1][1]),
                    float(data.get(b"velocity", data.get("velocity", [[0,0,0],[0,0,0]]))[1][2])
                )
            ))

    def __repr__(self) -> str:
        return f"custom_msgs.z_RobotState(name={self.name}, position={self.position}, velocity={self.velocity})"
