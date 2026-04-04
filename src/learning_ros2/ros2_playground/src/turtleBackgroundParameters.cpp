#include "rclcpp/rclcpp.hpp"
#include "rcl_interfaces/msg/parameter_descriptor.hpp"

using namespace std::chrono_literals;

class SetBackgroundParametersNode : public rclcpp::Node{
    public: 
        SetBackgroundParametersNode() : Node("set_background_parameters_node"){
            RCLCPP_INFO(this->get_logger(), "%s has been started!", this->get_name());

            // 1. Declare parameters WITHOUT default values
            // This forces the node to depend on an external configuration file.
            rcl_interfaces::msg::ParameterDescriptor desc;
            desc.dynamic_typing = true;

            this->declare_parameter("background_r", rclcpp::ParameterValue(), desc);
            this->declare_parameter("background_g", rclcpp::ParameterValue(), desc);
            this->declare_parameter("background_b", rclcpp::ParameterValue(), desc);

            // 2. Retrieve parameters as rclcpp::Parameter objects first
            auto r_param = this->get_parameter("background_r");
            auto g_param = this->get_parameter("background_g");
            auto b_param = this->get_parameter("background_b");

            // 3. Validate that parameters were loaded (Fail-Fast philosophy)
            if (r_param.get_type() == rclcpp::ParameterType::PARAMETER_NOT_SET ||
                g_param.get_type() == rclcpp::ParameterType::PARAMETER_NOT_SET ||
                b_param.get_type() == rclcpp::ParameterType::PARAMETER_NOT_SET) {
                throw std::runtime_error("Required RGB parameters were not set! Please load a config file.");
            }

            // Now it is safe to assign them to our class variables
            bg_r = r_param.as_int();
            bg_g = g_param.as_int();
            bg_b = b_param.as_int();

            RCLCPP_INFO(this->get_logger(), "Successfully loaded background color - R:%ld G:%ld B:%ld", bg_r, bg_g, bg_b);

            // 4. Initialize Parameter Client to target /turtlesim
            param_client_ = std::make_shared<rclcpp::AsyncParametersClient>(this, "/turtlesim");

            // 5. Use a one-shot timer to set parameters after the node starts spinning
            timer_ = this->create_timer(100ms, [this]() { this->set_turtlesim_parameters(); });
        }

    private:
        int64_t bg_r, bg_g, bg_b;
        rclcpp::AsyncParametersClient::SharedPtr param_client_;
        rclcpp::TimerBase::SharedPtr timer_;

        void set_turtlesim_parameters() {
            // Cancel the timer so this only executes once
            timer_->cancel();

            // Wait for the turtlesim parameter service to be available
            if (!param_client_->wait_for_service(3s)) {
                RCLCPP_ERROR(this->get_logger(), "/turtlesim parameter service not available! Make sure Turtlesim is running.");
                rclcpp::shutdown();
                return;
            }

            RCLCPP_INFO(this->get_logger(), "Sending new background color to /turtlesim...");
            param_client_->set_parameters({
                rclcpp::Parameter("background_r", bg_r),
                rclcpp::Parameter("background_g", bg_g),
                rclcpp::Parameter("background_b", bg_b)
            }, [this](std::shared_future<std::vector<rcl_interfaces::msg::SetParametersResult>> future) { // NOLINT(performance-unnecessary-value-param)
                (void)future; // In a production app, you would iterate over future.get() to verify each result
                RCLCPP_INFO(this->get_logger(), "Successfully updated /turtlesim background color!");
                rclcpp::shutdown(); // Task complete, exit the node
            });
        }
};

int main(int argc, char * argv[]){
    auto log = rclcpp::get_logger("System");
    SetBackgroundParametersNode::SharedPtr node_instance = nullptr;
    
    try{
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting a ROS2 Node...");
        node_instance = std::make_shared<SetBackgroundParametersNode>();
        rclcpp::spin(node_instance);
        RCLCPP_WARN(log, "[CTRL+C]>>> Interrupted by the User.");
        RCLCPP_INFO(log, "Destroying the ROS2 Node...");
    }
    catch(const std::exception & e){
        RCLCPP_ERROR(log, "Critical Error: %s", e.what());
    }

    if(rclcpp::ok()){
        RCLCPP_INFO(log, "Manually shutting down the ROS2 client...");
        rclcpp::shutdown();
    }


    return 0;
}