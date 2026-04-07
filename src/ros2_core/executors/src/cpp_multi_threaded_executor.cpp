#include "rclcpp/rclcpp.hpp"
#include <memory>
#include <sstream>

using namespace std::chrono_literals;

class MultiThreadedDemoNode : public rclcpp::Node{
    public:
        MultiThreadedDemoNode() : Node("multi_threaded_demo_node") {
            RCLCPP_INFO(this->get_logger(), "%s has been started!", this->get_name());
            this->fast_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
            this->slow_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

            this->fast_timer_ = this->create_timer(
                500ms,
                [this]()->void{this->fast_timer_callback();},
                this->fast_group_);

            this->slow_timer_ = this->create_timer(
                2000ms,
                [this]()->void{this->slow_timer_callback();},
                this->slow_group_);

        }

    private:
        rclcpp::CallbackGroup::SharedPtr fast_group_;
        rclcpp::CallbackGroup::SharedPtr slow_group_;

        rclcpp::TimerBase::SharedPtr fast_timer_;
        rclcpp::TimerBase::SharedPtr slow_timer_;

        void fast_timer_callback(){
            std::stringstream ss;
            ss << std::this_thread::get_id();
            RCLCPP_INFO(this->get_logger(), "[FAST] Tick! (Running on thread: %s)", ss.str().c_str());
        }

        void slow_timer_callback(){
            std::stringstream ss;
            ss << std::this_thread::get_id();
            RCLCPP_WARN(this->get_logger(), "[SLOW] Danger: Starting 3-second blocking sleep on thread: %s", ss.str().c_str());
            std::this_thread::sleep_for(3000ms);
            RCLCPP_WARN(this->get_logger(), "[SLOW] Woke up and finished on thread: %s", ss.str().c_str());
        }
};

int main(int argc, char * argv[]) {
    auto log = rclcpp::get_logger("System");
    std::shared_ptr<MultiThreadedDemoNode> node_instance = nullptr;

    try {
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting a ROS2 Node...");
        node_instance = std::make_shared<MultiThreadedDemoNode>();
        
        // Explicitly defining the MultiThreadedExecutor
        rclcpp::executors::MultiThreadedExecutor executor;
        executor.add_node(node_instance);
        
        RCLCPP_INFO(log, "Spinning with MultiThreadedExecutor...");
        executor.spin();

        RCLCPP_WARN(log, "[CTRL+C] >>> Interrupted by the User.");
        RCLCPP_INFO(log, "Destroying the ROS2 Node...");
    }
    catch (const std::exception & e) {
        RCLCPP_ERROR(log, "Critical Error: %s", e.what());
    }

    if (rclcpp::ok()) {
        RCLCPP_INFO(log, "Manually shutting down the ROS2 Client...");
        rclcpp::shutdown();
    }
    return 0;
}