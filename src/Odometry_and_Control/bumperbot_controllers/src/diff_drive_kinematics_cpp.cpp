#include "bumperbot_controller/diff_drive_kinematics.hpp"

using namespace std::chrono_literals;

using Twist = geometry_msgs::msg::Twist;
using Float64MultiArray = std_msgs::msg::Float64MultiArray;

// 1. Constructor Implementation
DiffDriveKinematicsClass::DiffDriveKinematicsClass() : Node("diff_drive_kinematics")
{
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
    
    // this->wheel_radius_ = 0.068 / 2.0;    // meters
    // this->wheel_separation_ = 0.174;      // meters
    
    // Logic Extraction: Using a lambda that only calls a private method
    this->cmd_vel_subscriber_ = this->create_subscription<Twist>(
        "/cmd_vel",
        10,
        [this](const Twist::SharedPtr msg) -> void {
            this->callback_cmd_vel(msg);
        });
        
    this->wheel_speed_publisher_ = this->create_publisher<Float64MultiArray>(
        "/velocity_controller/commands", 10);
        
    RCLCPP_INFO(this->get_logger(), "%s node has been successfully initialized.", this->get_name());
}

// 2. Callback Implementation
void DiffDriveKinematicsClass::callback_cmd_vel(const Twist::SharedPtr msg)
{   
    double v_linear = msg->linear.x;
    double v_angular = msg->angular.z;

    // Kinematics math for calculating individual wheel speeds from cmd_vel
    double left_wheel_speed = (v_linear - (v_angular * this->wheel_separation_ / 2.0)) / this->wheel_radius_;
    double right_wheel_speed = (v_linear + (v_angular * this->wheel_separation_ / 2.0)) / this->wheel_radius_;

    // EXPLICIT STEP 1: Allocate a brand-new heap box for this specific runtime cycle.
    auto wheel_speed_msg = std::make_shared<Float64MultiArray>();
    wheel_speed_msg->data.resize(2, 0.0);
    
    // EXPLICIT STEP 2: Populate the isolated heap container data
    wheel_speed_msg->data[0] = left_wheel_speed;
    wheel_speed_msg->data[1] = right_wheel_speed;
    
    // EXPLICIT STEP 3: Dereference with the asterisk (*) to match the publisher's standard const reference value signature.
    this->wheel_speed_publisher_->publish(*wheel_speed_msg);
} 

// 3. Main Execution Loop
int main(int argc, char * argv[])
{
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
    }
    catch(const std::exception & e) {
        RCLCPP_ERROR(log, "Critical Error: %s", e.what());
    }

    if(rclcpp::ok()) {
        RCLCPP_INFO(log, "Manually shutting down the ROS 2 client...");
        rclcpp::shutdown();
    }

    return 0;
}