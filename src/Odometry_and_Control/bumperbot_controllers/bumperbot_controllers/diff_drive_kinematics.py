#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from std_msgs.msg import Float64MultiArray
from geometry_msgs.msg import Twist
from rcl_interfaces.msg import ParameterDescriptor

class Diff_drive_kinematics_class(Node):
    def __init__(self) -> None:
        super().__init__("diff_drive_kinematics")

        desc = ParameterDescriptor(dynamic_typing=True)
        self.declare_parameter('wheel_radius', descriptor=desc)
        self.declare_parameter('wheel_separation', descriptor=desc)

        self.wheel_radius = self.get_parameter('wheel_radius').value
        self.wheel_separation = self.get_parameter('wheel_separation').value
        # If the config file is missing, the dynamically typed parameters will have a value of None.
        # We must check for this to enforce the "fail-fast" behavior.
        if any(p is None for p in [self.wheel_radius, self.wheel_separation]):
            error_msg = "A required parameter was not set! Please load a config file."
            # Raising an exception is a clean way to halt the node's execution.
            raise RuntimeError(error_msg)
            
        self.get_logger().info("Successfully loaded parameters from config file:")
        self.get_logger().info(f"-> wheel_radius: {self.wheel_radius:.3f}m")
        self.get_logger().info(f"-> wheel_separation: {self.wheel_separation:.3f}m")

        # self.wheel_radius = 0.068/2.0 # meters
        # self.wheel_separation = 0.174 # meters

        self.cmd_vel_subscriber_ = self.create_subscription(Twist, "/cmd_vel", self.callback_cmd_vel, 10)
        self.wheel_speed_publisher_ = self.create_publisher(Float64MultiArray, "/velocity_controller/commands", 10)   

    def callback_cmd_vel(self, msg: Twist) -> None:
        v_linear = msg.linear.x
        v_angular = msg.angular.z

        left_wheel_speed = (v_linear - (v_angular * self.wheel_separation / 2.0)) / self.wheel_radius
        right_wheel_speed = (v_linear + (v_angular * self.wheel_separation / 2.0)) / self.wheel_radius
        wheel_speed_msg = Float64MultiArray() 
        wheel_speed_msg.data = [left_wheel_speed, right_wheel_speed]
        self.wheel_speed_publisher_.publish(wheel_speed_msg)

def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = Diff_drive_kinematics_class() 
        
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
