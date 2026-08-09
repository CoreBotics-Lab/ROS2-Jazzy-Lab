import msgpack
from typing import Any, List, Optional, cast
from zenoh_ros.msg_interface.pre_defined_interface import z_geometry_msgs
from zenoh_ros.msg_interface.pre_defined_interface import z_std_msgs
class z_SensorTelemetry:
    def __init__(self, header: z_std_msgs.z_Header = z_std_msgs.z_Header(), accel: z_geometry_msgs.z_Vector3 = z_geometry_msgs.z_Vector3(), orientation: z_geometry_msgs.z_Quaternion = z_geometry_msgs.z_Quaternion(), sensor_id: int = 0, temp: float = 0.0, status_ok: bool = False) -> None:
        self.header = header
        self.accel = accel
        self.orientation = orientation
        self.sensor_id = sensor_id
        self.temp = temp
        self.status_ok = status_ok

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({"header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id}, "accel": [self.accel.x, self.accel.y, self.accel.z], "orientation": [self.orientation.x, self.orientation.y, self.orientation.z, self.orientation.w], "sensor_id": self.sensor_id, "temp": self.temp, "status_ok": self.status_ok}))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_SensorTelemetry':
        data = msgpack.unpackb(payload)
        # Handle empty/invalid payload
        if not isinstance(data, dict):
            data = {}
        return cls(z_std_msgs.z_Header.deserialize(msgpack.packb((data.get(b"header", data.get("header", {})) or {}))), z_geometry_msgs.z_Vector3(
                float(data.get(b"accel", data.get("accel", [0,0,0]))[0]),
                float(data.get(b"accel", data.get("accel", [0,0,0]))[1]),
                float(data.get(b"accel", data.get("accel", [0,0,0]))[2])
            ), z_geometry_msgs.z_Quaternion(
                float(data.get(b"orientation", data.get("orientation", [0,0,0,1]))[0]),
                float(data.get(b"orientation", data.get("orientation", [0,0,0,1]))[1]),
                float(data.get(b"orientation", data.get("orientation", [0,0,0,1]))[2]),
                float(data.get(b"orientation", data.get("orientation", [0,0,0,1]))[3])
            ), int(data.get(b"sensor_id", data.get("sensor_id", 0))), float(data.get(b"temp", data.get("temp", 0.0))), bool(data.get(b"status_ok", data.get("status_ok", False))))

    def __repr__(self) -> str:
        return f"custom_msgs.z_SensorTelemetry(header={self.header}, accel={self.accel}, orientation={self.orientation}, sensor_id={self.sensor_id}, temp={self.temp}, status_ok={self.status_ok})"
