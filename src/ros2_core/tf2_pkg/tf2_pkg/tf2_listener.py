#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from rclpy.time import Time
from rclpy.clock import ClockType

# TF2 specific imports
from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener
from tf2_ros import TransformException  # type: ignore

class TF2ListenerNode(Node):
    """
    This node listens to the TF2 tree and queries the spatial relationship
    between two frames using a Buffer and a Listener.
    """
    def __init__(self):
        super().__init__('tf2_listener_node')
        self.get_logger().info(f"{self.get_name()} has been started!")

        # 1. Create the Buffer (The Memory Cache)
        # It stores up to 10 seconds of transform history by default.
        self.tf_buffer = Buffer()

        # 2. Create the Listener (The Network Subscriber)
        # This automatically hooks into /tf and /tf_static and feeds the Buffer.
        self.tf_listener = TransformListener(self.tf_buffer, self)

        # 3. Create a timer to query the buffer every 1 second (1.0 Hz)
        self.timer = self.create_timer(1.0, self.timer_callback)

    def timer_callback(self):
        # We want to know: "Where is my_dynamic_frame relative to the world?"
        target_frame = 'world'
        source_frame = 'my_dynamic_frame'

        try:
            # 4. Query the Buffer
            # Time() means "give me the newest available transform"
            t = self.tf_buffer.lookup_transform(
                target_frame,
                source_frame,
                Time(clock_type=ClockType.ROS_TIME) 
            )

            # Extract the coordinates and print them!
            x = t.transform.translation.x
            y = t.transform.translation.y
            z = t.transform.translation.z
            
            self.get_logger().info(f"Dynamic Frame Location -> X: {x:.2f}, Y: {y:.2f}, Z: {z:.2f}")

        except TransformException as ex:
            # If the frames don't exist yet, or aren't connected, lookup_transform throws an exception.
            # We MUST catch it so our node doesn't crash!
            self.get_logger().warn(f"Could not transform {target_frame} to {source_frame}: {ex}")

def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = TF2ListenerNode()
        
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