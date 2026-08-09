import msgpack
from typing import List, Optional, cast
from .z_std_msgs import z_Header
from .z_geometry_msgs import z_Vector3, z_Quaternion, z_Pose, z_Twist
from .z_builtin_interfaces import z_Time

class z_Odometry:
    def __init__(self,
                 header: Optional[z_Header] = None,
                 child_frame_id: str = "",
                 pose: Optional[z_Pose] = None,
                 pose_covariance: Optional[List[float]] = None,
                 twist: Optional[z_Twist] = None,
                 twist_covariance: Optional[List[float]] = None) -> None:
        self.header = header if header is not None else z_Header()
        self.child_frame_id = child_frame_id
        self.pose = pose if pose is not None else z_Pose()
        self.pose_covariance = pose_covariance if pose_covariance is not None else [0.0]*36
        self.twist = twist if twist is not None else z_Twist()
        self.twist_covariance = twist_covariance if twist_covariance is not None else [0.0]*36

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({
            "header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id},
            "child_frame_id": self.child_frame_id,
            "pose": {
                "pose": {
                    "position": {"x": self.pose.position.x, "y": self.pose.position.y, "z": self.pose.position.z},
                    "orientation": {"x": self.pose.orientation.x, "y": self.pose.orientation.y, "z": self.pose.orientation.z, "w": self.pose.orientation.w}
                },
                "covariance": self.pose_covariance
            },
            "twist": {
                "twist": {
                    "linear": {"x": self.twist.linear.x, "y": self.twist.linear.y, "z": self.twist.linear.z},
                    "angular": {"x": self.twist.angular.x, "y": self.twist.angular.y, "z": self.twist.angular.z}
                },
                "covariance": self.twist_covariance
            }
        }))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Odometry':
        data = msgpack.unpackb(payload)
        if not isinstance(data, dict): data = {}
        hdr_data = data.get(b"header", data.get("header", {}))
        stamp_data = hdr_data.get(b"stamp", hdr_data.get("stamp", {}))
        header = z_Header(
            stamp=z_Time(sec=int(stamp_data.get(b"sec", stamp_data.get("sec", 0))), nanosec=int(stamp_data.get(b"nanosec", stamp_data.get("nanosec", 0)))),
            frame_id=str(hdr_data.get(b"frame_id", hdr_data.get("frame_id", "")))
        )
        child_frame_id = str(data.get(b"child_frame_id", data.get("child_frame_id", "")))
        
        p_d = data.get(b"pose", data.get("pose", {})).get(b"pose", data.get("pose", {}).get("pose", {}))
        pos_d = p_d.get(b"position", p_d.get("position", {}))
        ori_d = p_d.get(b"orientation", p_d.get("orientation", {}))
        pose = z_Pose(
            position=z_Vector3(float(pos_d.get(b"x", pos_d.get("x", 0.0))), float(pos_d.get(b"y", pos_d.get("y", 0.0))), float(pos_d.get(b"z", pos_d.get("z", 0.0)))),
            orientation=z_Quaternion(float(ori_d.get(b"x", ori_d.get("x", 0.0))), float(ori_d.get(b"y", ori_d.get("y", 0.0))), float(ori_d.get(b"z", ori_d.get("z", 0.0))), float(ori_d.get(b"w", ori_d.get("w", 1.0))))
        )

        t_d = data.get(b"twist", data.get("twist", {})).get(b"twist", data.get("twist", {}).get("twist", {}))
        lin_d = t_d.get(b"linear", t_d.get("linear", {}))
        ang_d = t_d.get(b"angular", t_d.get("angular", {}))
        twist = z_Twist(
            linear=z_Vector3(float(lin_d.get(b"x", lin_d.get("x", 0.0))), float(lin_d.get(b"y", lin_d.get("y", 0.0))), float(lin_d.get(b"z", lin_d.get("z", 0.0)))),
            angular=z_Vector3(float(ang_d.get(b"x", ang_d.get("x", 0.0))), float(ang_d.get(b"y", ang_d.get("y", 0.0))), float(ang_d.get(b"z", ang_d.get("z", 0.0))))
        )
        return cls(header=header, child_frame_id=child_frame_id, pose=pose, twist=twist)

    def __repr__(self) -> str:
        return f"z_nav_msgs.z_Odometry(header={self.header}, child_frame_id='{self.child_frame_id}', pose={self.pose}, twist={self.twist})"


class z_Path:
    def __init__(self, header=None, poses=None):
        self.header = header if header is not None else z_Header()
        self.poses = poses if poses is not None else []

    def serialize(self) -> bytes:
        pose_list = []
        for p in self.poses:
            pose_list.append([
                [p.pose.position.x, p.pose.position.y, p.pose.position.z],
                [p.pose.orientation.x, p.pose.orientation.y, p.pose.orientation.z, p.pose.orientation.w]
            ])
        return cast(bytes, msgpack.packb({
            "header": {"stamp": {"sec": self.header.stamp.sec, "nanosec": self.header.stamp.nanosec}, "frame_id": self.header.frame_id},
            "poses": pose_list
        }))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_Path':
        data = msgpack.unpackb(payload)
        def gd(d, k, default=None): return d.get(k.encode() if isinstance(k, str) else k, d.get(k, default))
        hdr = gd(data,"header",{}); stamp = gd(hdr,"stamp",{})
        header = z_Header(stamp=z_Time(sec=int(gd(stamp,"sec",0)), nanosec=int(gd(stamp,"nanosec",0))), frame_id=str(gd(hdr,"frame_id","")))
        from .z_geometry_msgs import z_Pose, z_Point, z_Quaternion
        poses = []
        for pa in gd(data,"poses",[]):
            pos = pa[0]; ori = pa[1]
            poses.append(z_Pose(z_Point(float(pos[0]),float(pos[1]),float(pos[2])), z_Quaternion(float(ori[0]),float(ori[1]),float(ori[2]),float(ori[3]))))
        return cls(header=header, poses=poses)

    def __repr__(self): return f"z_nav_msgs.z_Path(poses={len(self.poses)})"


class z_MapMetaData:
    def __init__(self, map_load_time=None, resolution: float = 0.0, width: int = 0, height: int = 0, origin=None):
        self.map_load_time = map_load_time if map_load_time is not None else z_Time()
        self.resolution = resolution; self.width = width; self.height = height
        from .z_geometry_msgs import z_Pose
        self.origin = origin if origin is not None else z_Pose()

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({
            "map_load_time": {"sec": self.map_load_time.sec, "nanosec": self.map_load_time.nanosec},
            "resolution": self.resolution, "width": self.width, "height": self.height
        }))

    @classmethod
    def deserialize(cls, payload: bytes) -> 'z_MapMetaData':
        data = msgpack.unpackb(payload)
        def gd(d, k, default=None): return d.get(k.encode() if isinstance(k, str) else k, d.get(k, default))
        ts = gd(data,"map_load_time",{})
        time = z_Time(sec=int(gd(ts,"sec",0)), nanosec=int(gd(ts,"nanosec",0)))
        return cls(map_load_time=time, resolution=float(gd(data,"resolution",0.0)), width=int(gd(data,"width",0)), height=int(gd(data,"height",0)))

    def __repr__(self): return f"z_MapMetaData({self.width}x{self.height} @ {self.resolution}m/cell)"
