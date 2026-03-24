#!/usr/bin/env python3
import rclpy
import time
from rclpy.action import ActionServer
from rclpy.action.server import ServerGoalHandle
from rclpy.node import Node
from rclpy.logging import get_logger
from ros2_interfaces.action import Counter

class CounterActionServer(Node):
    def __init__(self):
        super().__init__('counter_action_server')
        self.get_logger().info(f'{self.get_name()} has been started.')
        self._action_server = ActionServer(
            self,
            Counter,
            'counter',
            execute_callback=self.execute_callback
                )

    def execute_callback(self, goal_handle: ServerGoalHandle):
        """Executes the action goal by counting up to the target number."""
        self.get_logger().info('Executing goal...')

        goal_number = goal_handle.request.target_number
        feedback_msg = Counter.Feedback()
        sequence = [] 

        for i in range(goal_number + 1):
            if goal_handle.is_cancel_requested:
                goal_handle.canceled()
                self.get_logger().info('Goal canceled')
                return Counter.Result() # Exit immediately.

            feedback_msg.current_sequence = sequence
            goal_handle.publish_feedback(feedback_msg)
            
            self.get_logger().info(f'Feedback: {feedback_msg.current_sequence}')
            time.sleep(0.2)

        goal_handle.succeed()

        result = Counter.Result()
        result.final_sequence = sequence
        self.get_logger().info(f'Returning result: {result.final_sequence}')
        return result

def main(args=None):
    log = get_logger("System")
    node_instance = None
    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = CounterActionServer()
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