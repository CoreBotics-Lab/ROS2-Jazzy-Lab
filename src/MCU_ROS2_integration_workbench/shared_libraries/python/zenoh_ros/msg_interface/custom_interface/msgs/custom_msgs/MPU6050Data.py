import msgpack
from typing import Any, List, Optional, cast
class z_MPU6050Data:
    def __init__(self, accel_x: float = 0.0, accel_y: float = 0.0, accel_z: float = 0.0, gyro_x: float = 0.0, gyro_y: float = 0.0, gyro_z: float = 0.0, temperature: float = 0.0) -> None:
        self.accel_x = accel_x
        self.accel_y = accel_y
        self.accel_z = accel_z
        self.gyro_x = gyro_x
        self.gyro_y = gyro_y
        self.gyro_z = gyro_z
        self.temperature = temperature

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({"accel_x": self.accel_x, "accel_y": self.accel_y, "accel_z": self.accel_z, "gyro_x": self.gyro_x, "gyro_y": self.gyro_y, "gyro_z": self.gyro_z, "temperature": self.temperature}))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_MPU6050Data':
        data = msgpack.unpackb(payload)
        # Handle empty/invalid payload
        if not isinstance(data, dict):
            data = {}
        return cls(float(data.get(b"accel_x", data.get("accel_x", 0.0))), float(data.get(b"accel_y", data.get("accel_y", 0.0))), float(data.get(b"accel_z", data.get("accel_z", 0.0))), float(data.get(b"gyro_x", data.get("gyro_x", 0.0))), float(data.get(b"gyro_y", data.get("gyro_y", 0.0))), float(data.get(b"gyro_z", data.get("gyro_z", 0.0))), float(data.get(b"temperature", data.get("temperature", 0.0))))

    def __repr__(self) -> str:
        return f"custom_msgs.z_MPU6050Data(accel_x={self.accel_x}, accel_y={self.accel_y}, accel_z={self.accel_z}, gyro_x={self.gyro_x}, gyro_y={self.gyro_y}, gyro_z={self.gyro_z}, temperature={self.temperature})"
