#include "rclcpp/rclcpp.hpp"

using namespace std::chrono_literals;

class ParameterPollingNode_Class : public rclcpp::Node{
    public:
        ParameterPollingNode_Class() : Node("parameter_polling_node"){
            RCLCPP_INFO(this->get_logger(), "%s has been started!", this->get_name());

            // 1. Declare Parameters with default values
            this->declare_parameter<std::string>("robot_mode", "autonomous");
            this->declare_parameter<double>("max_velocity", 1.2);
            this->declare_parameter<int64_t>("publish_rate_ms", 500);

            // 2. Create a timer that will poll the parameters
            current_rate_ms_ = this->get_parameter("publish_rate_ms").as_int();
            timer_ = this->create_timer(
                std::chrono::milliseconds(current_rate_ms_),
                [this]() -> void{this->timer_callback();}
            );
            RCLCPP_INFO(this->get_logger(), "Node started. Polling parameters every %ld ms.", current_rate_ms_);
        }

    private:
        rclcpp::TimerBase::SharedPtr timer_;
        int64_t current_rate_ms_;

        void timer_callback(){
            // Poll the rate parameter first to see if we need to update the timer itself
            auto new_rate_ms = this->get_parameter("publish_rate_ms").as_int();
            if (new_rate_ms != current_rate_ms_) {
                RCLCPP_INFO(this->get_logger(), "Polling rate changed to %ldms. Recreating timer.", new_rate_ms);
                timer_->cancel();
                current_rate_ms_ = new_rate_ms;
                timer_ = this->create_timer(
                    std::chrono::milliseconds(current_rate_ms_),
                    [this]() -> void { this->timer_callback(); }
                );
                // Return immediately to avoid running the rest of the callback with a stale timer
                return;
            }

            // Poll the other parameters for their values
            auto current_mode = this->get_parameter("robot_mode").as_string();
            auto current_velocity = this->get_parameter("max_velocity").as_double();

            RCLCPP_INFO(this->get_logger(), "Polling: Mode is [%s], Max Velocity is [%.2f]", current_mode.c_str(), current_velocity);
        }
};


int main(int argc, char * argv[]){
    auto log = rclcpp::get_logger("System");
    ParameterPollingNode_Class::SharedPtr node_instance = nullptr;

    try{
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting a ROS2 Node...");
        node_instance = std::make_shared<ParameterPollingNode_Class>();
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