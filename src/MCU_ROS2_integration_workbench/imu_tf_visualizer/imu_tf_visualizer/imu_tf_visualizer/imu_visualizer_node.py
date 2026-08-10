#!/usr/bin/env python3
import sys
import os

# Resolve symlinks first so this works both when run directly and when
# installed via `colcon build --symlink-install` (where __file__ is a
# symlink inside /install/lib/ that points back to the source tree).
_this_dir = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(0, os.path.abspath(os.path.join(_this_dir, '../../../shared_libraries/python')))

import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from sensor_msgs.msg import Imu

from zenoh_ros import ZenohNode, ZenohConfig
from zenoh_ros.sensor_msgs import z_Imu


# ─── ROS 2 Node ──────────────────────────────────────────────────────────────
class IMURepublisherNode(Node):
    """Pure rclpy Node — owns the native ROS 2 /imu/data publisher."""

    def __init__(self) -> None:
        super().__init__("imu_republisher")

        # Native ROS 2 publisher — makes IMU data available to the ROS 2 graph
        self.imu_publisher_ = self.create_publisher(Imu, "/imu/data", 10)
        self.get_logger().info("IMU republisher ready on /imu/data")

    def publish_imu(self, msg: z_Imu) -> None:
        """Republish a z_Imu message as a native sensor_msgs/Imu."""
        imu_msg = Imu()
        imu_msg.header.stamp.sec     = msg.header.stamp.sec
        imu_msg.header.stamp.nanosec = msg.header.stamp.nanosec
        imu_msg.header.frame_id = "imu_link"

        imu_msg.linear_acceleration.x = msg.linear_acceleration.x
        imu_msg.linear_acceleration.y = msg.linear_acceleration.y
        imu_msg.linear_acceleration.z = msg.linear_acceleration.z

        imu_msg.angular_velocity.x = msg.angular_velocity.x
        imu_msg.angular_velocity.y = msg.angular_velocity.y
        imu_msg.angular_velocity.z = msg.angular_velocity.z

        self.imu_publisher_.publish(imu_msg)


# ─── Zenoh Node ──────────────────────────────────────────────────────────────
class MPU6050SubscriberNode(ZenohNode):
    """ZenohNode — subscribes to IMU data from the ESP32-S3 MCU over Zenoh."""

    def __init__(self, ros2_node: IMURepublisherNode) -> None:
        super().__init__("mpu6050_subscriber")
        self._ros2_node = ros2_node
        self.get_logger().info("Node has been started")

        # Zenoh subscription — receives IMU data from the ESP32-S3 MCU
        self.sub = self.z_create_subscription(
            z_Imu,
            "robot/mpu6050",
            self.listener_callback,
            10
        )

    def listener_callback(self, msg: z_Imu) -> None:
        self.get_logger().info(
            f"[IMU RECV] [stamp: {msg.header.stamp.sec}.{msg.header.stamp.nanosec:09d}] "
            f"Accel: ({msg.linear_acceleration.x:6.2f}, {msg.linear_acceleration.y:6.2f}, {msg.linear_acceleration.z:6.2f}) m/s² | "
            f"Gyro: ({msg.angular_velocity.x:6.2f}, {msg.angular_velocity.y:6.2f}, {msg.angular_velocity.z:6.2f}) rad/s"
        )

        # Forward to the rclpy Node for native ROS 2 publishing
        self._ros2_node.publish_imu(msg)


def main(args=None) -> None:
    log = get_logger("System")
    ros2_node = None
    _zenoh_node = None

    try:
        # 1. Connect to ESP32-S3 SoftAP and initialize Zenoh session
        cfg = ZenohConfig(host="192.168.4.1", port=7447)
        if not ZenohNode.init(cfg):
            return

        # 2. Initialize the ROS 2 client and create the publisher node
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        ros2_node = IMURepublisherNode()

        # 3. Create the Zenoh subscriber node, injecting the rclpy node
        # Held in _zenoh_node to keep the subscription alive for the process lifetime
        _zenoh_node = MPU6050SubscriberNode(ros2_node)

        # 4. rclpy.spin keeps the process alive and drives the ROS 2 executor
        rclpy.spin(ros2_node)

    except KeyboardInterrupt:
        log.warn("[CTRL+C]>>> Interrupted by the User.")

    except Exception as e:
        log.error(f"Critical Error: {e}")

    finally:
        if _zenoh_node is not None:
            log.info("Shutting down the Zenoh Node...")
            ZenohNode.shutdown()
            _zenoh_node = None

        if ros2_node is not None:
            log.info("Destroying the ROS2 Node...")
            ros2_node.destroy_node()
            ros2_node = None

        if rclpy.ok():
            log.info("Manually shutting down the ROS2 Client...")
            rclpy.shutdown()


if __name__ == "__main__":
    main()
