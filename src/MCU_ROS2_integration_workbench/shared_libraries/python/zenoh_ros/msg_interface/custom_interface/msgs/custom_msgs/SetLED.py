import msgpack
from typing import Any, List, Optional, cast
class z_SetLED:
    def __init__(self, r: int = 0, g: int = 0, b: int = 0, brightness: int = 0, led_num: int = 0) -> None:
        self.r = r
        self.g = g
        self.b = b
        self.brightness = brightness
        self.led_num = led_num

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({"r": self.r, "g": self.g, "b": self.b, "brightness": self.brightness, "led_num": self.led_num}))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_SetLED':
        data = msgpack.unpackb(payload)
        # Handle empty/invalid payload
        if not isinstance(data, dict):
            data = {}
        return cls(int(data.get(b"r", data.get("r", 0))), int(data.get(b"g", data.get("g", 0))), int(data.get(b"b", data.get("b", 0))), int(data.get(b"brightness", data.get("brightness", 0))), int(data.get(b"led_num", data.get("led_num", 0))))

    def __repr__(self) -> str:
        return f"custom_msgs.z_SetLED(r={self.r}, g={self.g}, b={self.b}, brightness={self.brightness}, led_num={self.led_num})"
