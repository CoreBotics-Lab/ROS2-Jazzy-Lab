#ifndef DIFF_DRIVE_KINEMATICS_MATRIX_METHOD_HPP
#define DIFF_DRIVE_KINEMATICS_MATRIX_METHOD_HPP

#include <geometry_msgs/msg/transform_stamped.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <memory>
#include <nav_msgs/msg/odometry.hpp>
#include <optional>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <std_msgs/msg/float64_multi_array.hpp>
#include <tf2_ros/transform_broadcaster.h>

// Professional combination profile headers
#include <Eigen/Geometry>
#include <tf2/LinearMath/Quaternion.h>
#include <Eigen/Core>

class DiffDriveKinematicsMatrixMethodClass : public rclcpp::Node {
public:
  DiffDriveKinematicsMatrixMethodClass();

private:
  double wheel_radius_;
  double wheel_separation_;
  double x_;
  double y_;
  double theta_;

  std::optional<double> last_left_wheel_pos_;
  std::optional<double> last_right_wheel_pos_;
  rclcpp::Time prev_time_;

  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_subscriber_;
  rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_state_subscriber_;
  rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr wheel_speed_publisher_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_publisher_;

  std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

  Eigen::Matrix<double, 2, 2> M_inv_;

  struct RobotVelocities {
    double linear;
    double angular;
  };

  Eigen::Matrix<double, 2, 1> compute_wheel_velocities(double Vb, double Omegab);
  Eigen::Matrix<double, 3, 1> forward_kinematics(double left_wheel_vel, double right_wheel_vel);

  void callback_cmd_vel(const geometry_msgs::msg::Twist::SharedPtr msg);
  void callback_joint_states(const sensor_msgs::msg::JointState::SharedPtr msg);
  void publish_odom_tf(const RobotVelocities &velocities, const rclcpp::Time &sim_time);
};

#endif // DIFF_DRIVE_KINEMATICS_MATRIX_METHOD_HPP