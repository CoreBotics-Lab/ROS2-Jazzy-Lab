#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from rcl_interfaces.msg import ParameterDescriptor
from rclpy.time import Time
import numpy as np

# ROS 2 Message Interfaces
from std_msgs.msg import Float64MultiArray
from geometry_msgs.msg import Twist, TransformStamped
from sensor_msgs.msg import JointState
from nav_msgs.msg import Odometry
from tf2_ros import TransformBroadcaster

# Spatial transformations library
from tf_transformations import quaternion_from_euler


class Diff_drive_kinematics_matrix_method_class(Node):

    def __init__(self) -> None:
        super().__init__("diff_drive_kinematics")
        desc = ParameterDescriptor(dynamic_typing=True)
        self.declare_parameter('wheel_radius', descriptor=desc)
        self.declare_parameter('wheel_separation', descriptor=desc)

        self.wheel_radius = self.get_parameter('wheel_radius').value
        self.wheel_separation = self.get_parameter('wheel_separation').value

        if any(p is None for p in [self.wheel_radius, self.wheel_separation]):
            error_msg = "A required parameter was not set! Please load a config file."
            raise RuntimeError(error_msg)
            
        self.get_logger().info("Successfully loaded parameters from config file:")
        self.get_logger().info(f"-> wheel_radius: {self.wheel_radius:.3f}m")
        self.get_logger().info(f"-> wheel_separation: {self.wheel_separation:.3f}m")

        # 1. Pre-compute the Forward Kinematics Matrix (M) ONCE at startup
        self.M = np.array([
            [1/2, 1/2],
            [-1/self.wheel_separation, 1/self.wheel_separation]
        ])

        # 2. Pre-invert it ONCE and save it to the object cache
        self.M_inv = np.linalg.inv(self.M)

        self.cmd_vel_subscriber_ = self.create_subscription(Twist, "/cmd_vel", self.callback_cmd_vel, 10)
        self.wheel_speed_publisher_ = self.create_publisher(Float64MultiArray, "/simple_velocity_controller/commands", 10)

    def callback_cmd_vel(self, msg: Twist) -> None:
        Vb = msg.linear.x
        Omegab = msg.angular.z
        wheel_velocities = self.compute_wheel_velocities(Vb, Omegab)
        wheel_velocities_msg = Float64MultiArray()
        wheel_velocities_msg.data = wheel_velocities
        self.wheel_speed_publisher_.publish(wheel_velocities_msg)
        
    def compute_wheel_velocities(self, Vb, Omegab):
        """
        Computes wheel angular velocities (rad/s) using the cached inverse matrix.
        """
        velocities = np.array([Vb, Omegab])
        
        # 1. Compute linear wheel velocities (m/s) via matrix multiplication
        linear_wheel_velocities = self.M_inv @ velocities
        
        # 2. Convert to angular velocities (rad/s) using element-wise division
        angular_wheel_velocities = linear_wheel_velocities / self.wheel_radius
        
        # 3. Convert back to a native Python list so it assigns perfectly to Float64MultiArray
        return angular_wheel_velocities.tolist()

        
        

        

def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = Diff_drive_kinematics_matrix_method_class() 
        
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
