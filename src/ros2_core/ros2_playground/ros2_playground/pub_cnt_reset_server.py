#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from ros2_interfaces.srv import ResetCounter
from ros2_interfaces.msg import String


class cntReset_server_node_class(Node):
    def __init__(self) -> None:
        super().__init__("cntReset_server")
        self.get_logger().info(f"{self.get_name()} has been started!")
        self.cnt_ = 0
        self.msg_ = String()  # Create message object once
        self.publisher_ = self.create_publisher(String, '/counter', 10)
        self.service_ = self.create_service(ResetCounter, 'cntReset', self.cntReset_callback)
        self.timer_ = self.create_timer(0.5, self.timer_callback)
        

    def timer_callback(self):
        self.msg_.data = f"Counter: {self.cnt_}"
        self.publisher_.publish(self.msg_)
        self.get_logger().info(f'Publishing: "{self.msg_.data}"') 
        self.cnt_ += 1

    def cntReset_callback(self, request: ResetCounter.Request, response: ResetCounter.Response):
        try: 
            self.cnt_ = request.reset_value
            self.get_logger().info(f'Incoming request: "{request.reset_value}"')  
            response.success = True
            response.message = f"Counter has been reset to {self.cnt_}"

        except Exception as e:
            self.get_logger().error(f"An error occurred in the service callback: {e}")
            response.message = "Error: Service failed to process request."
            response.success = False

        return response



def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = cntReset_server_node_class()
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