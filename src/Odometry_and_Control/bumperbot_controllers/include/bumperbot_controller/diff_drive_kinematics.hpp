#ifndef DIFF_DRIVE_KINEMATICS_HPP
#define DIFF_DRIVE_KINEMATICS_HPP

#include <rclcpp/rclcpp.hpp>
#include <geometry_msgs/msg/twist.hpp>
#include <std_msgs/msg/float64_multi_array.hpp>
#include <memory>

class DiffDriveKinematicsClass : public rclcpp::Node
{
public:
    // Only the constructor signature is declared here
    DiffDriveKinematicsClass();

private:
    // Physical Parameters
    double wheel_radius_;
    double wheel_separation_;
    
    // Communication Interfaces
    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_vel_subscriber_;
    rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr wheel_speed_publisher_;
    
    // Callback signature
    void callback_cmd_vel(const geometry_msgs::msg::Twist::SharedPtr msg);
};

#endif // DIFF_DRIVE_KINEMATICS_HPP