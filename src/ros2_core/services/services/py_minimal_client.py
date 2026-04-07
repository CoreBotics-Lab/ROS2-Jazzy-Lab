#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from ros2_interfaces.srv import Greetings

class Greeting_client_node_class(Node):
    def __init__(self) -> None:
        super().__init__("Greeting_client")
        self.get_logger().info(f"{self.get_name()} has been started!")
        self.client = self.create_client(Greetings, "greetings")
        self.request = Greetings.Request()
        self.timer_ = self.create_timer(0.5, self.timer_callback)

    def timer_callback(self):
        self.send_request(f"Hello from {self.get_name()}")

    def send_request(self, value: str):
        if self.client.service_is_ready():
            self.request.greetings = value
            future = self.client.call_async(self.request)
            future.add_done_callback(self.response_callback)
        else:
            self.get_logger().warn(f"Service is not found. Waiting for Service server...")
            
    def response_callback(self, future):
        try:
            response: Greetings.Response = future.result()
            if response.success:
                self.get_logger().info(f'Received response: "{response.response_greetings}"')
            else:
                self.get_logger().error(f'Service call failed: {response.response_greetings}')
            self.timer_.cancel()
            self.destroy_node()
            rclpy.shutdown()

        except Exception as e:
            self.get_logger().error(f'Service call failed: {e}')

        
            
    
def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = Greeting_client_node_class()
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