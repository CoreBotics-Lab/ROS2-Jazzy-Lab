#!/usr/bin/env python3
import sys
import os

# Resolve symlinks first so this works both when run directly and when
# installed via `colcon build --symlink-install` (where __file__ is a
# symlink inside /install/lib/ that points back to the source tree).
_this_dir = os.path.dirname(os.path.realpath(__file__))
sys.path.insert(0, os.path.abspath(os.path.join(_this_dir, '../../../shared_libraries/python')))

from zenoh_ros import ZenohNode, ZenohConfig
from zenoh_ros.sensor_msgs import z_Imu

class MPU6050SubscriberNode(ZenohNode):
    def __init__(self) -> None:
        super().__init__("mpu6050_subscriber")
        self.get_logger().info("Node has been started")

        # Create subscription to topic "robot/mpu6050" with standard ROS 2 z_Imu type
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


def main() -> None:
    # Connect to ESP32-S3 SoftAP IP (192.168.4.1)
    cfg = ZenohConfig(host="192.168.4.1", port=7447)
    if not ZenohNode.init(cfg):
        return
    node_instance = None
    try:
        node_instance = MPU6050SubscriberNode()
        node_instance.z_spin()

    except KeyboardInterrupt:
        if node_instance:
            node_instance.get_logger().info("Shutdown requested via KeyboardInterrupt.")

    except Exception as e:
        if node_instance:
            node_instance.get_logger().error(f"Critical Error: {e}")

    finally:
        ZenohNode.shutdown()



if __name__ == "__main__":
    main()
