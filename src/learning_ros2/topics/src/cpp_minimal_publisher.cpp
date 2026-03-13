#include "rclcpp/rclcpp.hpp"


using namespace std::chrono_literals;

class Counter_publisher_node_class : public rclcpp::Node{
    public:
        Counter_publisher_node_class() : Node("counter_publisher"){
            RCLCPP_INFO(this->get_logger(), "%s has been started", this->get_name());

        }

    private:

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