#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from rcl_interfaces.msg import SetParametersResult
from rclpy.parameter import Parameter

class ParameterEventDrivenNode_Class(Node):
    """A node to demonstrate ROS 2 parameters using the event-driven method."""

    def __init__(self):
        super().__init__('parameter_event_driven_node')
        self.get_logger().info(f"{self.get_name()} has been started!")
        
        # 1. Declare Parameters (Mandatory in modern ROS 2)
        self.declare_parameter('robot_mode', 'autonomous')
        self.declare_parameter('max_velocity', 1.2)
        self.declare_parameter('publish_rate_ms', 500)
        
        # 2. Retrieve Initial Values
        # .value automatically returns the correct Python type (str, float, int)
        self.current_mode = self.get_parameter('robot_mode').value
        self.current_velocity = self.get_parameter('max_velocity').value
        
        self.get_logger().info(
            f"Node started with mode: '{self.current_mode}', max_velocity: {self.current_velocity:.2f}"
        )
        
        # 3. Set up Dynamic Parameter Handling
        # In rclpy, this is the standard way a node listens to its own parameter changes.
        self.add_on_set_parameters_callback(self.parameter_callback)
        
        # 4. Timer to demonstrate usage
        # Convert milliseconds to seconds for the create_timer function
        rate_ms = self.get_parameter('publish_rate_ms').get_parameter_value().integer_value
        self.timer = self.create_timer(rate_ms / 1000.0, self.timer_callback)

    def parameter_callback(self, params: list[Parameter]):
        """Callback triggered whenever ANY parameter is changed at runtime."""
        for param in params:
            if param.name == 'max_velocity' and param.type_ == Parameter.Type.DOUBLE:
                self.current_velocity = param.value
                self.get_logger().info(f"max_velocity changed dynamically to: {self.current_velocity:.2f}")
            elif param.name == 'robot_mode' and param.type_ == Parameter.Type.STRING:
                self.get_logger().info(f"robot_mode changed dynamically to: {param.value}")
                self.current_mode = param.value
            elif param.name == 'publish_rate_ms' and param.type_ == Parameter.Type.INTEGER:
                self.get_logger().info(f"publish_rate_ms changed dynamically. Updating timer to {param.value} ms.")
                self.timer.cancel()
                rate_ms = param.get_parameter_value().integer_value
                self.timer = self.create_timer(rate_ms / 1000.0, self.timer_callback)
                
        # We must return a SetParametersResult to tell ROS 2 the change was accepted
        return SetParametersResult(successful=True)

    def timer_callback(self):
        """Timer callback using the cached event-driven parameter."""
        self.get_logger().info(f"Event-Driven: Mode is [{self.current_mode}], Max Velocity is [{self.current_velocity:.2f}]")

def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = ParameterEventDrivenNode_Class()
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