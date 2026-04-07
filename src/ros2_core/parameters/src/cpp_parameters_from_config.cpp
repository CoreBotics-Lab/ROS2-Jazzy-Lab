#include "rclcpp/rclcpp.hpp"
#include "rcl_interfaces/msg/set_parameters_result.hpp"
#include <string>

using namespace std::chrono_literals;

class ParameterFromConfigNode_Class : public rclcpp::Node{
    public:
        ParameterFromConfigNode_Class() : Node("parameter_from_config_node"){
            RCLCPP_INFO(this->get_logger(), "%s has been started!", this->get_name());

            // 1. Declare Parameters WITHOUT default values
            // This forces the node to depend on an external configuration file.
            rcl_interfaces::msg::ParameterDescriptor desc;
            desc.dynamic_typing = true;

            this->declare_parameter("robot_mode", rclcpp::ParameterValue(), desc);
            this->declare_parameter("max_velocity", rclcpp::ParameterValue(), desc);
            this->declare_parameter("publish_rate_ms", rclcpp::ParameterValue(), desc);

            // 2. Retrieve parameters as rclcpp::Parameter objects first
            auto mode_param = this->get_parameter("robot_mode");
            auto velocity_param = this->get_parameter("max_velocity");
            auto rate_param = this->get_parameter("publish_rate_ms");

            // 3. Validate that parameters were loaded.
            if (mode_param.get_type() == rclcpp::ParameterType::PARAMETER_NOT_SET ||
                velocity_param.get_type() == rclcpp::ParameterType::PARAMETER_NOT_SET ||
                rate_param.get_type() == rclcpp::ParameterType::PARAMETER_NOT_SET) {
                throw std::runtime_error("A required parameter was not set! Please load a config file.");
            }

            // Now it is safe to assign them to our class variables
            mode = mode_param.as_string();
            velocity = velocity_param.as_double();
            rate = rate_param.as_int();

            RCLCPP_INFO(this->get_logger(), "Successfully loaded parameters from config file:");
            RCLCPP_INFO(this->get_logger(), "-> robot_mode: %s", mode.c_str());
            RCLCPP_INFO(this->get_logger(), "-> max_velocity: %f", velocity);
            RCLCPP_INFO(this->get_logger(), "-> publish_rate_ms: %ld ms", rate);
        }

    private:
        std::string mode;
        double velocity;
        int64_t rate;
};


int main(int argc, char * argv[]){
    auto log = rclcpp::get_logger("System");
    ParameterFromConfigNode_Class::SharedPtr node_instance = nullptr;

    try{
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting a ROS2 Node...");
        node_instance = std::make_shared<ParameterFromConfigNode_Class>();
        // This blocks until Ctrl+C is pressed
        rclcpp::spin(node_instance);

        // This runs AFTER you press Ctrl+C
        RCLCPP_WARN(log, "[CTRL+C]>>> Interrupted by the User.");
        RCLCPP_INFO(log, "Destroying the ROS2 Node...");
    }
    catch(std::exception & e){
        RCLCPP_ERROR(log, "Critical Error: %s", e.what());
    }

    if(rclcpp::ok()){
        RCLCPP_INFO(log, "Manually shutting down the ROS2 Client...");
        rclcpp::shutdown();
    }

    return 0;
}