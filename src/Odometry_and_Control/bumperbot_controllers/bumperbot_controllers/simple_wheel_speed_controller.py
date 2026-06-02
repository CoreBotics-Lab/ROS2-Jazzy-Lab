#!/usr/bin/env python3

import sys
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from std_msgs.msg import Float64MultiArray

class Simple_wheel_speed_controller_class(Node):
    def __init__(self, left_wheel_speed: float, right_wheel_speed: float) -> None:
        super().__init__("simple_wheel_speed_controller")
        self.left_wheel_speed = left_wheel_speed
        self.right_wheel_speed = right_wheel_speed
        self.publisher_ = self.create_publisher(Float64MultiArray, "/velocity_controller/commands", 10)
        self.timer_ = self.create_timer(0.1, self.callback_timer)
        
    def callback_timer(self) -> None:
        msg = Float64MultiArray()
        msg.data = [self.left_wheel_speed, self.right_wheel_speed]
        self.publisher_.publish(msg)

def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    # Parse wheel speeds from the terminal (sys.argv)
    left_wheel_speed = 0.0
    right_wheel_speed = 0.0
    if len(sys.argv) >= 3:
        try:
            left_wheel_speed = float(sys.argv[1])
            right_wheel_speed = float(sys.argv[2])
        except ValueError:
            log.warn("Invalid speed arguments provided. Using 0.0 for both wheels.")

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info(f"Starting a ROS2 Node with speeds: Left={left_wheel_speed}, Right={right_wheel_speed}...")
        node_instance = Simple_wheel_speed_controller_class(left_wheel_speed, right_wheel_speed) 
        
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

if __name__ == "__main__":
    main()
