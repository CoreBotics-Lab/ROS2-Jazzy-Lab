#ifndef DIFF_DRIVE_KINEMATICS_HPP
#define DIFF_DRIVE_KINEMATICS_HPP

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <std_msgs/msg/float64_multi_array.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <memory>
#include <optional>

class DiffDriveKinematicsClass : public rclcpp::Node
{
public:
    // Only the constructor signature is declared here
    DiffDriveKinematicsClass();

private:
    // Physical Parameters
    double wheel_radius_;
    double wheel_separation_;
    
    // Odometry State
    double x_;
    double y_;
    double theta_;
    
    std::optional<double> last_left_wheel_pos_;
    std::optional<double> last_right_wheel_pos_;
    rclcpp::Time prev_time_;
    
    // Communication Interfaces
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_subscriber_;
    rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr joint_state_subscriber_;
    rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr wheel_speed_publisher_;
    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_publisher_;
    
    std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
    
    // Struct to group velocities and prevent parameter swapping
    struct RobotVelocities {
        double linear;
        double angular;
    };

    // Callbacks
    void callback_cmd_vel(const geometry_msgs::msg::Twist::SharedPtr msg);
    void callback_joint_states(const sensor_msgs::msg::JointState::SharedPtr msg);
    void publish_odom_tf(const RobotVelocities& velocities, const rclcpp::Time& sim_time);
};

#endif // DIFF_DRIVE_KINEMATICS_HPP