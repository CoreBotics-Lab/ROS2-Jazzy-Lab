#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from std_msgs.msg import Float64MultiArray
from geometry_msgs.msg import Twist

class Diff_drive_kinematics_class(Node):
    def __init__(self) -> None:
        super().__init__("diff_drive_kinematics")
        self.wheel_radius = 0.068/2.0 # meters
        self.wheel_seperation = 0.17454725 # meters
        self.cmd_vel_subscriber_ = self.create_subscription(Twist, "/cmd_vel", self.callback_cmd_vel, 10)
        self.wheel_speed_publisher_ = self.create_publisher(Float64MultiArray, "/velocity_controller/commands", 10)   

    def callback_cmd_vel(self, msg: Twist) -> None:
        v_linear = msg.linear.x
        v_angular = msg.angular.z

        left_wheel_speed = (v_linear - (v_angular * self.wheel_seperation / 2.0)) / self.wheel_radius
        right_wheel_speed = (v_linear + (v_angular * self.wheel_seperation / 2.0)) / self.wheel_radius
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
