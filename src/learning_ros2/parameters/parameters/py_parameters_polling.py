#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger

class ParameterPollingNode_Class(Node):
    """A node to demonstrate ROS 2 parameters using the polling method."""

    def __init__(self):
        super().__init__('parameter_polling_node')
        self.get_logger().info(f"{self.get_name()} has been started!")
        
        # 1. Declare Parameters with default values
        self.declare_parameter('robot_mode', 'autonomous')
        self.declare_parameter('max_velocity', 1.2)
        self.declare_parameter('publish_rate_ms', 500)
        
        # 2. Create a timer that will poll the parameters
        rate_ms = self.get_parameter('publish_rate_ms').get_parameter_value().integer_value
        self.timer = self.create_timer(rate_ms / 1000.0, self.timer_callback)
        self.get_logger().info(f"Node started. Polling parameters every {rate_ms} ms.")

    def timer_callback(self):
        """
        Timer callback that continuously polls (reads) the parameters.
        If a parameter is changed externally, this loop will pick up the
        new value on its next iteration.
        """
        # Poll the current values of the parameters in every loop
        current_mode = self.get_parameter('robot_mode').get_parameter_value().string_value
        current_velocity = self.get_parameter('max_velocity').get_parameter_value().double_value
        
        self.get_logger().info(f"Polling: Mode is [{current_mode}], Max Velocity is [{current_velocity:.2f}]")

def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = ParameterPollingNode_Class()
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
