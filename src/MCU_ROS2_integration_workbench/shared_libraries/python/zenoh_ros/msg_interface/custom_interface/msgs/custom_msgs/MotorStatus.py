import msgpack
from typing import Any, List, Optional, cast
class z_MotorStatus:
    def __init__(self, motor_id: int = 0, speed: float = 0.0, temperature: float = 0.0, is_active: bool = False) -> None:
        self.motor_id = motor_id
        self.speed = speed
        self.temperature = temperature
        self.is_active = is_active

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({"motor_id": self.motor_id, "speed": self.speed, "temperature": self.temperature, "is_active": self.is_active}))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_MotorStatus':
        data = msgpack.unpackb(payload)
        # Handle empty/invalid payload
        if not isinstance(data, dict):
            data = {}
        return cls(int(data.get(b"motor_id", data.get("motor_id", 0))), float(data.get(b"speed", data.get("speed", 0.0))), float(data.get(b"temperature", data.get("temperature", 0.0))), bool(data.get(b"is_active", data.get("is_active", False))))

    def __repr__(self) -> str:
        return f"custom_msgs.z_MotorStatus(motor_id={self.motor_id}, speed={self.speed}, temperature={self.temperature}, is_active={self.is_active})"
