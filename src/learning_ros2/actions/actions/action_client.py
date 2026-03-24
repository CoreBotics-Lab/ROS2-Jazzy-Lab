#!/usr/bin/env python3
import sys
import rclpy
from rclpy.action import ActionClient
from action_msgs.msg import GoalStatus
from rclpy.logging import get_logger
from rclpy.node import Node

from ros2_interfaces.action import Counter

class CounterActionClient(Node):

    def __init__(self):
        super().__init__('counter_action_client')
        self._action_client = ActionClient(self, Counter, 'counter')
        self._goal_done = False
        self._goal_handle = None
        self._last_feedback = None

    @property
    def goal_done(self):
        """Property to check if the goal is done (succeeded, rejected, or canceled)."""
        return self._goal_done

    def send_goal(self, target_number):
        """Sends a goal to the action server and handles the response."""
        goal_msg = Counter.Goal()
        goal_msg.target_number = target_number

        self.get_logger().info('Waiting for action server...')
        self._action_client.wait_for_server()

        self.get_logger().info(f'Sending goal request to count to {target_number}...')
        
        # This is the first future: the goal handshake
        self._send_goal_future = self._action_client.send_goal_async(
            goal_msg,
            feedback_callback=self.feedback_callback)
        
        self._send_goal_future.add_done_callback(self.goal_response_callback)

    def goal_response_callback(self, future):
        """Callback for when the server accepts or rejects the goal."""
        goal_handle = future.result()
        if not goal_handle.accepted:
            self.get_logger().info('Goal rejected :(')
            self._goal_done = True  # Signal that the action is complete
            return # Stop processing since the goal was rejected

        self.get_logger().info('Goal accepted :)')
        self._goal_handle = goal_handle

        # This is the second future: the result request
        self._get_result_future = goal_handle.get_result_async()
        self._get_result_future.add_done_callback(self.get_result_callback)

    def get_result_callback(self, future):
        """Callback for when the server returns the final result."""
        result_handle = future.result()
        result_payload = result_handle.result
        status = result_handle.status
        
        if status == GoalStatus.STATUS_SUCCEEDED:
            # The result_payload can be empty due to a race condition when using a
            # MultiThreadedExecutor on the server. Instead, we use the last feedback
            # message received, which is guaranteed to contain the final sequence.
            self.get_logger().info(f'Goal succeeded! Result: {self._last_feedback}')
        elif status == GoalStatus.STATUS_CANCELED:
            self.get_logger().info('Goal was canceled.')
        elif status == GoalStatus.STATUS_ABORTED:
            self.get_logger().info('Goal was aborted by the server.')
        else:
            self.get_logger().warn(f'Goal finished with unknown status: {status}')
        
        self._goal_done = True # Signal that the action is complete

    def feedback_callback(self, feedback_msg):
        """Callback for receiving feedback during goal execution."""
        feedback = feedback_msg.feedback
        self.get_logger().info(f'Received feedback: {feedback.current_sequence}')
        # Store the latest feedback to ensure we have the final result,
        # working around the race condition in the result callback.
        self._last_feedback = feedback.current_sequence

    def cancel_goal(self):
        """Sends a cancel request to the server."""
        # The 'is_active' property is for the ServerGoalHandle, not the ClientGoalHandle.
        # We check the status instead. A goal can be canceled if it is accepted or executing.
        if self._goal_handle is not None and self._goal_handle.status in [
                GoalStatus.STATUS_ACCEPTED, GoalStatus.STATUS_EXECUTING]:
            self.get_logger().info('Canceling goal...')
            self._goal_handle.cancel_goal_async()


def main(args=None):
    log = get_logger("System")
    action_client = None
    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        action_client = CounterActionClient()

        # Send a goal using the value from the first command-line argument.
        action_client.send_goal(int(sys.argv[1]))

        # Use a custom spin loop to wait for the action to complete.
        # This gives the main thread control over the application lifecycle.
        while rclpy.ok() and not action_client.goal_done:
            rclpy.spin_once(action_client, timeout_sec=0.1)

    except KeyboardInterrupt:
        log.warn("[CTRL+C]>>> Interrupted by the User. Canceling the active goal...")
        if action_client:
            action_client.cancel_goal()
            # Keep spinning until the result callback marks the goal as done.
            while rclpy.ok() and not action_client.goal_done:
                # Avoid ROS logging during the shutdown sequence to prevent "context invalid" errors.
                rclpy.spin_once(action_client, timeout_sec=0.1)

    except Exception as e:
        log.error(f"Critical Error: {e}")

    finally:
        if action_client is not None:
            log.info("Destroying the ROS2 Node...")
            action_client.destroy_node()
            action_client = None

        if rclpy.ok():
            log.info("Manually shutting down the ROS2 Client...")
            rclpy.shutdown()

if __name__ == '__main__':
    main()
