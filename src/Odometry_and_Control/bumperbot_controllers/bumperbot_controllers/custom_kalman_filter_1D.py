#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from nav_msgs.msg import Odometry
from sensor_msgs.msg import Imu


class KalmanFilter1D(Node):
    """
    1D Kalman Filter for fusing noisy Odometry (motion model) and IMU (sensor measurement).
    """

    def __init__(self):
        super().__init__("kalman_filter")
        self.odom_sub_ = self.create_subscription(Odometry, "/odom_noisy", self.odomCallback, 10)
        self.imu_sub_ = self.create_subscription(Imu, "/imu", self.imuCallback, 10)
        self.odom_pub_ = self.create_publisher(Odometry, "/odom_kalman", 10)
        
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
            return
        
        # Calculate motion command (difference in odometry)
        self.motion_ = odom.twist.twist.angular.z - self.last_angular_z_

        # 1. Predict Step (State Prediction based on motion)
        self.statePrediction()
        
        # 2. Update Step (Measurement Update based on IMU sensor)
        self.measurementUpdate()

        # Update for the next iteration
        self.last_angular_z_ = odom.twist.twist.angular.z

        # Update and publish the filtered odom message
        self.kalman_odom_.twist.twist.angular.z = self.mean_
        self.odom_pub_.publish(self.kalman_odom_)


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
