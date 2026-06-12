#include "bumperbot_controller/diff_drive_kinematics_matrix_method.hpp"
#include <cmath>
#include <tf2/LinearMath/Quaternion.h>
#include <numeric>

using namespace std::chrono_literals;

using Twist = geometry_msgs::msg::Twist;
using Float64MultiArray = std_msgs::msg::Float64MultiArray;
using JointState = sensor_msgs::msg::JointState;
using Odometry = nav_msgs::msg::Odometry;

DiffDriveKinematicsMatrixMethodClass::DiffDriveKinematicsMatrixMethodClass()
    : Node("diff_drive_kinematics"), x_(0.0), y_(0.0), theta_(0.0),
      last_left_wheel_pos_(std::nullopt), last_right_wheel_pos_(std::nullopt) {
        
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

    // --- PRE-COMPUTE KINEMATICS MATRIX CACHE (Eigen Best Practice) ---
    Eigen::Matrix<double, 2, 2> M;
    M << 0.5, 0.5,
        -1.0 / this->wheel_separation_, 1.0 / this->wheel_separation_;
    
    this->M_inv_ = M.inverse();

    // --- TOPIC INTERFACES ---
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

void DiffDriveKinematicsMatrixMethodClass::callback_cmd_vel(const Twist::SharedPtr msg) {
    double Vb = msg->linear.x;
    double Omegab = msg->angular.z;

    Eigen::Matrix<double, 2, 1> wheel_velocities = this->compute_wheel_velocities(Vb, Omegab);

    auto wheel_velocities_msg = std::make_unique<Float64MultiArray>();
    wheel_velocities_msg->data = {wheel_velocities(0), wheel_velocities(1)};
    this->wheel_speed_publisher_->publish(std::move(wheel_velocities_msg));
}

//inverse kinematics
Eigen::Matrix<double, 2, 1> DiffDriveKinematicsMatrixMethodClass::compute_wheel_velocities(double Vb, double Omegab) {
    Eigen::Matrix<double, 2, 1> velocities(Vb, Omegab);
    
    // 1. Compute linear wheel velocities (m/s) via matrix multiplication
    Eigen::Matrix<double, 2, 1> linear_wheel_velocities = this->M_inv_ * velocities;
    
    // 2. Convert to angular velocities (rad/s) using element-wise division
    Eigen::Matrix<double, 2, 1> angular_wheel_velocities = linear_wheel_velocities / this->wheel_radius_;
    
    return angular_wheel_velocities;
}

void DiffDriveKinematicsMatrixMethodClass::callback_joint_states(const JointState::SharedPtr msg) {
    rclcpp::Time current_time = msg->header.stamp;

    double left_wheel_position = 0.0;
    double right_wheel_position = 0.0;

    for (size_t i = 0; i < msg->name.size(); ++i) {
        if (msg->name[i] == "wheel_left_joint") {
        left_wheel_position = msg->position[i];
        } else if (msg->name[i] == "wheel_right_joint") {
        right_wheel_position = msg->position[i];
        }
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

    double left_wheel_vel = (left_wheel_position - this->last_left_wheel_pos_.value()) / dt;
    double right_wheel_vel = (right_wheel_position - this->last_right_wheel_pos_.value()) / dt;

    this->last_left_wheel_pos_ = left_wheel_position;
    this->last_right_wheel_pos_ = right_wheel_position;

    // Run the Forward Kinematics Matrix mapping loop step
    Eigen::Matrix<double, 3, 1> chassis_velocities = this->forward_kinematics(left_wheel_vel, right_wheel_vel);

    double x_dot = chassis_velocities(0);
    double y_dot = chassis_velocities(1);
    double theta_dot = chassis_velocities(2);

    // Position update accumulation
    this->x_ += x_dot * dt;
    this->y_ += y_dot * dt;
    this->theta_ += theta_dot * dt;

    this->theta_ = std::atan2(std::sin(this->theta_), std::cos(this->theta_));

    DiffDriveKinematicsMatrixMethodClass::RobotVelocities local_vel;
    local_vel.linear = (this->wheel_radius_ / 2.0) * (left_wheel_vel + right_wheel_vel);
    local_vel.angular = theta_dot; 

    this->publish_odom_tf(local_vel, current_time);
}

Eigen::Matrix<double, 3, 1> DiffDriveKinematicsMatrixMethodClass::forward_kinematics(double left_wheel_vel, double right_wheel_vel) {
    Eigen::Matrix<double, 3, 2> jacobian_matrix;
    jacobian_matrix << (this->wheel_radius_ / 2.0) * std::cos(this->theta_), (this->wheel_radius_ / 2.0) * std::cos(this->theta_),
                        (this->wheel_radius_ / 2.0) * std::sin(this->theta_), (this->wheel_radius_ / 2.0) * std::sin(this->theta_),
                        -this->wheel_radius_ / this->wheel_separation_,        this->wheel_radius_ / this->wheel_separation_;

    Eigen::Matrix<double, 2, 1> wheel_velocities(left_wheel_vel, right_wheel_vel);
    Eigen::Matrix<double, 3, 1> chassis_velocities = jacobian_matrix * wheel_velocities;
    return chassis_velocities;
}

void DiffDriveKinematicsMatrixMethodClass::publish_odom_tf(const DiffDriveKinematicsMatrixMethodClass::RobotVelocities &velocities, const rclcpp::Time &sim_time) {
    // Best Practice: Use TF2 primitives to format standard ROS 2 transport targets cleanly
    tf2::Quaternion q;
    q.setRPY(0.0, 0.0, this->theta_);

    auto odom_msg = Odometry();
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

    auto transform = geometry_msgs::msg::TransformStamped();
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

int main(int argc, char *argv[]) {
    auto log = rclcpp::get_logger("System");
    DiffDriveKinematicsMatrixMethodClass::SharedPtr node_instance = nullptr;

    try {
        RCLCPP_INFO(log, "Initializing the ROS 2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting the ROS 2 Node...");
        node_instance = std::make_shared<DiffDriveKinematicsMatrixMethodClass>();
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