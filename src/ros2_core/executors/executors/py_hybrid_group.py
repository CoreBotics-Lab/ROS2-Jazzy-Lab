#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from rclpy.executors import MultiThreadedExecutor
from rclpy.callback_groups import MutuallyExclusiveCallbackGroup, ReentrantCallbackGroup
import time
import threading

class HybridDemoNode(Node):
    def __init__(self):
        super().__init__('hybrid_demo_node')
        self.get_logger().info(f"[{self.get_name()}] Node has been started.")
        
        # --- CALLBACK GROUPS ---
        # Group A: Strict. Only one callback inside this group can run at a time.
        self.exclusive_group = MutuallyExclusiveCallbackGroup()
        
        # Group B: Danger Zone. Callbacks here can overlap with everything.
        self.reentrant_group = ReentrantCallbackGroup()
        
        # Shared Memory & Lock
        self.counter = 0
        self.mutex = threading.Lock()

        # --- TIMERS ---
        # Timer 1 & 2 share the Exclusive Group. They will never overlap each other.
        self.timer1 = self.create_timer(1.0, self.timer1_callback, callback_group=self.exclusive_group)
        self.timer2 = self.create_timer(1.5, self.timer2_callback, callback_group=self.exclusive_group)

        # Timer 3 uses the Reentrant Group. It will self-overlap aggressively!
        self.timer3 = self.create_timer(0.5, self.timer3_callback, callback_group=self.reentrant_group)

    def timer1_callback(self):
        thread_name = threading.current_thread().name
        self.get_logger().info(f"[TIMER 1] (Exclusive) Tick on Thread: {thread_name}")
        time.sleep(0.1) # Small delay to prove it doesn't block Timer 3

    def timer2_callback(self):
        thread_name = threading.current_thread().name
        self.get_logger().info(f"[TIMER 2] (Exclusive) Tick on Thread: {thread_name}")
        time.sleep(0.1)


    def timer3_callback(self):
        thread_name = threading.current_thread().name
        self.get_logger().warn(f"[TIMER 3] [ENTER] Reentrant Tick on Thread: {thread_name}")
        
        # Critical Section
        with self.mutex:
            self.counter += 1
            current_count = self.counter
            self.get_logger().info(f"[{thread_name}] Safely incremented counter to: {current_count}")
            
        # The 2.0s Sleep (Non-Critical Section)
        time.sleep(2.0) 
        self.get_logger().warn(f"[TIMER 3] [EXIT] Finished work on Thread: {thread_name}")

def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = HybridDemoNode()
        executor = MultiThreadedExecutor()
        executor.add_node(node_instance)
        
        log.info(f"Spinning with {type(executor).__name__}...")
        executor.spin()

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