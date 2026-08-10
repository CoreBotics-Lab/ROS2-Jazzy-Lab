#!/usr/bin/env python3
import sys
import os

# Resolve symlinks first so this works both when run directly and when
# installed via `colcon build --symlink-install` (where __file__ is a
# symlink inside /install/lib/ that points back to the source tree).
_this_dir = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(0, os.path.abspath(os.path.join(_this_dir, '../../../shared_libraries/python')))

import math
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from geometry_msgs.msg import TransformStamped
from tf2_ros import TransformBroadcaster
from tf_transformations import quaternion_from_euler

from zenoh_ros import ZenohNode, ZenohConfig
from zenoh_ros.sensor_msgs import z_Imu


# ─── ROS 2 TF Broadcaster Node ─────────────────────────────────────────────
class IMUTFBroadcasterNode(Node):
    """Pure rclpy Node — owns the TF Broadcaster for world -> imu_link."""

    def __init__(self) -> None:
        super().__init__("imu_tf_broadcaster")

        # Track orientation state (roll, pitch, yaw) calculated from IMU readings
        self.roll = 0.0
        self.pitch = 0.0
        self.yaw = 0.0
        self.last_time = None

        # TF Broadcaster
        self.tf_broadcaster_ = TransformBroadcaster(self)
        self.get_logger().info("IMU TF Broadcaster initialized (broadcasting world -> imu_link)...")

    def process_and_broadcast_tf(self, msg: z_Imu) -> None:
        """Calculate orientation from MCU IMU data and broadcast TF."""
        # Calculate Roll & Pitch from Accelerometer
        ax = msg.linear_acceleration.x
        ay = msg.linear_acceleration.y
        az = msg.linear_acceleration.z

        # Avoid division by zero when computing roll/pitch
        accel_norm = math.sqrt(ax * ax + ay * ay + az * az)
        if accel_norm > 1e-6:
            self.roll = math.atan2(ay, az)
            self.pitch = math.atan2(-ax, math.sqrt(ay * ay + az * az))

        # Integrate Gyroscope Z for Yaw (heading) drift tracking over dt
        sec = msg.header.stamp.sec
        nanosec = msg.header.stamp.nanosec
        if self.last_time is not None:
            last_sec, last_nanosec = self.last_time
            dt = (sec - last_sec) + (nanosec - last_nanosec) * 1e-9
            if dt > 0.0:
                self.yaw += msg.angular_velocity.z * dt
                # Normalize yaw between [-pi, pi]
                self.yaw = math.atan2(math.sin(self.yaw), math.cos(self.yaw))

        self.last_time = (sec, nanosec)

        # Convert Roll, Pitch, Yaw to Quaternion
        q = quaternion_from_euler(self.roll, self.pitch, self.yaw)

        # Broadcast world -> imu_link transform
        transform = TransformStamped()
        transform.header.stamp.sec = sec
        transform.header.stamp.nanosec = nanosec
        transform.header.frame_id = "world"
        transform.child_frame_id = "imu_link"

        transform.transform.translation.x = 0.0
        transform.transform.translation.y = 0.0
        transform.transform.translation.z = 0.0

        transform.transform.rotation.x = q[0]
        transform.transform.rotation.y = q[1]
        transform.transform.rotation.z = q[2]
        transform.transform.rotation.w = q[3]

        self.tf_broadcaster_.sendTransform(transform)


# ─── Zenoh MCU Subscriber Node ─────────────────────────────────────────────
class MPU6050TFSubscriberNode(ZenohNode):
    """ZenohNode — receives IMU data directly from ESP32-S3 MCU over Zenoh."""

    def __init__(self, tf_node: IMUTFBroadcasterNode) -> None:
        super().__init__("mpu6050_tf_subscriber")
        self._tf_node = tf_node
        self.get_logger().info("MCU Zenoh TF subscriber started")

        # Zenoh subscription — receives IMU data directly from MCU
        self.sub = self.z_create_subscription(
            z_Imu,
            "robot/mpu6050",
            self.listener_callback,
            10
        )

    def listener_callback(self, msg: z_Imu) -> None:
        # Forward directly to TF broadcaster
        self._tf_node.process_and_broadcast_tf(msg)


# ─── Entry Point ─────────────────────────────────────────────────────────────
def main(args=None) -> None:
    log = get_logger("System")
    ros2_node = None
    _zenoh_node = None

    try:
        # 1. Connect to ESP32-S3 SoftAP and initialize Zenoh session
        cfg = ZenohConfig(host="192.168.4.1", port=7447)
        if not ZenohNode.init(cfg):
            return

        # 2. Initialize ROS 2 client and TF node
        log.info("Initializing ROS 2 Client for MCU IMU TF Broadcaster...")
        rclpy.init(args=args)

        ros2_node = IMUTFBroadcasterNode()
        _zenoh_node = MPU6050TFSubscriberNode(ros2_node)

        rclpy.spin(ros2_node)

    except KeyboardInterrupt:
        log.warn("[CTRL+C]>>> Interrupted by the User.")

    except Exception as e:
        log.error(f"Critical Error: {e}")

    finally:
        if _zenoh_node is not None:
            log.info("Shutting down Zenoh Node...")
            ZenohNode.shutdown()
            _zenoh_node = None

        if ros2_node is not None:
            log.info("Destroying ROS 2 TF Broadcaster Node...")
            ros2_node.destroy_node()
            ros2_node = None

        if rclpy.ok():
            log.info("Shutting down ROS 2 Client...")
            rclpy.shutdown()


if __name__ == "__main__":
    main()
