#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger

from geometry_msgs.msg import TransformStamped
from tf2_ros.static_transform_broadcaster import StaticTransformBroadcaster
from tf_transformations import quaternion_from_euler
import math

class StaticTFBroadcasterNode(Node):
    """
    This node broadcasts a static transform between two frames.
    Static transforms are for relationships that do not change over time,
    like the position of a camera relative to the robot's base.
    It publishes once to the /tf_static topic.
    """
    def __init__(self):
        super().__init__('static_tf_broadcaster_node')
        self.get_logger().info(f"{self.get_name()} has been started!")

        # 1. Initialize the broadcaster. This is a special tool from the tf2_ros
        # library that is designed to publish to the /tf_static topic.
        self.tf_static_broadcaster = StaticTransformBroadcaster(self)

        # 2. Create and publish the static transform.
        self.publish_static_transform()

    def publish_static_transform(self):
        """Creates and publishes the static transform."""
        t = TransformStamped()

        # --- Fill in the message ---
        t.header.stamp = self.get_clock().now().to_msg()
        t.header.frame_id = 'world'
        t.child_frame_id = 'my_static_frame'

        # Define the translation (position) of the child frame relative to the parent
        t.transform.translation.x = 1.0
        t.transform.translation.y = 2.0
        t.transform.translation.z = 0.5

        # Define the rotation (as a quaternion)
        # For this example, a 45-degree rotation around the Z-axis
        q = quaternion_from_euler(math.pi / 2, 0, 0) # (roll, pitch, yaw)
        t.transform.rotation.x = q[0]
        t.transform.rotation.y = q[1]
        t.transform.rotation.z = q[2]
        t.transform.rotation.w = q[3]

        # The broadcaster sends the transform out over the /tf_static topic.
        # It only needs to be sent once.
        self.tf_static_broadcaster.sendTransform(t)
        self.get_logger().info(
            f"Broadcasting static transform from '{t.header.frame_id}' to '{t.child_frame_id}'"
        )

def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = StaticTFBroadcasterNode()
        
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
