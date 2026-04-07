#!/usr/bin/env python3
import sys
import rclpy
from rclpy.action import ActionClient
from rclpy.node import Node
from rclpy.logging import get_logger
from action_msgs.msg import GoalStatus
from ros2_interfaces.action import GoToGoal

class GoToGoalActionClient(Node):
    """
    An action client to send navigation goals to the turtleGoToGoalActionServer.
    """
    def __init__(self):
        super().__init__('go_to_goal_action_client')
        self._action_client = ActionClient(self, GoToGoal, 'go_to_goal')
        self._goal_done = False
        self._goal_handle = None

    @property
    def goal_done(self):
        """Property to check if the goal is done (succeeded, rejected, or canceled)."""
        return self._goal_done

    def send_goal(self, x, y, theta):
        """Sends a goal to the action server and handles the response."""
        goal_msg = GoToGoal.Goal()
        goal_msg.x = float(x)
        goal_msg.y = float(y)
        goal_msg.theta = float(theta)

        self.get_logger().info('Waiting for action server...')
        self._action_client.wait_for_server()

        self.get_logger().info(f'Sending goal request: (x:{x}, y:{y}, theta:{theta})')
        
        # First future: the goal handshake
        self._send_goal_future = self._action_client.send_goal_async(
            goal_msg,
            feedback_callback=self.feedback_callback)
        
        self._send_goal_future.add_done_callback(self.goal_response_callback)

    def goal_response_callback(self, future):
        """Callback for when the server accepts or rejects the goal."""
        goal_handle = future.result()
        if not goal_handle.accepted:
            self.get_logger().info('Goal rejected by server.')
            self._goal_done = True
            return

        self.get_logger().info('Goal accepted by server.')
        self._goal_handle = goal_handle

        # Second future: the result request
        self._get_result_future = goal_handle.get_result_async()
        self._get_result_future.add_done_callback(self.get_result_callback)

    def get_result_callback(self, future):
        """Callback for when the server returns the final result."""
        result_handle = future.result()
        status = result_handle.status
        
        if status == GoalStatus.STATUS_SUCCEEDED:
            self.get_logger().info('Goal succeeded!')
            self.get_logger().info(f'Result: {result_handle.result.message}')
        elif status == GoalStatus.STATUS_CANCELED:
            self.get_logger().info('Goal was canceled.')
        elif status == GoalStatus.STATUS_ABORTED:
            self.get_logger().info(f'Goal was aborted by the server: {result_handle.result.message}')
        else:
            self.get_logger().warn(f'Goal finished with unknown status: {status}')
        
        self._goal_done = True

    def feedback_callback(self, feedback_msg):
        """Callback for receiving feedback during goal execution."""
        feedback = feedback_msg.feedback
        self.get_logger().info(
            f'Feedback: At (x:{feedback.current_x:.2f}, y:{feedback.current_y:.2f}), '
            f'distance to goal: {feedback.distance_remaining:.2f}'
        )

    def cancel_goal(self):
        """Sends a cancel request to the server."""
        if self._goal_handle is not None and self._goal_handle.status in [
                GoalStatus.STATUS_ACCEPTED, GoalStatus.STATUS_EXECUTING]:
            # This log call is removed to prevent a race condition during shutdown.
            # When Ctrl+C is pressed, this can race with rclpy.shutdown(), causing a
            # 'context invalid' error. The log in the KeyboardInterrupt handler is sufficient.
            self._goal_handle.cancel_goal_async()

def main(args=None):
    rclpy.init(args=args)
    action_client = None
    log = get_logger("main")

    if len(sys.argv) != 4:
        log.error("Incorrect number of arguments. Usage: ros2 run <pkg> <node> x y theta")
        rclpy.shutdown()
        return

    try:
        action_client = GoToGoalActionClient()
        action_client.send_goal(sys.argv[1], sys.argv[2], sys.argv[3])

        while rclpy.ok() and not action_client.goal_done:
            rclpy.spin_once(action_client, timeout_sec=0.1)

    except KeyboardInterrupt:
        log.warn("User interrupted execution. Canceling goal...")
        if action_client:
            action_client.cancel_goal()
            while rclpy.ok() and not action_client.goal_done:
                rclpy.spin_once(action_client, timeout_sec=0.1)
    finally:
        if action_client:
            action_client.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()

if __name__ == '__main__':
    main()