#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from rclpy.executors import MultiThreadedExecutor
from rclpy.callback_groups import ReentrantCallbackGroup
import time
import threading

# ==============================================================================
# CONCEPT: The Reentrant Group & Self-Overlap
# This script demonstrates a Reentrant group. The Timer ticks every 0.5s, but
# takes 2.0s to finish. The Executor will spawn multiple threads to run this 
# EXACT SAME callback concurrently. 
# We MUST use a mutex (Lock) to protect the shared variable `self.counter`!
# ==============================================================================

class ReentrantDemoNode(Node):
    def __init__(self):
        super().__init__('reentrant_demo_node')
        self.get_logger().info(f"[{self.get_name()}] Node has been started.")
        
        # The Danger Zone Group
        self.reentrant_group = ReentrantCallbackGroup()
        
        # Shared memory that multiple threads will try to touch at the same time
        self.counter = 0
        
        # The Mutex (Lock). This is the Python equivalent of `std::mutex`.
        self.mutex = threading.Lock()

        # Timer ticks every 0.5s -> Assigned to Reentrant Group
        self.timer = self.create_timer(
            0.5, 
            self.timer_callback, 
            callback_group=self.reentrant_group
        )

    def timer_callback(self):
        thread_name = threading.current_thread().name
        self.get_logger().info(f"[ENTER] Timer triggered on Thread: {thread_name}")
        
        # --- CRITICAL SECTION (The Single-Occupancy Restroom) ---
        # The `with` statement safely locks the mutex, modifies the memory, 
        # and automatically unlocks it. It is the exact equivalent of C++ `std::lock_guard`
        with self.mutex:
            self.counter += 1
            current_count = self.counter
            self.get_logger().info(f"[{thread_name}] Safely incremented counter to: {current_count}")
            
        # --- NON-CRITICAL SECTION (Concurrent Execution) ---
        # Threads will overlap here!
        self.get_logger().warn(f"[{thread_name}] Starting heavy 2.0s work...")
        time.sleep(2.0) 
        self.get_logger().warn(f"[EXIT] Finished work on Thread: {thread_name}")

def main(args=None):
    rclpy.init(args=args)
    logger = get_logger("System")
    node = None
    
    try:
        logger.info("Starting Reentrant Executor Demo...")
        node = ReentrantDemoNode()
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

# ==============================================================================
# EXPERIMENT RESULTS:
# 
# Scenario 1: Without `time.sleep(2.0)` (The Fast Worker)
# If you comment out the sleep, you will see `ThreadPoolExecutor-0_0` used 
# over and over. The callback grabs the lock, increments, and exits instantly.
# When the next 0.5s tick happens, Clerk 0 is free, so the Manager just reuses
# Clerk 0. No new threads are spawned!
#
# Scenario 2: With `time.sleep(2.0)` (The Self-Overlap)
# The callback finishes the locked section instantly, but then sleeps for 2s.
# 0.5s later, the Timer triggers. Clerk 0 is still sleeping, so the Manager 
# MUST hire Clerk 1 (`0_1`). 0.5s later, both are sleeping, so it hires Clerk 2.
# This loop requires 4 new clerks to cover the 2-second sleep window before 
# Clerk 0 finally wakes up and becomes available again.
# 
# This perfectly proves the Executor only creates new threads if the active 
# ones are blocked AND the Callback Group rules (Reentrant) allow for it!
# ==============================================================================