#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from ros2_interfaces.msg import String

class Counter_subscriber_node_class(Node):
    def __init__(self) -> None:
        super().__init__("counter_subscriber")
        self.get_logger().info(f"{self.get_name()} has been started!")
        self.create_subscription(String, "/counter", self.callback_subscriber, 10)

    def callback_subscriber(self, msg: String):
        self.get_logger().info(f"{msg.data}")


def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = Counter_subscriber_node_class()
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