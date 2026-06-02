#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from std_msgs.msg import Float64MultiArray

class Simple_speed_controller_class(Node):
    def __init__(self) -> None:
        super().__init__("simple_speed_controller")
        self.publisher_ = self.create_publisher(Float64MultiArray, "/velocity_controller/commands", 10)
        self.timer_ = self.create_timer(0.1, self.callback_timer)
        
    def callback_timer(self) -> None:
        msg = Float64MultiArray()
        msg.data = [-1.0, 1.0]
        self.publisher_.publish(msg)

def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = Simple_speed_controller_class() 
        
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
