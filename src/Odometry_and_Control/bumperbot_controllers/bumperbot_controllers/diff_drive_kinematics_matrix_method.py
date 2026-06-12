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
        self.joint_state_subscriber_ = self.create_subscription(JointState, "/joint_states", self.callback_joint_states, 10)   

        # Initialize Odometry publisher and TF Broadcaster
        self.odom_publisher_ = self.create_publisher(Odometry, "/odom", 10)
        self.tf_broadcaster_ = TransformBroadcaster(self)

        # Initialize robot odometry pose state
        self.x = 0.0
        self.y = 0.0
        self.theta = 0.0

        # Initialize wheel position tracking variables
        self.last_left_wheel_pos_ = None
        self.last_right_wheel_pos_ = None
        self.prev_time_ = None

    def callback_cmd_vel(self, msg: Twist) -> None:
        Vb = msg.linear.x
        Omegab = msg.angular.z
        wheel_velocities = self.compute_wheel_velocities(Vb, Omegab)
        wheel_velocities_msg = Float64MultiArray()
        wheel_velocities_msg.data = wheel_velocities
        self.wheel_speed_publisher_.publish(wheel_velocities_msg)
    
    def callback_joint_states(self, msg: JointState) -> None:
        current_time = Time.from_msg(msg.header.stamp)
        
        # 1. Zip names and positions together into a dynamic lookup dictionary
        joint_position_map = dict(zip(msg.name, msg.position))
        
        # Safely pull the data out by string name, defaulting to 0.0 if not found
        left_wheel_position = joint_position_map.get("wheel_left_joint", 0.0)
        right_wheel_position = joint_position_map.get("wheel_right_joint", 0.0)
        
        # 2. First-run guard initialization
        if self.last_left_wheel_pos_ is None or self.last_right_wheel_pos_ is None or self.prev_time_ is None:
            self.last_left_wheel_pos_ = left_wheel_position
            self.last_right_wheel_pos_ = right_wheel_position
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
        self.last_left_wheel_pos_ = left_wheel_position
        self.last_right_wheel_pos_ = right_wheel_position

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

    def publish_odom_tf(self, v_linear: float, v_angular: float, sim_time) -> None:
        # Use tf_transformations to generate [x, y, z, w] array from Roll, Pitch, Yaw
        q = quaternion_from_euler(0.0, 0.0, self.theta)

        # 1. Build and Publish the Odometry message over /odom topic
        odom_msg = Odometry()
        odom_msg.header.stamp = sim_time  
        odom_msg.header.frame_id = "odom"
        odom_msg.child_frame_id = "base_footprint" 

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
        transform.child_frame_id = "base_footprint"

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
