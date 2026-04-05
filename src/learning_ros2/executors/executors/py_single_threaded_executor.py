#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from rclpy.executors import SingleThreadedExecutor
from rclpy.callback_groups import MutuallyExclusiveCallbackGroup
import time
import threading

# ==============================================================================
# 🧠 CONCEPT: rclpy.spin(node) works exactly like this script!
# Standard ROS 2 tutorials use `rclpy.spin()`, which secretly creates a 
# SingleThreadedExecutor and Mutually Exclusive groups under the hood. 
# We are just writing it out explicitly here to learn how it actually works.
# ==============================================================================

class SingleThreadedDemoNode(Node):
    def __init__(self):
        super().__init__('single_threaded_demo_node')
        self.get_logger().info(f"[{self.get_name()}] Node has been started.")
        
        # Callback Groups
        # NOTE: If you don't specify a callback group, ROS 2 assigns it to the node's default group,
        # which is ALWAYS a MutuallyExclusiveCallbackGroup. 
        # Even if you delete these groups, it will run exactly the same! Because the Executor 
        # only has 1 thread, everything in basic ROS 2 is mutually exclusive and blocking by default.
        # We explicitly write them here just to prepare for the Multi-Threaded file!
        self.fast_group = MutuallyExclusiveCallbackGroup()
        self.slow_group = MutuallyExclusiveCallbackGroup()

        # Fast Timer: Ticks every 0.5 seconds
        self.fast_timer = self.create_timer(
            0.5, 
            self.fast_timer_callback, 
            callback_group=self.fast_group
        )

        # Slow Timer: Ticks every 2.0 seconds
        self.slow_timer = self.create_timer(
            2.0, 
            self.slow_timer_callback, 
            callback_group=self.slow_group
        )

    def fast_timer_callback(self):
        thread_name = threading.current_thread().name
        self.get_logger().info(f"[FAST] Tick! (Running on thread: {thread_name})")

    def slow_timer_callback(self):
        thread_name = threading.current_thread().name
        self.get_logger().warn(f"[SLOW] Danger: Starting 3-second blocking sleep on thread: {thread_name}...")
        
        # The "Run-to-Completion" Trap!
        # This blocks the ONE available thread, paralyzing the node.
        time.sleep(3.0) 
        
        self.get_logger().warn(f"[SLOW] Woke up and finished on thread: {thread_name}")

def main(args=None):
    rclpy.init(args=args)
    logger = get_logger("System")
    node = None
    
    try:
        logger.info("Starting Single-Threaded Executor Demo...")
        node = SingleThreadedDemoNode()
        
        # --- THE SHIFT MANAGER ---
        # NOTE: Calling `rclpy.spin(node)` does the exact same thing as the 3 lines below.
        # `rclpy.spin()` secretly creates a SingleThreadedExecutor under the hood!
        # We write it out explicitly here to expose the architecture and to match
        # the exact syntax we will use for the MultiThreadedExecutor.
        executor = SingleThreadedExecutor()
        executor.add_node(node)
        logger.info(f"Spinning with {type(executor).__name__}...")
        executor.spin()
        
    except KeyboardInterrupt:
        logger.warn("[CTRL+C] >>> Interrupted by the User.")
    finally:
        if node:
            logger.info("Destroying the ROS2 Node...")
            node.destroy_node()
        if rclpy.ok():
            logger.info("Manually shutting down the ROS2 client...")
            rclpy.try_shutdown()

if __name__ == '__main__':
    main()