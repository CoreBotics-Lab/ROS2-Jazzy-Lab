import msgpack
from typing import Optional, cast, Any

class z_Vector3:
    def __init__(self, x: float = 0.0, y: float = 0.0, z: float = 0.0) -> None:
        self.x = x
        self.y = y
        self.z = z

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([self.x, self.y, self.z]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Vector3':
        data = msgpack.unpackb(payload)
        return cls(float(data[0]), float(data[1]), float(data[2]))

    def __repr__(self) -> str:
        return f"z_geometry_msgs.z_Vector3(x={self.x}, y={self.y}, z={self.z})"


class z_Point:
    def __init__(self, x: float = 0.0, y: float = 0.0, z: float = 0.0) -> None:
        self.x = x
        self.y = y
        self.z = z

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([self.x, self.y, self.z]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Point':
        data = msgpack.unpackb(payload)
        return cls(float(data[0]), float(data[1]), float(data[2]))

    def __repr__(self) -> str:
        return f"z_geometry_msgs.z_Point(x={self.x}, y={self.y}, z={self.z})"


class z_Quaternion:
    def __init__(self, x: float = 0.0, y: float = 0.0, z: float = 0.0, w: float = 1.0) -> None:
        self.x = x
        self.y = y
        self.z = z
        self.w = w

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([self.x, self.y, self.z, self.w]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Quaternion':
        data = msgpack.unpackb(payload)
        return cls(float(data[0]), float(data[1]), float(data[2]), float(data[3]))

    def __repr__(self) -> str:
        return f"z_geometry_msgs.z_Quaternion(x={self.x}, y={self.y}, z={self.z}, w={self.w})"


class z_Pose:
    def __init__(self, position: Optional[z_Point] = None, orientation: Optional[z_Quaternion] = None) -> None:
        self.position = position if position is not None else z_Point()
        self.orientation = orientation if orientation is not None else z_Quaternion()

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([
            [self.position.x, self.position.y, self.position.z],
            [self.orientation.x, self.orientation.y, self.orientation.z, self.orientation.w]
        ]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Pose':
        data = msgpack.unpackb(payload)
        pos = z_Point(float(data[0][0]), float(data[0][1]), float(data[0][2]))
        ori = z_Quaternion(float(data[1][0]), float(data[1][1]), float(data[1][2]), float(data[1][3]))
        return cls(pos, ori)

    def __repr__(self) -> str:
        return f"z_geometry_msgs.z_Pose(position={self.position}, orientation={self.orientation})"


class z_Twist:
    def __init__(self, linear: Optional[z_Vector3] = None, angular: Optional[z_Vector3] = None) -> None:
        self.linear = linear if linear is not None else z_Vector3()
        self.angular = angular if angular is not None else z_Vector3()

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([
            [self.linear.x, self.linear.y, self.linear.z],
            [self.angular.x, self.angular.y, self.angular.z]
        ]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Twist':
        data = msgpack.unpackb(payload)
        lin = z_Vector3(float(data[0][0]), float(data[0][1]), float(data[0][2]))
        ang = z_Vector3(float(data[1][0]), float(data[1][1]), float(data[1][2]))
        return cls(lin, ang)

    def __repr__(self) -> str:
        return f"z_geometry_msgs.z_Twist(linear={self.linear}, angular={self.angular})"


class z_Wrench:
    def __init__(self, force: Optional[z_Vector3] = None, torque: Optional[z_Vector3] = None) -> None:
        self.force = force if force is not None else z_Vector3()
        self.torque = torque if torque is not None else z_Vector3()

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([
            [self.force.x, self.force.y, self.force.z],
            [self.torque.x, self.torque.y, self.torque.z]
        ]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Wrench':
        data = msgpack.unpackb(payload)
        force = z_Vector3(float(data[0][0]), float(data[0][1]), float(data[0][2]))
        torque = z_Vector3(float(data[1][0]), float(data[1][1]), float(data[1][2]))
        return cls(force, torque)

    def __repr__(self) -> str:
        return f"z_geometry_msgs.z_Wrench(force={self.force}, torque={self.torque})"


class z_Transform:
    def __init__(self, translation: Optional[z_Vector3] = None, rotation: Optional[z_Quaternion] = None) -> None:
        self.translation = translation if translation is not None else z_Vector3()
        self.rotation = rotation if rotation is not None else z_Quaternion()

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([
            [self.translation.x, self.translation.y, self.translation.z],
            [self.rotation.x, self.rotation.y, self.rotation.z, self.rotation.w]
        ]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Transform':
        data = msgpack.unpackb(payload)
        trans = z_Vector3(float(data[0][0]), float(data[0][1]), float(data[0][2]))
        rot = z_Quaternion(float(data[1][0]), float(data[1][1]), float(data[1][2]), float(data[1][3]))
        return cls(trans, rot)

    def __repr__(self) -> str:
        return f"z_geometry_msgs.z_Transform(translation={self.translation}, rotation={self.rotation})"


class z_Pose2D:
    def __init__(self, x: float = 0.0, y: float = 0.0, theta: float = 0.0) -> None:
        self.x = x
        self.y = y
        self.theta = theta

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([self.x, self.y, self.theta]))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Pose2D':
        data = msgpack.unpackb(payload)
        return cls(float(data[0]), float(data[1]), float(data[2]))

    def __repr__(self) -> str:
        return f"z_geometry_msgs.z_Pose2D(x={self.x}, y={self.y}, theta={self.theta})"

# ─── NEW types for ROS 2 Jazzy parity ────────────────────────────────────────
from typing import List
from .z_builtin_interfaces import z_Time

class z_Point32:
    def __init__(self, x: float = 0.0, y: float = 0.0, z: float = 0.0):
        self.x = x; self.y = y; self.z = z
    def serialize(self) -> bytes: return cast(bytes, msgpack.packb([self.x, self.y, self.z]))
    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Point32':
        data = msgpack.unpackb(payload); return cls(float(data[0]), float(data[1]), float(data[2]))
    def __repr__(self): return f"z_Point32(x={self.x}, y={self.y}, z={self.z})"

class z_Accel:
    def __init__(self, linear=None, angular=None):
        self.linear = linear if linear is not None else z_Vector3()
        self.angular = angular if angular is not None else z_Vector3()
    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb([[self.linear.x, self.linear.y, self.linear.z], [self.angular.x, self.angular.y, self.angular.z]]))
    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Accel':
        data = msgpack.unpackb(payload)
        return cls(z_Vector3(float(data[0][0]), float(data[0][1]), float(data[0][2])), z_Vector3(float(data[1][0]), float(data[1][1]), float(data[1][2])))
    def __repr__(self): return f"z_Accel(linear={self.linear}, angular={self.angular})"

def _gh(d, k, default=None):
    return d.get(k.encode() if isinstance(k, str) else k, d.get(k, default))

class z_PoseStamped:
    def __init__(self, stamp=None, frame_id: str = "", pose=None):
        self.stamp = stamp if stamp is not None else z_Time()
        self.frame_id = frame_id
        self.pose = pose if pose is not None else z_Pose()
    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({
            "header": {"stamp": {"sec": self.stamp.sec, "nanosec": self.stamp.nanosec}, "frame_id": self.frame_id},
            "pose": {"position": [self.pose.position.x, self.pose.position.y, self.pose.position.z],
                     "orientation": [self.pose.orientation.x, self.pose.orientation.y, self.pose.orientation.z, self.pose.orientation.w]}
        }))
    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_PoseStamped':
        data = msgpack.unpackb(payload)
        hdr = _gh(data,"header",{}); stamp_d = _gh(hdr,"stamp",{})
        stamp = z_Time(sec=int(_gh(stamp_d,"sec",0)), nanosec=int(_gh(stamp_d,"nanosec",0)))
        frame_id = str(_gh(hdr,"frame_id",""))
        pd = _gh(data,"pose",{}); pos = _gh(pd,"position",[0,0,0]); ori = _gh(pd,"orientation",[0,0,0,1])
        pose = z_Pose(z_Point(float(pos[0]), float(pos[1]), float(pos[2])), z_Quaternion(float(ori[0]), float(ori[1]), float(ori[2]), float(ori[3])))
        return cls(stamp=stamp, frame_id=frame_id, pose=pose)
    def __repr__(self): return f"z_PoseStamped(frame_id='{self.frame_id}', pose={self.pose})"

class z_TwistStamped:
    def __init__(self, stamp=None, frame_id: str = "", twist=None):
        self.stamp = stamp if stamp is not None else z_Time()
        self.frame_id = frame_id
        self.twist = twist if twist is not None else z_Twist()
    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({
            "header": {"stamp": {"sec": self.stamp.sec, "nanosec": self.stamp.nanosec}, "frame_id": self.frame_id},
            "twist": {"linear": [self.twist.linear.x, self.twist.linear.y, self.twist.linear.z],
                      "angular": [self.twist.angular.x, self.twist.angular.y, self.twist.angular.z]}
        }))
    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_TwistStamped':
        data = msgpack.unpackb(payload)
        hdr = _gh(data,"header",{}); stamp_d = _gh(hdr,"stamp",{})
        stamp = z_Time(sec=int(_gh(stamp_d,"sec",0)), nanosec=int(_gh(stamp_d,"nanosec",0)))
        td = _gh(data,"twist",{}); lin = _gh(td,"linear",[0,0,0]); ang = _gh(td,"angular",[0,0,0])
        twist = z_Twist(z_Vector3(float(lin[0]),float(lin[1]),float(lin[2])), z_Vector3(float(ang[0]),float(ang[1]),float(ang[2])))
        return cls(stamp=stamp, frame_id=str(_gh(hdr,"frame_id","")), twist=twist)
    def __repr__(self): return f"z_TwistStamped(frame_id='{self.frame_id}', twist={self.twist})"

class z_TransformStamped:
    def __init__(self, stamp=None, frame_id: str = "", child_frame_id: str = "", transform=None):
        self.stamp = stamp if stamp is not None else z_Time()
        self.frame_id = frame_id; self.child_frame_id = child_frame_id
        self.transform = transform if transform is not None else z_Transform()
    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({
            "header": {"stamp": {"sec": self.stamp.sec, "nanosec": self.stamp.nanosec}, "frame_id": self.frame_id},
            "child_frame_id": self.child_frame_id,
            "transform": {"translation": [self.transform.translation.x, self.transform.translation.y, self.transform.translation.z],
                          "rotation": [self.transform.rotation.x, self.transform.rotation.y, self.transform.rotation.z, self.transform.rotation.w]}
        }))
    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_TransformStamped':
        data = msgpack.unpackb(payload)
        hdr = _gh(data,"header",{}); stamp_d = _gh(hdr,"stamp",{})
        stamp = z_Time(sec=int(_gh(stamp_d,"sec",0)), nanosec=int(_gh(stamp_d,"nanosec",0)))
        td = _gh(data,"transform",{}); tr = _gh(td,"translation",[0,0,0]); rot = _gh(td,"rotation",[0,0,0,1])
        transform = z_Transform(z_Vector3(float(tr[0]),float(tr[1]),float(tr[2])), z_Quaternion(float(rot[0]),float(rot[1]),float(rot[2]),float(rot[3])))
        return cls(stamp=stamp, frame_id=str(_gh(hdr,"frame_id","")), child_frame_id=str(_gh(data,"child_frame_id","")), transform=transform)
    def __repr__(self): return f"z_TransformStamped(frame_id='{self.frame_id}' -> '{self.child_frame_id}')"
