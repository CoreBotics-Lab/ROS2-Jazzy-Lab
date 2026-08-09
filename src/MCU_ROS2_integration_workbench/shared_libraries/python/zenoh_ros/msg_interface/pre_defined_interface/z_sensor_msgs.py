import msgpack
from typing import List, Optional, cast
from .z_std_msgs import z_Header
from .z_geometry_msgs import z_Vector3, z_Quaternion, z_Pose, z_Twist
from .z_builtin_interfaces import z_Time

class z_Imu:
    def __init__(self,
                 header: Optional[z_Header] = None,
                 orientation: Optional[z_Quaternion] = None,
                 orientation_covariance: Optional[List[float]] = None,
                 angular_velocity: Optional[z_Vector3] = None,
                 angular_velocity_covariance: Optional[List[float]] = None,
                 linear_acceleration: Optional[z_Vector3] = None,
                 linear_acceleration_covariance: Optional[List[float]] = None) -> None:
        self.header = header if header is not None else z_Header()
        self.orientation = orientation if orientation is not None else z_Quaternion()
        self.orientation_covariance = orientation_covariance if orientation_covariance is not None else [0.0]*9
        self.angular_velocity = angular_velocity if angular_velocity is not None else z_Vector3()
        self.angular_velocity_covariance = angular_velocity_covariance if angular_velocity_covariance is not None else [0.0]*9
        self.linear_acceleration = linear_acceleration if linear_acceleration is not None else z_Vector3()
        self.linear_acceleration_covariance = linear_acceleration_covariance if linear_acceleration_covariance is not None else [0.0]*9

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({
            "header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id},
            "orientation": {"x": self.orientation.x, "y": self.orientation.y, "z": self.orientation.z, "w": self.orientation.w},
            "orientation_covariance": self.orientation_covariance,
            "angular_velocity": {"x": self.angular_velocity.x, "y": self.angular_velocity.y, "z": self.angular_velocity.z},
            "angular_velocity_covariance": self.angular_velocity_covariance,
            "linear_acceleration": {"x": self.linear_acceleration.x, "y": self.linear_acceleration.y, "z": self.linear_acceleration.z},
            "linear_acceleration_covariance": self.linear_acceleration_covariance
        }))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Imu':
        data = msgpack.unpackb(payload)
        if not isinstance(data, dict): data = {}
        
        hdr_data = data.get(b"header", data.get("header", {}))
        if not isinstance(hdr_data, dict): hdr_data = {}
        
        stamp_data = hdr_data.get(b"stamp", hdr_data.get("stamp", {}))
        if not isinstance(stamp_data, dict): stamp_data = {}
        
        sec_val = stamp_data.get(b"sec", stamp_data.get("sec", 0))
        nanosec_val = stamp_data.get(b"nanosec", stamp_data.get("nanosec", 0))
        frame_val = hdr_data.get(b"frame_id", hdr_data.get("frame_id", ""))
        if frame_val is None: frame_val = ""
        if isinstance(frame_val, bytes): frame_val = frame_val.decode('utf-8', errors='ignore')
        
        header = z_Header(
            stamp=z_Time(sec=int(sec_val) if sec_val is not None else 0, nanosec=int(nanosec_val) if nanosec_val is not None else 0),
            frame_id=str(frame_val)
        )
        
        ori_d = data.get(b"orientation", data.get("orientation", {}))
        if not isinstance(ori_d, dict): ori_d = {}
        ori = z_Quaternion(
            float(ori_d.get(b"x", ori_d.get("x", 0.0)) or 0.0),
            float(ori_d.get(b"y", ori_d.get("y", 0.0)) or 0.0),
            float(ori_d.get(b"z", ori_d.get("z", 0.0)) or 0.0),
            float(ori_d.get(b"w", ori_d.get("w", 1.0)) or 1.0)
        )
        
        ang_d = data.get(b"angular_velocity", data.get("angular_velocity", {}))
        if not isinstance(ang_d, dict): ang_d = {}
        ang = z_Vector3(
            float(ang_d.get(b"x", ang_d.get("x", 0.0)) or 0.0),
            float(ang_d.get(b"y", ang_d.get("y", 0.0)) or 0.0),
            float(ang_d.get(b"z", ang_d.get("z", 0.0)) or 0.0)
        )
        
        lin_d = data.get(b"linear_acceleration", data.get("linear_acceleration", {}))
        if not isinstance(lin_d, dict): lin_d = {}
        lin = z_Vector3(
            float(lin_d.get(b"x", lin_d.get("x", 0.0)) or 0.0),
            float(lin_d.get(b"y", lin_d.get("y", 0.0)) or 0.0),
            float(lin_d.get(b"z", lin_d.get("z", 0.0)) or 0.0)
        )
        return cls(header=header, orientation=ori, angular_velocity=ang, linear_acceleration=lin)

    def __repr__(self) -> str:
        return f"z_sensor_msgs.z_Imu(header={self.header}, orientation={self.orientation}, angular_velocity={self.angular_velocity}, linear_acceleration={self.linear_acceleration})"


class z_Temperature:
    def __init__(self, header: Optional[z_Header] = None, temperature: float = 0.0, variance: float = 0.0) -> None:
        self.header = header if header is not None else z_Header()
        self.temperature = temperature
        self.variance = variance

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({
            "header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id},
            "temperature": self.temperature,
            "variance": self.variance
        }))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Temperature':
        data = msgpack.unpackb(payload)
        if not isinstance(data, dict): data = {}
        hdr_data = data.get(b"header", data.get("header", {}))
        stamp_data = hdr_data.get(b"stamp", hdr_data.get("stamp", {}))
        header = z_Header(
            stamp=z_Time(sec=int(stamp_data.get(b"sec", stamp_data.get("sec", 0))), nanosec=int(stamp_data.get(b"nanosec", stamp_data.get("nanosec", 0)))),
            frame_id=str(hdr_data.get(b"frame_id", hdr_data.get("frame_id", "")))
        )
        return cls(header=header, temperature=float(data.get(b"temperature", data.get("temperature", 0.0))), variance=float(data.get(b"variance", data.get("variance", 0.0))))

    def __repr__(self) -> str:
        return f"z_sensor_msgs.z_Temperature(header={self.header}, temperature={self.temperature}, variance={self.variance})"


class z_Range:
    def __init__(self, header: Optional[z_Header] = None, radiation_type: int = 0, field_of_view: float = 0.0, min_range: float = 0.0, max_range: float = 0.0, range: float = 0.0) -> None:
        self.header = header if header is not None else z_Header()
        self.radiation_type = radiation_type
        self.field_of_view = field_of_view
        self.min_range = min_range
        self.max_range = max_range
        self.range = range

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({
            "header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id},
            "radiation_type": self.radiation_type,
            "field_of_view": self.field_of_view,
            "min_range": self.min_range,
            "max_range": self.max_range,
            "range": self.range
        }))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Range':
        data = msgpack.unpackb(payload)
        if not isinstance(data, dict): data = {}
        hdr_data = data.get(b"header", data.get("header", {}))
        stamp_data = hdr_data.get(b"stamp", hdr_data.get("stamp", {}))
        header = z_Header(
            stamp=z_Time(sec=int(stamp_data.get(b"sec", stamp_data.get("sec", 0))), nanosec=int(stamp_data.get(b"nanosec", stamp_data.get("nanosec", 0)))),
            frame_id=str(hdr_data.get(b"frame_id", hdr_data.get("frame_id", "")))
        )
        return cls(
            header=header,
            radiation_type=int(data.get(b"radiation_type", data.get("radiation_type", 0))),
            field_of_view=float(data.get(b"field_of_view", data.get("field_of_view", 0.0))),
            min_range=float(data.get(b"min_range", data.get("min_range", 0.0))),
            max_range=float(data.get(b"max_range", data.get("max_range", 0.0))),
            range=float(data.get(b"range", data.get("range", 0.0)))
        )

    def __repr__(self) -> str:
        return f"z_sensor_msgs.z_Range(header={self.header}, range={self.range})"

# ─── NEW types for ROS 2 Jazzy parity ────────────────────────────────────────
from .z_builtin_interfaces import z_Time

def _gd(d, k, default=None):
    """Helper: get from msgpack dict by bytes or str key."""
    return d.get(k.encode() if isinstance(k, str) else k, d.get(k, default))


class z_RelativeHumidity:
    def __init__(self, header=None, relative_humidity: float = 0.0, variance: float = 0.0):
        self.header = header if header is not None else z_Header()
        self.relative_humidity = relative_humidity
        self.variance = variance

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({
            "header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id},
            "relative_humidity": self.relative_humidity, "variance": self.variance
        }))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_RelativeHumidity':
        data = msgpack.unpackb(payload)
        hdr = _gd(data, "header", {}); stamp = _gd(hdr, "stamp", {})
        header = z_Header(stamp=z_Time(sec=int(_gd(stamp, "sec", 0)), nanosec=int(_gd(stamp, "nanosec", 0))), frame_id=str(_gd(hdr, "frame_id", "")))
        return cls(header=header, relative_humidity=float(_gd(data, "relative_humidity", 0.0)), variance=float(_gd(data, "variance", 0.0)))

    def __repr__(self): return f"z_RelativeHumidity(rh={self.relative_humidity})"


class z_RegionOfInterest:
    def __init__(self, x_offset: int = 0, y_offset: int = 0, height: int = 0, width: int = 0, do_rectify: bool = False):
        self.x_offset = x_offset; self.y_offset = y_offset; self.height = height; self.width = width; self.do_rectify = do_rectify

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({"x_offset": self.x_offset, "y_offset": self.y_offset, "height": self.height, "width": self.width, "do_rectify": self.do_rectify}))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_RegionOfInterest':
        data = msgpack.unpackb(payload)
        return cls(x_offset=int(_gd(data,"x_offset",0)), y_offset=int(_gd(data,"y_offset",0)), height=int(_gd(data,"height",0)), width=int(_gd(data,"width",0)), do_rectify=bool(_gd(data,"do_rectify",False)))

    def __repr__(self): return f"z_RegionOfInterest({self.width}x{self.height})"


class z_NavSatStatus:
    STATUS_NO_FIX  = -1
    STATUS_FIX     = 0
    STATUS_SBAS_FIX = 1
    STATUS_GBAS_FIX = 2
    SERVICE_GPS    = 1
    SERVICE_GLONASS = 2
    SERVICE_COMPASS = 4
    SERVICE_GALILEO = 8

    def __init__(self, status: int = -1, service: int = 1):
        self.status = status; self.service = service

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({"status": self.status, "service": self.service}))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_NavSatStatus':
        data = msgpack.unpackb(payload)
        return cls(status=int(_gd(data,"status",-1)), service=int(_gd(data,"service",1)))

    def __repr__(self): return f"z_NavSatStatus(status={self.status})"


class z_NavSatFix:
    COVARIANCE_TYPE_UNKNOWN         = 0
    COVARIANCE_TYPE_APPROXIMATED    = 1
    COVARIANCE_TYPE_DIAGONAL_KNOWN  = 2
    COVARIANCE_TYPE_KNOWN           = 3

    def __init__(self, header=None, status=None, latitude: float = 0.0, longitude: float = 0.0, altitude: float = 0.0,
                 position_covariance=None, position_covariance_type: int = 0):
        self.header = header if header is not None else z_Header()
        self.status = status if status is not None else z_NavSatStatus()
        self.latitude = latitude; self.longitude = longitude; self.altitude = altitude
        self.position_covariance = position_covariance if position_covariance is not None else [0.0]*9
        self.position_covariance_type = position_covariance_type

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({
            "header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id},
            "status": {"status": self.status.status, "service": self.status.service},
            "latitude": self.latitude, "longitude": self.longitude, "altitude": self.altitude,
            "position_covariance": self.position_covariance, "position_covariance_type": self.position_covariance_type
        }))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_NavSatFix':
        data = msgpack.unpackb(payload)
        hdr = _gd(data,"header",{}); stamp = _gd(hdr,"stamp",{})
        header = z_Header(stamp=z_Time(sec=int(_gd(stamp,"sec",0)), nanosec=int(_gd(stamp,"nanosec",0))), frame_id=str(_gd(hdr,"frame_id","")))
        sd = _gd(data,"status",{}); status = z_NavSatStatus(status=int(_gd(sd,"status",-1)), service=int(_gd(sd,"service",1)))
        return cls(header=header, status=status, latitude=float(_gd(data,"latitude",0.0)), longitude=float(_gd(data,"longitude",0.0)),
                   altitude=float(_gd(data,"altitude",0.0)), position_covariance_type=int(_gd(data,"position_covariance_type",0)))

    def __repr__(self): return f"z_NavSatFix(lat={self.latitude:.6f}, lon={self.longitude:.6f}, alt={self.altitude:.2f})"


class z_BatteryState:
    POWER_SUPPLY_STATUS_UNKNOWN     = 0
    POWER_SUPPLY_STATUS_CHARGING    = 1
    POWER_SUPPLY_STATUS_DISCHARGING = 2
    POWER_SUPPLY_STATUS_NOT_CHARGING = 3
    POWER_SUPPLY_STATUS_FULL        = 4
    POWER_SUPPLY_HEALTH_UNKNOWN     = 0
    POWER_SUPPLY_HEALTH_GOOD        = 1
    POWER_SUPPLY_TECHNOLOGY_UNKNOWN = 0
    POWER_SUPPLY_TECHNOLOGY_NIMH    = 1
    POWER_SUPPLY_TECHNOLOGY_LION    = 2
    POWER_SUPPLY_TECHNOLOGY_LIPO    = 3
    POWER_SUPPLY_TECHNOLOGY_LIFE    = 4
    POWER_SUPPLY_TECHNOLOGY_NICD    = 5
    POWER_SUPPLY_TECHNOLOGY_LIMN    = 6

    def __init__(self, header=None, voltage: float = 0.0, temperature: float = float('nan'),
                 current: float = float('nan'), charge: float = float('nan'), capacity: float = float('nan'),
                 design_capacity: float = float('nan'), percentage: float = 0.0,
                 power_supply_status: int = 0, power_supply_health: int = 0,
                 power_supply_technology: int = 0, present: bool = True):
        self.header = header if header is not None else z_Header()
        self.voltage = voltage; self.temperature = temperature; self.current = current
        self.charge = charge; self.capacity = capacity; self.design_capacity = design_capacity
        self.percentage = percentage; self.power_supply_status = power_supply_status
        self.power_supply_health = power_supply_health; self.power_supply_technology = power_supply_technology
        self.present = present

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({
            "header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id},
            "voltage": self.voltage, "temperature": self.temperature, "current": self.current,
            "charge": self.charge, "capacity": self.capacity, "design_capacity": self.design_capacity,
            "percentage": self.percentage, "power_supply_status": self.power_supply_status,
            "power_supply_health": self.power_supply_health, "power_supply_technology": self.power_supply_technology,
            "present": self.present
        }))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_BatteryState':
        data = msgpack.unpackb(payload)
        hdr = _gd(data,"header",{}); stamp = _gd(hdr,"stamp",{})
        header = z_Header(stamp=z_Time(sec=int(_gd(stamp,"sec",0)), nanosec=int(_gd(stamp,"nanosec",0))), frame_id=str(_gd(hdr,"frame_id","")))
        return cls(header=header, voltage=float(_gd(data,"voltage",0.0)), temperature=float(_gd(data,"temperature",0.0)),
                   current=float(_gd(data,"current",0.0)), charge=float(_gd(data,"charge",0.0)),
                   capacity=float(_gd(data,"capacity",0.0)), design_capacity=float(_gd(data,"design_capacity",0.0)),
                   percentage=float(_gd(data,"percentage",0.0)), power_supply_status=int(_gd(data,"power_supply_status",0)),
                   power_supply_health=int(_gd(data,"power_supply_health",0)), power_supply_technology=int(_gd(data,"power_supply_technology",0)),
                   present=bool(_gd(data,"present",True)))

    def __repr__(self): return f"z_BatteryState(voltage={self.voltage:.2f}V, {self.percentage*100:.1f}%)"


class z_JoyFeedback:
    TYPE_LED        = 0
    TYPE_RUMBLE     = 1
    TYPE_BUZZER     = 2

    def __init__(self, type: int = 0, id: int = 0, intensity: float = 0.0):
        self.type = type; self.id = id; self.intensity = intensity

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({"type": self.type, "id": self.id, "intensity": self.intensity}))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_JoyFeedback':
        data = msgpack.unpackb(payload)
        return cls(type=int(_gd(data,"type",0)), id=int(_gd(data,"id",0)), intensity=float(_gd(data,"intensity",0.0)))

    def __repr__(self): return f"z_JoyFeedback(type={self.type}, intensity={self.intensity})"


class z_TimeReference:
    def __init__(self, header=None, time_ref=None, source: str = ""):
        self.header = header if header is not None else z_Header()
        self.time_ref = time_ref if time_ref is not None else z_Time()
        self.source = source

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({
            "header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id},
            "time_ref": {"sec": self.time_ref.sec, "nanosec": self.time_ref.nanosec},
            "source": self.source
        }))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_TimeReference':
        data = msgpack.unpackb(payload)
        hdr = _gd(data,"header",{}); stamp = _gd(hdr,"stamp",{})
        header = z_Header(stamp=z_Time(sec=int(_gd(stamp,"sec",0)), nanosec=int(_gd(stamp,"nanosec",0))), frame_id=str(_gd(hdr,"frame_id","")))
        tr = _gd(data,"time_ref",{}); time_ref = z_Time(sec=int(_gd(tr,"sec",0)), nanosec=int(_gd(tr,"nanosec",0)))
        return cls(header=header, time_ref=time_ref, source=str(_gd(data,"source","")))

    def __repr__(self): return f"z_TimeReference(source='{self.source}')"


class z_Joy:
    """PC-only full serialize; MCU gets struct-only."""
    def __init__(self, header=None, axes=None, buttons=None):
        self.header = header if header is not None else z_Header()
        self.axes = axes if axes is not None else []
        self.buttons = buttons if buttons is not None else []

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({
            "header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id},
            "axes": self.axes, "buttons": self.buttons
        }))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Joy':
        data = msgpack.unpackb(payload)
        hdr = _gd(data,"header",{}); stamp = _gd(hdr,"stamp",{})
        header = z_Header(stamp=z_Time(sec=int(_gd(stamp,"sec",0)), nanosec=int(_gd(stamp,"nanosec",0))), frame_id=str(_gd(hdr,"frame_id","")))
        return cls(header=header, axes=[float(v) for v in _gd(data,"axes",[])], buttons=[int(v) for v in _gd(data,"buttons",[])])

    def __repr__(self): return f"z_Joy(axes={self.axes}, buttons={self.buttons})"


class z_LaserScan:
    """PC-only full serialize; MCU gets struct-only."""
    def __init__(self, header=None, angle_min: float = 0.0, angle_max: float = 0.0, angle_increment: float = 0.0,
                 time_increment: float = 0.0, scan_time: float = 0.0, range_min: float = 0.0, range_max: float = 0.0,
                 ranges=None, intensities=None):
        self.header = header if header is not None else z_Header()
        self.angle_min = angle_min; self.angle_max = angle_max; self.angle_increment = angle_increment
        self.time_increment = time_increment; self.scan_time = scan_time
        self.range_min = range_min; self.range_max = range_max
        self.ranges = ranges if ranges is not None else []
        self.intensities = intensities if intensities is not None else []

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({
            "header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id},
            "angle_min": self.angle_min, "angle_max": self.angle_max, "angle_increment": self.angle_increment,
            "time_increment": self.time_increment, "scan_time": self.scan_time,
            "range_min": self.range_min, "range_max": self.range_max,
            "ranges": self.ranges, "intensities": self.intensities
        }))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_LaserScan':
        data = msgpack.unpackb(payload)
        hdr = _gd(data,"header",{}); stamp = _gd(hdr,"stamp",{})
        header = z_Header(stamp=z_Time(sec=int(_gd(stamp,"sec",0)), nanosec=int(_gd(stamp,"nanosec",0))), frame_id=str(_gd(hdr,"frame_id","")))
        return cls(header=header, angle_min=float(_gd(data,"angle_min",0.0)), angle_max=float(_gd(data,"angle_max",0.0)),
                   angle_increment=float(_gd(data,"angle_increment",0.0)), time_increment=float(_gd(data,"time_increment",0.0)),
                   scan_time=float(_gd(data,"scan_time",0.0)), range_min=float(_gd(data,"range_min",0.0)), range_max=float(_gd(data,"range_max",0.0)),
                   ranges=[float(v) for v in _gd(data,"ranges",[])], intensities=[float(v) for v in _gd(data,"intensities",[])])

    def __repr__(self): return f"z_LaserScan(ranges={len(self.ranges)} pts)"


class z_FluidPressure:
    def __init__(self, header=None, fluid_pressure: float = 0.0, variance: float = 0.0):
        self.header = header if header is not None else z_Header()
        self.fluid_pressure = fluid_pressure; self.variance = variance
    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({"header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id}, "fluid_pressure": self.fluid_pressure, "variance": self.variance}))
    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_FluidPressure':
        data = msgpack.unpackb(payload)
        hdr = _gd(data,"header",{}); stamp = _gd(hdr,"stamp",{})
        header = z_Header(stamp=z_Time(sec=int(_gd(stamp,"sec",0)), nanosec=int(_gd(stamp,"nanosec",0))), frame_id=str(_gd(hdr,"frame_id","")))
        return cls(header=header, fluid_pressure=float(_gd(data,"fluid_pressure",0.0)), variance=float(_gd(data,"variance",0.0)))
    def __repr__(self): return f"z_FluidPressure(pa={self.fluid_pressure})"


class z_Illuminance:
    def __init__(self, header=None, illuminance: float = 0.0, variance: float = 0.0):
        self.header = header if header is not None else z_Header()
        self.illuminance = illuminance; self.variance = variance
    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({"header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id}, "illuminance": self.illuminance, "variance": self.variance}))
    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Illuminance':
        data = msgpack.unpackb(payload)
        hdr = _gd(data,"header",{}); stamp = _gd(hdr,"stamp",{})
        header = z_Header(stamp=z_Time(sec=int(_gd(stamp,"sec",0)), nanosec=int(_gd(stamp,"nanosec",0))), frame_id=str(_gd(hdr,"frame_id","")))
        return cls(header=header, illuminance=float(_gd(data,"illuminance",0.0)), variance=float(_gd(data,"variance",0.0)))
    def __repr__(self): return f"z_Illuminance(lux={self.illuminance})"


class z_MagneticField:
    def __init__(self, header=None, magnetic_field=None):
        from .z_geometry_msgs import z_Vector3
        self.header = header if header is not None else z_Header()
        self.magnetic_field = magnetic_field if magnetic_field is not None else z_Vector3()
    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({"header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id}, "magnetic_field": {"x": self.magnetic_field.x, "y": self.magnetic_field.y, "z": self.magnetic_field.z}}))
    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_MagneticField':
        from .z_geometry_msgs import z_Vector3
        data = msgpack.unpackb(payload)
        hdr = _gd(data,"header",{}); stamp = _gd(hdr,"stamp",{})
        header = z_Header(stamp=z_Time(sec=int(_gd(stamp,"sec",0)), nanosec=int(_gd(stamp,"nanosec",0))), frame_id=str(_gd(hdr,"frame_id","")))
        mf = _gd(data,"magnetic_field",{})
        mag = z_Vector3(float(_gd(mf,"x",0.0)), float(_gd(mf,"y",0.0)), float(_gd(mf,"z",0.0)))
        return cls(header=header, magnetic_field=mag)
    def __repr__(self): return f"z_MagneticField(field={self.magnetic_field})"


class z_JointState:
    def __init__(self, header=None, name=None, position=None, velocity=None, effort=None):
        self.header = header if header is not None else z_Header()
        self.name = name if name is not None else []
        self.position = position if position is not None else []
        self.velocity = velocity if velocity is not None else []
        self.effort = effort if effort is not None else []
    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({"header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id}, "name": self.name, "position": self.position, "velocity": self.velocity, "effort": self.effort}))
    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_JointState':
        data = msgpack.unpackb(payload)
        hdr = _gd(data,"header",{}); stamp = _gd(hdr,"stamp",{})
        header = z_Header(stamp=z_Time(sec=int(_gd(stamp,"sec",0)), nanosec=int(_gd(stamp,"nanosec",0))), frame_id=str(_gd(hdr,"frame_id","")))
        return cls(header=header, name=[str(n) for n in _gd(data,"name",[])], position=[float(v) for v in _gd(data,"position",[])], velocity=[float(v) for v in _gd(data,"velocity",[])], effort=[float(v) for v in _gd(data,"effort",[])])
    def __repr__(self): return f"z_JointState(joints={self.name})"
