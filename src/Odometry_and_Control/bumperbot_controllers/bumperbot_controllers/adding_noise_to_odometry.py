#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from rcl_interfaces.msg import ParameterDescriptor
from rclpy.time import Time
import numpy as np
import math

# ROS 2 Message Interfaces
from std_msgs.msg import Float64MultiArray
from geometry_msgs.msg import Twist, TransformStamped
from sensor_msgs.msg import JointState
from nav_msgs.msg import Odometry
from tf2_ros import TransformBroadcaster

# Spatial transformations library
from tf_transformations import quaternion_from_euler

class adding_noise_to_odometry_class(Node):
    """
    Subscribes to perfect simulated odometry and republishes it with added Gaussian noise.
    
    In a simulation environment, sensor data (like odometry) is typically perfect. 
    However, real-world robots experience external interferences such as electrical 
    and magnetic noise, wheel slippage, and mechanical drift. 
    
    This node simulates these real-world imperfections by injecting artificial noise 
    into the clean odometry data. This provides an almost realistic, synthetic noisy dataset 
    that will be used later to demonstrate how an Extended Kalman Filter (EKF) can fuse 
    and correct noisy sensor readings to estimate the robot's true state.
    """

    def __init__(self) -> None:
        super().__init__("adding_noise_to_odometry")
        self.get_logger().info(f"{self.get_name()} has been initialized")
        desc = ParameterDescriptor(dynamic_typing=True)
        self.declare_parameter('wheel_radius', descriptor=desc)
        self.declare_parameter('wheel_separation', descriptor=desc)
        self.declare_parameter('noise_factor', descriptor=desc)
        
        self.wheel_radius = self.get_parameter('wheel_radius').value
        self.wheel_separation = self.get_parameter('wheel_separation').value
        self.noise_factor = self.get_parameter('noise_factor').value

        if any(p is None for p in [self.wheel_radius, self.wheel_separation, self.noise_factor]):
            error_msg = "A required parameter was not set! Please load a config file."
            raise RuntimeError(error_msg)
            
        self.get_logger().info("Successfully loaded parameters from config file:")
        self.get_logger().info(f"-> wheel_radius: {self.wheel_radius:.3f}m")
        self.get_logger().info(f"-> wheel_separation: {self.wheel_separation:.3f}m")
        self.get_logger().info(f"-> noise_factor: {self.noise_factor:.4f}")

        self.joint_state_subscriber_ = self.create_subscription(JointState, "/joint_states", self.callback_joint_states, 10)   

        # Initialize Odometry publisher and TF Broadcaster
        self.odom_publisher_ = self.create_publisher(Odometry, "/odom_noisy", 10)
        self.tf_broadcaster_ = TransformBroadcaster(self)

        # Initialize robot odometry pose state
        self.x = 0.0
        self.y = 0.0
        self.theta = 0.0

        # Initialize wheel position tracking variables
        self.last_left_wheel_pos_ = None
        self.last_right_wheel_pos_ = None
        self.prev_time_ = None
        
    def callback_joint_states(self, msg: JointState) -> None:
        current_time = Time.from_msg(msg.header.stamp)
        
        # 1. Zip names and positions together into a dynamic lookup dictionary
        joint_position_map = dict(zip(msg.name, msg.position))
        
        # Safely pull the data out by string name, defaulting to 0.0 if not found
        left_wheel_position = joint_position_map.get("wheel_left_joint", 0.0) + np.random.normal(0, self.noise_factor)
        right_wheel_position = joint_position_map.get("wheel_right_joint", 0.0) + np.random.normal(0, self.noise_factor)
        
        # 2. First-run guard initialization
        if self.last_left_wheel_pos_ is None or self.last_right_wheel_pos_ is None or self.prev_time_ is None:
            self.last_left_wheel_pos_ = joint_position_map.get("wheel_left_joint", 0.0)
            self.last_right_wheel_pos_ = joint_position_map.get("wheel_right_joint", 0.0)
            self.prev_time_ = current_time
            return

        # 3. Dynamic time step safety filter
        dt = (current_time - self.prev_time_).nanoseconds * 1e-9
        if dt <= 0.0:
            return
            
        self.prev_time_ = current_time

        # 4. Compute wheel angular velocities (rad/s) using discrete time derivative
        left_wheel_vel = (left_wheel_position - self.last_left_wheel_pos_) / dt
        right_wheel_vel = (right_wheel_position - self.last_right_wheel_pos_) / dt

        # Update state variables for the next iteration
        self.last_left_wheel_pos_ = joint_position_map.get("wheel_left_joint", 0.0)
        self.last_right_wheel_pos_ = joint_position_map.get("wheel_right_joint", 0.0)

        # 5. Transform wheel speeds to global map frame velocities using your matrix function
        x_dot, y_dot, theta_dot = self.forward_kinematics(left_wheel_vel, right_wheel_vel)

        # 6. Accumulate local step changes into the global pose coordinates
        self.x += x_dot * dt
        self.y += y_dot * dt
        self.theta += theta_dot * dt

        # Keep global orientation angle normalized securely between -pi and +pi
        self.theta = np.arctan2(np.sin(self.theta), np.cos(self.theta))
        
       # 7. Compute local robot linear velocity for the Odometry message twist field
        v_linear_local = (self.wheel_radius / 2.0) * (left_wheel_vel + right_wheel_vel)
        v_angular_local = theta_dot  # In differential drive, local turning rate matches global turning rate

        # 8. Trigger your custom publication and broadcast pipeline
        self.publish_odom_tf(v_linear_local, v_angular_local, msg.header.stamp)
        
    def forward_kinematics(self, left_wheel_vel, right_wheel_vel):
        """
        Computes global robot velocities (x_dot, y_dot, theta_dot)
        from individual wheel angular velocities (rad/s) using the Forward Jacobian matrix.
        """
        # 1. Define the 3x2 Forward Kinematics Matrix
        jacobian_matrix = np.array([
            [(self.wheel_radius / 2.0) * np.cos(self.theta),  (self.wheel_radius / 2.0) * np.cos(self.theta)],
            [(self.wheel_radius / 2.0) * np.sin(self.theta),  (self.wheel_radius / 2.0) * np.sin(self.theta)],
            [-self.wheel_radius / self.wheel_separation,       self.wheel_radius / self.wheel_separation]
        ])

        # 2. Pack your wheel angular velocities (rad/s) into a 1D input vector
        wheel_velocities = np.array([left_wheel_vel, right_wheel_vel])

        # 3. Perform matrix multiplication: [x_dot, y_dot, theta_dot] = Jacobian @ Wheel_Velocities
        robot_velocities = jacobian_matrix @ wheel_velocities

        # 4. Unpack the resulting vector into explicit, readable variables
        x_dot, y_dot, theta_dot = robot_velocities

        return [x_dot, y_dot, theta_dot]

    def publish_odom_tf(self, v_linear: float, v_angular: float, sim_time) -> None:
        # Use tf_transformations to generate [x, y, z, w] array from Roll, Pitch, Yaw
        q = quaternion_from_euler(0.0, 0.0, self.theta)

        # 1. Build and Publish the Odometry message over /odom topic
        odom_msg = Odometry()
        odom_msg.header.stamp = sim_time  
        odom_msg.header.frame_id = "odom"
        odom_msg.child_frame_id = "base_footprint_ekf" 

        # Set running position updates
        odom_msg.pose.pose.position.x = self.x
        odom_msg.pose.pose.position.y = self.y
        odom_msg.pose.pose.position.z = 0.0
        odom_msg.pose.pose.orientation.x = q[0]
        odom_msg.pose.pose.orientation.y = q[1]
        odom_msg.pose.pose.orientation.z = q[2]
        odom_msg.pose.pose.orientation.w = q[3]

        # Set linear and angular velocities
        odom_msg.twist.twist.linear.x = v_linear
        odom_msg.twist.twist.angular.z = v_angular

        self.odom_publisher_.publish(odom_msg)

        # 2. Build and Broadcast the dynamic transform link over /tf
        transform = TransformStamped()
        transform.header.stamp = sim_time  
        transform.header.frame_id = "odom"
        transform.child_frame_id = "base_footprint_noisy"

        transform.transform.translation.x = self.x
        transform.transform.translation.y = self.y
        transform.transform.translation.z = 0.0
        transform.transform.rotation.x = q[0]
        transform.transform.rotation.y = q[1]
        transform.transform.rotation.z = q[2]
        transform.transform.rotation.w = q[3]

        self.tf_broadcaster_.sendTransform(transform)

def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = adding_noise_to_odometry_class() 
        
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
