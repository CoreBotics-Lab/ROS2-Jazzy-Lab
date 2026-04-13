#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from rclpy.time import Time

from geometry_msgs.msg import PoseStamped
from visualization_msgs.msg import Marker
from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener
from tf2_ros import TransformException  # type: ignore

# Required import: Registers the math operations for geometry_msgs
# to allow the Buffer to transform PoseStamped messages.
import tf2_geometry_msgs  # type: ignore

class TF2DataTransformerNode(Node):
    """
    This node simulates a sensor detecting an object. It takes the coordinates
    of the object measured from 'my_dynamic_frame' and transforms them to be 
    relative to the 'world' frame.
    """
    def __init__(self):
        super().__init__('tf2_data_transformer_node')
        self.get_logger().info(f"{self.get_name()} has been started!")

        # 1. Create the Buffer and Listener (Pass the node to track use_sim_time)
        self.tf_buffer = Buffer(node=self)
        self.tf_listener = TransformListener(self.tf_buffer, self)
        
        # Create a publisher to visualize the detected object in RViz
        self.marker_pub = self.create_publisher(Marker, 'detected_object', 10)

        # 2. Create a timer to perform the transformation at 10 Hz (every 0.1 seconds)
        self.timer = self.create_timer(0.1, self.timer_callback)

    def timer_callback(self):
        # --- Simulating Sensor Data ---
        # Simulating an object detected exactly 1 meter in front of the camera.
        sensor_data = PoseStamped()
        
        # Specify the coordinate frame the data was measured in.
        sensor_data.header.frame_id = 'my_dynamic_frame'
        
        # Use Time().to_msg() (equivalent to Time 0) to request the most recent
        # available transform, avoiding extrapolation errors in a single-threaded node.
        sensor_data.header.stamp = Time().to_msg() 
        
        # The coordinates of the object relative to the sensor frame.
        sensor_data.pose.position.x = 1.0
        sensor_data.pose.position.y = 0.0
        sensor_data.pose.position.z = 0.0
        sensor_data.pose.orientation.w = 1.0

        try:
            # --- Core Transformation ---
            # Transform the sensor_data into the global 'world' frame.
            world_pose = self.tf_buffer.transform(
                sensor_data, 
                'world'
            )

            # Extract and print the new coordinates
            x = world_pose.pose.position.x
            y = world_pose.pose.position.y
            z = world_pose.pose.position.z
            
            self.get_logger().info(f"Object in World Frame -> X: {x:.2f}, Y: {y:.2f}, Z: {z:.2f}")

            # --- RViz Visualization ---
            # Publish the marker using the transformed global coordinates.
            self.publish_marker(world_pose.pose)

        except TransformException as ex:
            self.get_logger().warn(f"Could not transform data: {ex}")

    def publish_marker(self, pose):
        """Helper function to create and publish the visualization marker."""
        marker = Marker()
        marker.header.frame_id = 'world'
        marker.header.stamp = self.get_clock().now().to_msg()
        marker.ns = 'sensor_data'
        marker.id = 0
        marker.type = Marker.SPHERE
        marker.action = Marker.ADD
        marker.pose = pose
        
        # Set the marker's scale (0.2 meters wide) and color (red).
        marker.scale.x = 0.2
        marker.scale.y = 0.2
        marker.scale.z = 0.2
        marker.color.r = 1.0
        marker.color.a = 1.0  # Alpha (transparency) must be 1.0 to be visible
        
        self.marker_pub.publish(marker)

def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = TF2DataTransformerNode()
        
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