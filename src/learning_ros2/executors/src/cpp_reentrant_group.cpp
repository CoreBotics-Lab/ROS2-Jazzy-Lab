#include "rclcpp/rclcpp.hpp"
#include <memory>

using namespace std::chrono_literals;

class ReentrantDemoNode : public rclcpp::Node{
    public:
        ReentrantDemoNode() : Node("reentrant_demo_node"), counter_(0) {
            RCLCPP_INFO(this->get_logger(), "%s has been started!", this->get_name());
            this->reentrant_group_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);
            this->timer_ = this->create_timer(
                500ms,
                [this]() -> void{this->timer_callback();},
                this->reentrant_group_
            );
        }

    private:
        int counter_;
        rclcpp::CallbackGroup::SharedPtr reentrant_group_;
        rclcpp::TimerBase::SharedPtr timer_;
        std::mutex mutex_;

        void timer_callback(){
            std::stringstream ss;
            ss << std::this_thread::get_id();
            RCLCPP_INFO(this->get_logger(), "[ENTER] Timer triggered on Thread: %s", ss.str().c_str());

            // Increment the counter in a thread-safe manner
            {
                std::lock_guard<std::mutex> lock(this->mutex_);
                this->counter_++;
                RCLCPP_INFO(this->get_logger(), "[%s] Safely incremented counter to: %d", ss.str().c_str(), this->counter_);
            }

            RCLCPP_WARN(this->get_logger(), "[%s] Starting heavy 2.0s work...", ss.str().c_str());
            std::this_thread::sleep_for(2.0s);
            RCLCPP_WARN(this->get_logger(), "[EXIT] Finished work on Thread: %s", ss.str().c_str());
        }
};

int main(int argc, char * argv[]) {
    auto log = rclcpp::get_logger("System");
    std::shared_ptr<ReentrantDemoNode> node_instance = nullptr;

    try {
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting Reentrant Executor Demo...");
        node_instance = std::make_shared<ReentrantDemoNode>();
        
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