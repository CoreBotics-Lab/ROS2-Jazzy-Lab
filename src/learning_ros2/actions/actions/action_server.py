#!/usr/bin/env python3
import rclpy
import time
from rclpy.action import ActionServer
from rclpy.action import CancelResponse, GoalResponse
from rclpy.node import Node
from rclpy.executors import MultiThreadedExecutor
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
            goal_callback=self.goal_callback,
            cancel_callback=self.cancel_callback,
            execute_callback=self.execute_callback)

    def goal_callback(self, goal_request):
        """Accept or reject a client request to begin an action."""
        self.get_logger().info(f'Received goal request to count to {goal_request.target_number}')
        # For this example, we will accept all goals.
        return GoalResponse.ACCEPT

    def cancel_callback(self, goal_handle):
        """
        Accept or reject a client request to cancel an action.
        This callback is the gatekeeper for cancellation. Returning ACCEPT allows
        the 'is_cancel_requested' flag to be set in the execute_callback.
        """
        self.get_logger().info('Received cancel request, accepting.')
        return CancelResponse.ACCEPT

    def execute_callback(self, goal_handle):
        """Executes the action goal by counting up to the target number."""
        self.get_logger().info('Executing goal...')

        goal_number = goal_handle.request.target_number
        feedback_msg = Counter.Feedback()
        sequence = [] 

        # Create a rate object to control the loop frequency (5 Hz = 1 / 0.2s)
        rate = self.create_rate(5) # 5 Hz

        for i in range(goal_number + 1):
            if goal_handle.is_cancel_requested:
                goal_handle.canceled()
                self.get_logger().info('Goal canceled')
                return Counter.Result() # Exit immediately.
            
            sequence.append(i)
            feedback_msg.current_sequence = sequence
            goal_handle.publish_feedback(feedback_msg)
            
            self.get_logger().info(f'Feedback: {feedback_msg.current_sequence}')
            # Use the rate object to sleep for the correct duration to maintain the frequency
            rate.sleep()

        # You must explicitly set the final state of the goal before returning.
        # If you simply return, the framework will assume the goal was aborted,
        # as shown by the "Goal state not set" warning.
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

        # A MultiThreadedExecutor is required for a responsive, cancellable action server.
        # This allows the server to process cancel requests even if the execute_callback is blocked.
        executor = MultiThreadedExecutor()
        executor.add_node(node_instance)
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