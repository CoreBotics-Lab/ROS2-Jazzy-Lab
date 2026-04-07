#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from rclpy.executors import MultiThreadedExecutor
from rclpy.callback_groups import MutuallyExclusiveCallbackGroup
import time
import threading

# ==============================================================================
# 🧠 CONCEPT: Group Blocking vs Node Blocking
# This script demonstrates that even with a MultiThreadedExecutor, callbacks
# sharing the SAME MutuallyExclusiveCallbackGroup will block each other.
# - Timer 1 and Timer 2 share Group A. Timer 2's sleep WILL block Timer 1.
# - Timer 3 is in Group B. It will run concurrently and won't block Group A.
# ==============================================================================

class MultiGroupDemoNode(Node):
    def __init__(self):
        super().__init__('multi_group_demo_node')
        self.get_logger().info(f"[{self.get_name()}] Node has been started.")
        
        # Create two distinct Mutually Exclusive groups
        self.group_a = MutuallyExclusiveCallbackGroup()
        self.group_b = MutuallyExclusiveCallbackGroup()

        # Timer 1 (Fast): Ticks every 0.5s -> Assigned to Group A
        self.timer_1 = self.create_timer(
            0.5, 
            self.timer_1_callback, 
            callback_group=self.group_a
        )

        # Timer 2 (Medium): Ticks every 1.0s -> Assigned to Group A
        self.timer_2 = self.create_timer(
            1.0, 
            self.timer_2_callback, 
            callback_group=self.group_a
        )
        
        # Timer 3 (Slow): Ticks every 2.0s -> Assigned to Group B
        self.timer_3 = self.create_timer(
            2.0, 
            self.timer_3_callback, 
            callback_group=self.group_b
        )

    def timer_1_callback(self):
        thread_name = threading.current_thread().name
        self.get_logger().info(f"[TIMER 1 - Grp A] Tick! (Thread: {thread_name})")

    def timer_2_callback(self):
        thread_name = threading.current_thread().name
        self.get_logger().warn(f"[TIMER 2 - Grp A] Starting 2.0s sleep on Thread: {thread_name}...")
        time.sleep(2.0) 
        self.get_logger().warn(f"[TIMER 2 - Grp A] Woke up and finished on Thread: {thread_name}")
        
    def timer_3_callback(self):
        thread_name = threading.current_thread().name
        self.get_logger().error(f"[TIMER 3 - Grp B] Starting 5.0s sleep on Thread: {thread_name}...")
        time.sleep(5.0) 
        self.get_logger().error(f"[TIMER 3 - Grp B] Woke up and finished on Thread: {thread_name}")

def main(args=None):
    rclpy.init(args=args)
    logger = get_logger("System")
    node = None
    
    try:
        logger.info("Starting Multi-Group Executor Demo...")
        node = MultiGroupDemoNode()
        executor = MultiThreadedExecutor()
        executor.add_node(node)
        logger.info(f"Spinning with {type(executor).__name__}...")
        executor.spin()
    except KeyboardInterrupt:
        logger.warn("[CTRL+C] >>> Interrupted by the User.")
    except Exception as e:
        logger.error(f"Critical Error: {e}")
    finally:
        if node:
            logger.info("Destroying the ROS2 Node...")
            node.destroy_node()
        if rclpy.ok():
            logger.info("Manually shutting down the ROS2 client...")
            rclpy.try_shutdown()

if __name__ == '__main__':
    main()