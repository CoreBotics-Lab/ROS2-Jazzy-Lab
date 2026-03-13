#include "rclcpp/rclcpp.hpp"
#include "ros2_interfaces/msg/string.hpp"

using namespace std::chrono_literals;
using String = ros2_interfaces::msg::String;

class Counter_publisher_node_class : public rclcpp::Node{
    public:
        Counter_publisher_node_class() : Node("counter_publisher"), cnt_(0){
            RCLCPP_INFO(this->get_logger(), "%s has been started", this->get_name());
            publisher_ = this->create_publisher<String>("/counter", 10);
            timer_ = this->create_timer(500ms,
                    [this]() -> void{this->callback_timer();}
            );
            msg = std::make_shared<String>();
        }

    private:
        int cnt_;
        rclcpp::Publisher<String>::SharedPtr publisher_;
        rclcpp::TimerBase::SharedPtr timer_;
        String::SharedPtr msg;

        void callback_timer(){
            msg->data = "Counter: " + std::to_string(this->cnt_);
            this->publisher_->publish(*msg);
            RCLCPP_INFO(this->get_logger(), "%s", this->msg->data.c_str());
            this->cnt_++;
        }
};


int main(int argc, char * argv[]){
    auto log = rclcpp::get_logger("System");
    try{
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting a ROS2 Node...");
        auto node_instance = std::make_shared<Counter_publisher_node_class>();
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