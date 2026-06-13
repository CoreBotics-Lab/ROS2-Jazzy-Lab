#!/usr/bin/env python3

import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from nav_msgs.msg import Odometry, Path
from geometry_msgs.msg import PoseStamped

class TrajectoryPublisher(Node):
    def __init__(self) -> None:
        super().__init__("TrajectoryPublisher")
        self.trajectory_publisher_ = self.create_publisher(Path, "/trajectory", 10)
        self.odom_subscriber_ = self.create_subscription(Odometry, "/odom", self.odom_callback, 10)
        self.trajectory_msg = Path()
        self.trajectory_msg.header.frame_id = "odom"

    def odom_callback(self, msg: Odometry):
        self.trajectory_msg.header.stamp = msg.header.stamp
        waypoint = PoseStamped()

        if len(self.trajectory_msg.poses) > 1000: 
            self.trajectory_msg.poses.pop(0)

        waypoint.header.stamp = msg.header.stamp
        waypoint.header.frame_id = "odom"
        waypoint.pose = msg.pose.pose
        self.trajectory_msg.poses.append(waypoint)
        self.trajectory_publisher_.publish(self.trajectory_msg)

def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = TrajectoryPublisher() 
        
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
