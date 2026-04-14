#!/usr/bin/env python3
import math
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from rclpy.time import Time
from rclpy.clock_type import ClockType

from geometry_msgs.msg import Twist
from turtlesim.srv import Spawn

from tf2_ros.buffer import Buffer
from tf2_ros.transform_listener import TransformListener
from tf2_ros import TransformException  # type: ignore

class TurtleFollowerNode(Node):
    def __init__(self):
        super().__init__('turtle_follower_node')
        self.get_logger().info(f"{self.get_name()} has been started!")

        # 1. Spawn turtle2 using a Service Client
        self.spawn_client = self.create_client(Spawn, 'spawn')
        while not self.spawn_client.wait_for_service(timeout_sec=1.0):
            self.get_logger().info('Spawn service not available, waiting again...')
        
        request = Spawn.Request()
        request.x = 2.0
        request.y = 2.0
        request.theta = 0.0
        request.name = 'turtle2'
        
        # Call the service asynchronously so we don't block the node's initialization
        self.spawn_client.call_async(request)
        self.get_logger().info("Spawned turtle2!")

        # 2. Create the TF2 Buffer and Listener
        self.tf_buffer = Buffer()
        self.tf_listener = TransformListener(self.tf_buffer, self)

        # 3. Create a publisher to drive turtle2
        self.cmd_vel_pub = self.create_publisher(Twist, 'turtle2/cmd_vel', 10)

        # 4. Create a control loop timer (30 Hz)
        self.timer = self.create_timer(1.0 / 30.0, self.timer_callback)

    def timer_callback(self):
        try:
            # QUESTION: "Where is turtle1 relative to turtle2?"
            # Target Frame: 'turtle2' (The frame we want the coordinates to be in)
            # Source Frame: 'turtle1' (The object we are tracking)
            t = self.tf_buffer.lookup_transform('turtle2', 'turtle1', Time(clock_type=ClockType.ROS_TIME))
        except TransformException as ex:
            # If turtle2 hasn't spawned yet, or the broadcasters aren't running, just wait.
            return

        # Because we asked for the coordinates relative to turtle2, 
        # 'x' is distance straight ahead, and 'y' is distance to the left.
        x = t.transform.translation.x
        y = t.transform.translation.y
        
        # Calculate Euclidean distance and Angle (atan2)
        distance = math.sqrt(x**2 + y**2)
        angle = math.atan2(y, x)

        # Calculate the error based on our desired following distance
        target_distance = 1.0
        distance_error = distance - target_distance

        # Simple Proportional Controller (PID with just the P)
        msg = Twist()
        msg.linear.x = 1.5 * distance_error
        msg.angular.z = 4.0 * angle

        self.cmd_vel_pub.publish(msg)

def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = TurtleFollowerNode()
        
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

# --- HOW TO RUN THIS DEMO ---
# Terminal 1: ros2 run turtlesim turtlesim_node
# Terminal 2: ros2 run turtle_tf2_py turtle_tf2_broadcaster --ros-args -p turtlename:=turtle1
# Terminal 3: ros2 run turtle_tf2_py turtle_tf2_broadcaster --ros-args -p turtlename:=turtle2
# Terminal 4: ros2 run ros2_playground turtle_follower
# Terminal 5: ros2 run turtlesim turtle_teleop_key
