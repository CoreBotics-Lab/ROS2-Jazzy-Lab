#!/usr/bin/env python3
import rclpy
import math
from rclpy.node import Node
from rclpy.logging import get_logger
from nav_msgs.msg import Odometry
from sensor_msgs.msg import Imu
from tf2_ros import TransformBroadcaster
from geometry_msgs.msg import TransformStamped

# Spatial transformations library
from tf_transformations import quaternion_from_euler, euler_from_quaternion

class KalmanFilter1D(Node):
    """
    1D Kalman Filter for fusing noisy Odometry (motion model) and IMU (sensor measurement).
    """

    def __init__(self):
        super().__init__("kalman_filter")
        self.odom_sub_ = self.create_subscription(Odometry, "/odom_noisy", self.odomCallback, 10)
        self.imu_sub_ = self.create_subscription(Imu, "/imu", self.imuCallback, 10)
        self.odom_pub_ = self.create_publisher(Odometry, "/odom_kalman", 10)
        self.tf_broadcaster_ = TransformBroadcaster(self)

        # --- Initial State ---
        # Initially, the robot has no idea how fast it is going.
        # mean_: Estimated state (angular velocity z). Starts at 0.
        # variance_: Uncertainty in our state estimate. Starts very high (1000.0) 
        #            because we don't know the initial state.
        self.mean_ = 0.0
        self.variance_ = 1000.0

        # Modeling the uncertainty of the motion model and the sensor
        self.motion_variance_ = 4.0      # Uncertainty added by moving (predict step)
        self.measurement_variance_ = 0.5 # Uncertainty of the IMU sensor (update step)

        # Store the messages - only for the angular velocity z
        self.imu_angular_z_ = 0.0

        self.is_first_odom_ = True
        self.last_angular_z_ = 0.0
        self.motion_ = 0.0

        # Odometry integration variables
        self.x = 0.0
        self.y = 0.0
        self.theta = 0.0
        self.last_time_ = None

        # Publish the filtered odometry message
        self.kalman_odom_ = Odometry()

    def odomCallback(self, odom):
        self.kalman_odom_ = odom

        # --- Initialization on First Message ---
        # We need at least two odometry messages to calculate the difference (motion).
        # On the very first message, we simply seed our initial state (mean_) and 
        # store the reading so we can compare against it in the next callback.
        if self.is_first_odom_:
            self.last_angular_z_ = odom.twist.twist.angular.z
            self.is_first_odom_ = False
            self.mean_ = odom.twist.twist.angular.z
            
            # Initialize our pose with the first odometry reading
            self.x = odom.pose.pose.position.x
            self.y = odom.pose.pose.position.y
            q_in = [
                odom.pose.pose.orientation.x,
                odom.pose.pose.orientation.y,
                odom.pose.pose.orientation.z,
                odom.pose.pose.orientation.w
            ]
            _, _, self.theta = euler_from_quaternion(q_in)
            self.last_time_ = odom.header.stamp
            return
        
        # Calculate motion command (difference in odometry)
        self.motion_ = odom.twist.twist.angular.z - self.last_angular_z_

        # 1. Predict Step (State Prediction based on motion)
        self.statePrediction()
        
        # 2. Update Step (Measurement Update based on IMU sensor)
        self.measurementUpdate()

        # Update for the next iteration
        self.last_angular_z_ = odom.twist.twist.angular.z

        # Odometry Integration to calculate filtered position/orientation
        current_time = odom.header.stamp
        if self.last_time_ is not None:
            dt = (current_time.sec - self.last_time_.sec) + (current_time.nanosec - self.last_time_.nanosec) * 1e-9
            
            # Use raw linear velocity and FILTERED angular velocity
            v_linear = odom.twist.twist.linear.x
            v_angular = self.mean_
            
            # Mid-Angle Integration (more accurate than Euler integration)
            delta_theta = v_angular * dt
            self.x += v_linear * math.cos(self.theta + delta_theta / 2.0) * dt
            self.y += v_linear * math.sin(self.theta + delta_theta / 2.0) * dt
            self.theta += delta_theta
        
        self.last_time_ = current_time

        # Create quaternion from our new filtered heading
        q = quaternion_from_euler(0.0, 0.0, self.theta)

        # Update the kalman_odom_ message with our filtered state
        self.kalman_odom_.child_frame_id = "base_footprint_ekf"
        self.kalman_odom_.pose.pose.position.x = self.x
        self.kalman_odom_.pose.pose.position.y = self.y
        self.kalman_odom_.pose.pose.position.z = 0.0
        self.kalman_odom_.pose.pose.orientation.x = q[0]
        self.kalman_odom_.pose.pose.orientation.y = q[1]
        self.kalman_odom_.pose.pose.orientation.z = q[2]
        self.kalman_odom_.pose.pose.orientation.w = q[3]
        self.kalman_odom_.twist.twist.linear.x = odom.twist.twist.linear.x
        self.kalman_odom_.twist.twist.angular.z = self.mean_
        
        # Publish the updated Odometry message and TF
        self.odom_pub_.publish(self.kalman_odom_)
        self.publish_tf()


    def imuCallback(self, imu):
        # Store the measurement update from the IMU
        self.imu_angular_z_ = imu.angular_velocity.z


    def measurementUpdate(self):
        """
        Measurement Update (Correction Step)
        Fuses the predicted state with the actual sensor measurement (IMU).
        It calculates a weighted average of the predicted mean and the sensor measurement,
        weighted by their respective certainties (inverse of variance).
        The new variance will always be smaller than both the predicted variance and the measurement variance.
        """
        self.mean_ = (self.measurement_variance_ * self.mean_ + self.variance_ * self.imu_angular_z_) \
                   / (self.variance_ + self.measurement_variance_)
                     
        self.variance_ = (self.variance_ * self.measurement_variance_) \
                       / (self.variance_ + self.measurement_variance_)


    def statePrediction(self):
        """
        State Prediction (Predict Step)
        Predicts the new state based on the motion model (odometry change).
        The new mean is the old mean plus the motion.
        The uncertainty (variance) increases because moving adds uncertainty.
        """
        self.mean_ = self.mean_ + self.motion_
        self.variance_ = self.variance_ + self.motion_variance_
    
    def publish_tf(self) -> None:
        # Build and Broadcast the dynamic transform link over /tf
        transform = TransformStamped()
        transform.header.stamp = self.kalman_odom_.header.stamp  
        transform.header.frame_id = "odom"
        transform.child_frame_id = "base_footprint_ekf"

        transform.transform.translation.x = self.kalman_odom_.pose.pose.position.x
        transform.transform.translation.y = self.kalman_odom_.pose.pose.position.y
        transform.transform.translation.z = self.kalman_odom_.pose.pose.position.z
        transform.transform.rotation.x = self.kalman_odom_.pose.pose.orientation.x
        transform.transform.rotation.y = self.kalman_odom_.pose.pose.orientation.y
        transform.transform.rotation.z = self.kalman_odom_.pose.pose.orientation.z
        transform.transform.rotation.w = self.kalman_odom_.pose.pose.orientation.w

        self.tf_broadcaster_.sendTransform(transform)

def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = KalmanFilter1D() 
        
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
