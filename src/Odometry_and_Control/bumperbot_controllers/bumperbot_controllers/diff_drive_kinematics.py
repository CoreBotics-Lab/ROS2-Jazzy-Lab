#!/usr/bin/env python3

from numpy import double
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from std_msgs.msg import Float64MultiArray
from geometry_msgs.msg import Twist
from rcl_interfaces.msg import ParameterDescriptor
from sensor_msgs.msg import JointState
from rclpy.time import Time
import math

from nav_msgs.msg import Odometry
from tf2_ros import TransformBroadcaster
from geometry_msgs.msg import TransformStamped

# Import the standard transformations library
from tf_transformations import quaternion_from_euler


class Diff_drive_kinematics_class(Node):

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

        self.last_left_wheel_pos_ = None
        self.last_right_wheel_pos_ = None
        self.prev_time_ = None

        # Initialize robot odometry pose state
        self.x = 0.0
        self.y = 0.0
        self.theta = 0.0

        self.cmd_vel_subscriber_ = self.create_subscription(Twist, "/cmd_vel", self.callback_cmd_vel, 10)
        self.wheel_speed_publisher_ = self.create_publisher(Float64MultiArray, "/simple_velocity_controller/commands", 10)
        self.joint_state_subscriber_ = self.create_subscription(JointState, "/joint_states", self.callback_joint_states, 10)   

        # Initialize Odometry publisher and TF Broadcaster
        self.odom_publisher_ = self.create_publisher(Odometry, "/odom", 10)
        self.tf_broadcaster_ = TransformBroadcaster(self)

    def callback_joint_states(self, msg: JointState) -> None:
        current_time = Time.from_msg(msg.header.stamp)
        
        # 1. Zip names and positions together into a dynamic lookup dictionary
        joint_position_map = dict(zip(msg.name, msg.position))
        
        # Safely pull the data out by string name, defaulting to 0.0 if not found
        left_wheel_position = joint_position_map.get("wheel_left_joint", 0.0)
        right_wheel_position = joint_position_map.get("wheel_right_joint", 0.0)
        
        # 2. First-run guard initialization
        if self.last_left_wheel_pos_ is None or self.last_right_wheel_pos_ is None:
            self.last_left_wheel_pos_ = left_wheel_position
            self.last_right_wheel_pos_ = right_wheel_position
            self.prev_time_ = current_time
            return

        # 3. Dynamic time step safety filter
        dt = (current_time - self.prev_time_).nanoseconds * 1e-9
        if dt <= 0.0:
            return
            
        self.prev_time_ = current_time

        # 4. Calculate the raw change in wheel positions (radians)
        delta_left_wheel_pos = left_wheel_position - self.last_left_wheel_pos_
        delta_right_wheel_pos = right_wheel_position - self.last_right_wheel_pos_

        # Update historical cache tracking variables
        self.last_left_wheel_pos_ = left_wheel_position
        self.last_right_wheel_pos_ = right_wheel_position

        # 5. Calculate linear travel distance increments directly
        distance_left = self.wheel_radius * delta_left_wheel_pos
        distance_right = self.wheel_radius * delta_right_wheel_pos

        # 6. Extract exact chassis spatial deltas directly (No dt dependencies yet)
        delta_distance = (distance_left + distance_right) / 2.0
        delta_angle = (distance_right - distance_left) / self.wheel_separation

        # 7. Mid-Angle Integration (Eliminates tracking drift over curves)
        mid_theta = self.theta + (delta_angle / 2.0)
        self.x += delta_distance * math.cos(mid_theta)
        self.y += delta_distance * math.sin(mid_theta)
        self.theta += delta_angle

        # 8. Angle Normalization: Bound heading angle explicitly between [-pi, pi]
        self.theta = math.atan2(math.sin(self.theta), math.cos(self.theta))

        # 9. Calculate velocities cleanly for your downstream /odom Twist messages
        v_linear = delta_distance / dt
        v_angular = delta_angle / dt

        # 10. Publish Odom Pose
        self.publish_odom_tf(v_linear, v_angular, msg.header.stamp)

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
