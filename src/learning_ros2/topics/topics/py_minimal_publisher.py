#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from ros2_interfaces.msg import String

from rclpy.qos import qos_profile_sensor_data

class Counter_publisher_node_class(Node):
    def __init__(self) -> None:
        super().__init__("counter_publisher")
        self.get_logger().info(f"{self.get_name()} has been started!")
        self.cnt_ = 0
        # self.publisher_ = self.create_publisher(String, "/counter", 10)
        self.publisher_ = self.create_publisher(String, "/counter", qos_profile_sensor_data) # Optimized for speed over reliability
        self.timer_ = self.create_timer(0.5, self.callback_timer)   #publishes every 0.5s

    def callback_timer(self):
        msg = String()
        msg.data = f"Counter: {self.cnt_}"
        self.publisher_.publish(msg)
        self.get_logger().info(msg.data)
        self.cnt_ += 1

def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = Counter_publisher_node_class()
        rclpy.spin(node_instance)

    except KeyboardInterrupt:
        log.warn("[CTRL+C]>>> Interrupted by the User.")

    except Exception as e:
        log.error(f"Critical Error: {e}")
    
    finally:
        if node_instance is not None:
            log.info("Destroying the ROS2 Node...")
            node_instance.destroy_node()
            node_instance = None

        if rclpy.ok():
            log.info("Manually shutting down the ROS2 Client...")
            rclpy.shutdown()

if __name__ == '__main__':
    main()