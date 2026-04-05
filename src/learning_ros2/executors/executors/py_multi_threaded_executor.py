#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from rclpy.executors import MultiThreadedExecutor
from rclpy.callback_groups import MutuallyExclusiveCallbackGroup
import time
import threading

# ==============================================================================
# 🧠 CONCEPT: True Concurrency in ROS 2
# To get two callbacks to run at the exact same time, you must have TWO things:
# 1. The Rules: Distinct Callback Groups (permission to run concurrently).
# 2. The Hardware: A MultiThreadedExecutor (the physical threads to do the work).
# This script uses both to perfectly bypass the "Run-to-Completion" trap!
# ==============================================================================

class MultiThreadedDemoNode(Node):
    def __init__(self):
        super().__init__('multi_threaded_demo_node')
        self.get_logger().info(f"[{self.get_name()}] Node has been started.")
        
        # Callback Groups
        # Because these are in distinct MutuallyExclusive groups, the MultiThreadedExecutor
        # can process them concurrently on completely different OS threads.
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
        
        # The sleep blocks this specific thread, but the executor has other
        # free threads in its pool to handle the fast_timer_callback!
        time.sleep(3.0) 
        
        self.get_logger().warn(f"[SLOW] Woke up and finished on thread: {thread_name}")

def main(args=None):
    rclpy.init(args=args)
    logger = get_logger("System")
    node = None
    
    try:
        logger.info("Starting Multi-Threaded Executor Demo...")
        node = MultiThreadedDemoNode()
        
        # --- THE SHIFT MANAGER (Multi-Threaded) ---
        # Instead of `rclpy.spin()`, we explicitly create a MultiThreadedExecutor.
        # This acts like an office manager with a whole pool of clerks (threads).
        # When the slow timer goes to sleep, the manager just hands the fast timer
        # to a different, free clerk so the node never stops responding!
        executor = MultiThreadedExecutor()
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