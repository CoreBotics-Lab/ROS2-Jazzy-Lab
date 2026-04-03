#include "rclcpp/rclcpp.hpp"
#include "rcl_interfaces/msg/set_parameters_result.hpp"

using namespace std::chrono_literals;

class ParameterEventDrivenNode_Class : public rclcpp::Node{
    public:
        ParameterEventDrivenNode_Class() : Node("parameter_event_driven_node"){
            RCLCPP_INFO(this->get_logger(), "%s has been started!", this->get_name());

            // 1. Declare Parameters (Mandatory in modern ROS 2)
            this->declare_parameter<std::string>("robot_mode", "autonomous");
            this->declare_parameter<double>("max_velocity", 1.2);
            this->declare_parameter<int64_t>("publish_rate_ms", 500);

            // 2. Retrieve Initial Values
            current_mode_ = this->get_parameter("robot_mode").as_string();
            current_velocity_ = this->get_parameter("max_velocity").as_double();
            current_rate_ms_ = this->get_parameter("publish_rate_ms").as_int();

            RCLCPP_INFO(this->get_logger(), 
                "Node started with mode: '%s', max_velocity: %.2f", 
                current_mode_.c_str(), current_velocity_);

            // 3. Set up Dynamic Parameter Handling
            // We must store this handle, otherwise the callback gets destroyed immediately
            param_subscriber_ = this->add_on_set_parameters_callback(
                [this](const std::vector<rclcpp::Parameter> & params) -> rcl_interfaces::msg::SetParametersResult {
                    return this->parameter_callback(params);
                }
            );

            // 4. Timer to demonstrate usage
            timer_ = this->create_timer(
                std::chrono::milliseconds(current_rate_ms_),
                [this]() -> void{this->timer_callback();}
            );
        }

    private:
        rclcpp::TimerBase::SharedPtr timer_;
        rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr param_subscriber_;
        
        std::string current_mode_;
        double current_velocity_;
        int64_t current_rate_ms_;

        rcl_interfaces::msg::SetParametersResult parameter_callback(const std::vector<rclcpp::Parameter> & params){
            rcl_interfaces::msg::SetParametersResult result;
            result.successful = true;

            for (const auto & param : params) {
                if (param.get_name() == "max_velocity" && param.get_type() == rclcpp::ParameterType::PARAMETER_DOUBLE) {
                    current_velocity_ = param.as_double();
                    RCLCPP_INFO(this->get_logger(), "max_velocity changed dynamically to: %.2f", current_velocity_);
                } else if (param.get_name() == "robot_mode" && param.get_type() == rclcpp::ParameterType::PARAMETER_STRING) {
                    current_mode_ = param.as_string();
                    RCLCPP_INFO(this->get_logger(), "robot_mode changed dynamically to: %s", current_mode_.c_str());
                } else if (param.get_name() == "publish_rate_ms" && param.get_type() == rclcpp::ParameterType::PARAMETER_INTEGER) {
                    current_rate_ms_ = param.as_int();
                    RCLCPP_INFO(this->get_logger(), "publish_rate_ms changed dynamically. Updating timer to %ld ms.", current_rate_ms_);
                    timer_->cancel();
                    timer_ = this->create_timer(
                        std::chrono::milliseconds(current_rate_ms_),
                        [this]() -> void { this->timer_callback(); }
                    );
                }
            }
            // We must return a SetParametersResult to tell ROS 2 the change was accepted
            return result;
        }

        void timer_callback(){
            RCLCPP_INFO(this->get_logger(), "Event-Driven: Mode is [%s], Max Velocity is [%.2f]", current_mode_.c_str(), current_velocity_);
        }
};

int main(int argc, char * argv[]){
    auto log = rclcpp::get_logger("System");
    ParameterEventDrivenNode_Class::SharedPtr node_instance = nullptr;

    try{
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting a ROS2 Node...");
        node_instance = std::make_shared<ParameterEventDrivenNode_Class>();
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