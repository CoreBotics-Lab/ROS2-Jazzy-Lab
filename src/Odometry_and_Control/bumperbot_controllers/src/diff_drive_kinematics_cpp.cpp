#include "bumperbot_controller/diff_drive_kinematics.hpp"
#include <tf2/LinearMath/Quaternion.h>
#include <cmath>

using namespace std::chrono_literals;

using Twist = geometry_msgs::msg::Twist;
using Float64MultiArray = std_msgs::msg::Float64MultiArray;
using JointState = sensor_msgs::msg::JointState;
using Odometry = nav_msgs::msg::Odometry;

// 1. Constructor Implementation
DiffDriveKinematicsClass::DiffDriveKinematicsClass()
    : Node("diff_drive_kinematics"), x_(0.0), y_(0.0), theta_(0.0) {
  RCLCPP_INFO(this->get_logger(), "%s node has been successfully initialized.",
              this->get_name());
  rcl_interfaces::msg::ParameterDescriptor desc;
  desc.dynamic_typing = true;

  this->declare_parameter("wheel_radius", rclcpp::ParameterValue(), desc);
  this->declare_parameter("wheel_separation", rclcpp::ParameterValue(), desc);

  auto wheel_radius_param = this->get_parameter("wheel_radius");
  auto wheel_separation_param = this->get_parameter("wheel_separation");

  if (wheel_radius_param.get_type() == rclcpp::ParameterType::PARAMETER_NOT_SET ||
      wheel_separation_param.get_type() == rclcpp::ParameterType::PARAMETER_NOT_SET) {
    throw std::runtime_error("A required parameter was not set! Please load a config file.");
  }

  this->wheel_radius_ = wheel_radius_param.as_double();
  this->wheel_separation_ = wheel_separation_param.as_double();

  RCLCPP_INFO(this->get_logger(), "Successfully loaded parameters from config file:");
  RCLCPP_INFO(this->get_logger(), "-> wheel_radius: %.3fm", this->wheel_radius_);
  RCLCPP_INFO(this->get_logger(), "-> wheel_separation: %.3fm", this->wheel_separation_);

  this->cmd_vel_subscriber_ = this->create_subscription<Twist>(
      "/cmd_vel", 10, [this](const Twist::SharedPtr msg) -> void {
        this->callback_cmd_vel(msg);
      });

  this->joint_state_subscriber_ = this->create_subscription<JointState>(
      "/joint_states", 10, [this](const JointState::SharedPtr msg) -> void {
        this->callback_joint_states(msg);
      });

  this->wheel_speed_publisher_ = this->create_publisher<Float64MultiArray>(
      "/simple_velocity_controller/commands", 10);
      
  this->odom_publisher_ = this->create_publisher<Odometry>("/odom", 10);
  this->tf_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(this);
}

// 2. Callback Implementation
void DiffDriveKinematicsClass::callback_cmd_vel(const Twist::SharedPtr msg) {
  double v_linear = msg->linear.x;
  double v_angular = msg->angular.z;

  double left_wheel_speed = (v_linear - (v_angular * this->wheel_separation_ / 2.0)) / this->wheel_radius_;
  double right_wheel_speed = (v_linear + (v_angular * this->wheel_separation_ / 2.0)) / this->wheel_radius_;

  auto wheel_speed_msg = std::make_shared<Float64MultiArray>();
  wheel_speed_msg->data.resize(2, 0.0);
  wheel_speed_msg->data[0] = left_wheel_speed;
  wheel_speed_msg->data[1] = right_wheel_speed;

  this->wheel_speed_publisher_->publish(*wheel_speed_msg);
}

void DiffDriveKinematicsClass::callback_joint_states(const JointState::SharedPtr msg) {
  rclcpp::Time current_time = msg->header.stamp;

  double left_wheel_position = 0.0;
  double right_wheel_position = 0.0;
  
  for (size_t i = 0; i < msg->name.size(); ++i) {
    if (msg->name[i] == "wheel_left_joint") left_wheel_position = msg->position[i];
    if (msg->name[i] == "wheel_right_joint") right_wheel_position = msg->position[i];
  }

  if (!this->last_left_wheel_pos_.has_value() || !this->last_right_wheel_pos_.has_value()) {
    this->last_left_wheel_pos_ = left_wheel_position;
    this->last_right_wheel_pos_ = right_wheel_position;
    this->prev_time_ = current_time;
    return;
  }

  double dt = (current_time - this->prev_time_).seconds();
  if (dt <= 0.0) return;

  this->prev_time_ = current_time;

  double delta_left_wheel_pos = left_wheel_position - this->last_left_wheel_pos_.value();
  double delta_right_wheel_pos = right_wheel_position - this->last_right_wheel_pos_.value();

  this->last_left_wheel_pos_ = left_wheel_position;
  this->last_right_wheel_pos_ = right_wheel_position;

  double distance_left = this->wheel_radius_ * delta_left_wheel_pos;
  double distance_right = this->wheel_radius_ * delta_right_wheel_pos;

  double delta_distance = (distance_left + distance_right) / 2.0;
  double delta_angle = (distance_right - distance_left) / this->wheel_separation_;

  double mid_theta = this->theta_ + (delta_angle / 2.0);
  this->x_ += delta_distance * std::cos(mid_theta);
  this->y_ += delta_distance * std::sin(mid_theta);
  this->theta_ += delta_angle;

  this->theta_ = std::atan2(std::sin(this->theta_), std::cos(this->theta_));

  RobotVelocities velocities;
  velocities.linear = delta_distance / dt;
  velocities.angular = delta_angle / dt;

  this->publish_odom_tf(velocities, current_time);
}

void DiffDriveKinematicsClass::publish_odom_tf(const RobotVelocities& velocities, const rclcpp::Time& sim_time) {
  tf2::Quaternion q;
  q.setRPY(0, 0, this->theta_);

  Odometry odom_msg;
  odom_msg.header.stamp = sim_time;
  odom_msg.header.frame_id = "odom";
  odom_msg.child_frame_id = "base_footprint";

  odom_msg.pose.pose.position.x = this->x_;
  odom_msg.pose.pose.position.y = this->y_;
  odom_msg.pose.pose.position.z = 0.0;
  odom_msg.pose.pose.orientation.x = q.x();
  odom_msg.pose.pose.orientation.y = q.y();
  odom_msg.pose.pose.orientation.z = q.z();
  odom_msg.pose.pose.orientation.w = q.w();

  odom_msg.twist.twist.linear.x = velocities.linear;
  odom_msg.twist.twist.angular.z = velocities.angular;

  this->odom_publisher_->publish(odom_msg);

  geometry_msgs::msg::TransformStamped transform;
  transform.header.stamp = sim_time;
  transform.header.frame_id = "odom";
  transform.child_frame_id = "base_footprint";

  transform.transform.translation.x = this->x_;
  transform.transform.translation.y = this->y_;
  transform.transform.translation.z = 0.0;
  transform.transform.rotation.x = q.x();
  transform.transform.rotation.y = q.y();
  transform.transform.rotation.z = q.z();
  transform.transform.rotation.w = q.w();

  this->tf_broadcaster_->sendTransform(transform);
}

// 3. Main Execution Loop
int main(int argc, char *argv[]) {
  auto log = rclcpp::get_logger("System");
  DiffDriveKinematicsClass::SharedPtr node_instance = nullptr;

  try {
    RCLCPP_INFO(log, "Initializing the ROS 2 Client...");
    rclcpp::init(argc, argv);

    RCLCPP_INFO(log, "Starting the ROS 2 Node...");
    node_instance = std::make_shared<DiffDriveKinematicsClass>();
    rclcpp::spin(node_instance);

    RCLCPP_WARN(log, "[CTRL+C]>>> Interrupted by the user.");
    RCLCPP_INFO(log, "Destroying the ROS 2 Node...");
  } catch (const std::exception &e) {
    RCLCPP_ERROR(log, "Critical Error: %s", e.what());
  }

  if (rclcpp::ok()) {
    RCLCPP_INFO(log, "Manually shutting down the ROS 2 client...");
    rclcpp::shutdown();
  }

  return 0;
}