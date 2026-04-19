#!/usr/bin/env python3
import sys
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from std_msgs.msg import Float64MultiArray
from sensor_msgs.msg import JointState

class MotorTestBenchPIDNode(Node):
    def __init__(self):
        super().__init__("MotorTestBenchPIDNode")
        self.get_logger().info(f'{self.get_name()} has been started...')
        self.pub_speed_ = self.create_publisher(Float64MultiArray, '/velocity_controller/commands', 10)
        self.speed_: float = float(sys.argv[1])
        self.jointStateSub_ = self.create_subscription(JointState, '/joint_states', self.jointStateSub_callback, 10)
        self.kP: float = 10.0
        self.setPoint_ = float(sys.argv[1])
        self.error = 0.0
        self.is_position_reached = False

    def jointStateSub_callback(self, msg: JointState):
        
        joint_data = dict(zip(msg.name, msg.position))
        if "rotor_joint" in joint_data:
            self.error = joint_data["rotor_joint"]  
            self.get_logger().info(f'Current Position: {joint_data["rotor_joint"]}')

        pub_msg_ = Float64MultiArray()
        error: float = self.setPoint_ - self.error
        # pub_msg_.data = [self.kP * error]
        if self.is_position_reached:
            pub_msg_.data = [0.0]
        elif abs(error) < 0.01:
            self.is_position_reached = True
            self.get_logger().info('Target Position Reached.')
            pub_msg_.data = [0.0]
        else:
            pub_msg_.data = [self.kP * error]

        self.pub_speed_.publish(pub_msg_)
        
        if self.is_position_reached:
            rclpy.shutdown()
            return




def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = MotorTestBenchPIDNode()
        
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