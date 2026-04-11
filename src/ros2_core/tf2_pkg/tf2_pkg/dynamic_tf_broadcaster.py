#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger

from geometry_msgs.msg import TransformStamped
from tf2_ros.transform_broadcaster import TransformBroadcaster
from tf_transformations import quaternion_from_euler
import math

class DynamicTFBroadcasterNode(Node):
    """
    This node broadcasts a dynamic transform between two frames.
    Dynamic transforms are for relationships that change over time.
    It publishes repeatedly to the /tf topic via a timer loop.
    """
    def __init__(self):
        super().__init__('dynamic_tf_broadcaster_node')
        self.get_logger().info(f"{self.get_name()} has been started!")

        # 1. Initialize the dynamic broadcaster. 
        # Notice it is TransformBroadcaster, NOT StaticTransformBroadcaster
        self.tf_broadcaster = TransformBroadcaster(self)

        # 2. Create a timer to periodically publish the transform (e.g., 30 Hz, 30 times a second)
        timer_period = 1.0 / 30.0
        self.timer = self.create_timer(timer_period, self.timer_callback)

    def timer_callback(self):
        """Calculates and publishes the moving transform."""
        t = TransformStamped()

        # --- Fill in the message ---
        t.header.stamp = self.get_clock().now().to_msg()
        t.header.frame_id = 'world'
        t.child_frame_id = 'my_dynamic_frame'

        # Get the current time in seconds to use for our math functions
        time_now = self.get_clock().now().nanoseconds / 1e9

        # Define the dynamic translation (oscillating in a 2-meter circle)
        t.transform.translation.x = 2.0 * math.sin(time_now)
        t.transform.translation.y = 2.0 * math.cos(time_now)
        t.transform.translation.z = 0.5

        # Define the dynamic rotation (spinning around the Z axis)
        q = quaternion_from_euler(0.0, 0.0, time_now) # yaw increases over time
        t.transform.rotation.x = q[0]
        t.transform.rotation.y = q[1]
        t.transform.rotation.z = q[2]
        t.transform.rotation.w = q[3]

        # The broadcaster sends the transform out over the /tf topic.
        self.tf_broadcaster.sendTransform(t)

def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = DynamicTFBroadcasterNode()
        
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