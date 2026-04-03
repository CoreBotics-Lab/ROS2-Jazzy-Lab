#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from rcl_interfaces.msg import ParameterDescriptor

class ParameterFromConfigNode(Node):
    """
    A minimal node to demonstrate loading initial parameter values from a YAML file.
    This node intentionally does not set default values when declaring parameters,
    forcing them to be provided by an external source at startup.
    """

    def __init__(self):
        super().__init__('parameter_from_config_node')
        self.get_logger().info(f"{self.get_name()} has been started!")

        # 1. Declare parameters WITHOUT default values.
        # This makes the node dependent on an external configuration file and
        # satisfies the modern ROS 2 API. By setting dynamic_typing=True, we
        # resolve the conflict between declaring a type and not providing a
        # default value, telling the system the type will be set at runtime.
        self.declare_parameter('robot_mode', descriptor=ParameterDescriptor(dynamic_typing=True))
        self.declare_parameter('max_velocity', descriptor=ParameterDescriptor(dynamic_typing=True))
        self.declare_parameter('publish_rate_ms', descriptor=ParameterDescriptor(dynamic_typing=True))

        # 2. Retrieve the values.
        mode = self.get_parameter('robot_mode').value
        velocity = self.get_parameter('max_velocity').value
        rate = self.get_parameter('publish_rate_ms').value

        # 3. Validate that parameters were loaded.
        # If the config file is missing, the dynamically typed parameters will have a value of None.
        # We must check for this to enforce the "fail-fast" behavior.
        if any(p is None for p in [mode, velocity, rate]):
            error_msg = "A required parameter was not set! Please load a config file."
            # Raising an exception is a clean way to halt the node's execution.
            raise RuntimeError(error_msg)

        self.get_logger().info("Successfully loaded parameters from config file:")
        self.get_logger().info(f"-> robot_mode: {mode}")
        self.get_logger().info(f"-> max_velocity: {velocity}")
        self.get_logger().info(f"-> publish_rate_ms: {rate} ms")

def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = ParameterFromConfigNode()
        # This node does its work in the constructor and doesn't need to spin to be useful,
        # but we spin to keep it alive for introspection with `ros2 param list`.
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