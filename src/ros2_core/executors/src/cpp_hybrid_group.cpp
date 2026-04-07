#include "rclcpp/rclcpp.hpp"
#include <memory>
#include <sstream>

using namespace std::chrono_literals;

class HybridDemoNode : public rclcpp::Node{
    public:
        HybridDemoNode() : Node("hybrid_demo_node"), counter_(0){
            RCLCPP_INFO(this->get_logger(), "%s has been started!", this->get_name());  
            this->exclusive_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
            this->reentrant_group_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);

            this->timer_1_ = this->create_timer(
                1000ms,
                [this]() -> void{this->timer1_callback();},
                this->exclusive_group_
            );
            this->timer_2_ = this->create_timer(
                1500ms,
                [this]() -> void{this->timer2_callback();},
                this->exclusive_group_
            );
            this->timer_3_ = this->create_timer(
                500ms,
                [this]() -> void{this->timer3_callback();},
                this->reentrant_group_
            );

        }

    private:
        int counter_;
        rclcpp::CallbackGroup::SharedPtr exclusive_group_;
        rclcpp::CallbackGroup::SharedPtr reentrant_group_;
        rclcpp::TimerBase::SharedPtr timer_1_;
        rclcpp::TimerBase::SharedPtr timer_2_;
        rclcpp::TimerBase::SharedPtr timer_3_;
        std::mutex mutex_;

        void timer1_callback(){
            std::stringstream ss;
            ss << std::this_thread::get_id();
            RCLCPP_INFO(this->get_logger(), "[TIMER 1] (Exclusive) Tick on Thread: %s", ss.str().c_str());
            std::this_thread::sleep_for(100ms); // Simulate a long-running task
        }

        void timer2_callback(){
            std::stringstream ss;
            ss << std::this_thread::get_id();
            RCLCPP_INFO(this->get_logger(), "[TIMER 2] (Exclusive) Tick on Thread: %s", ss.str().c_str());
            std::this_thread::sleep_for(100ms); // Simulate a long-running task
        }

        void timer3_callback(){
            std::stringstream ss;
            ss << std::this_thread::get_id();
            RCLCPP_WARN(this->get_logger(), "[TIMER 3] [ENTER] Reentrant Tick on Thread: %s", ss.str().c_str());
            {
                std::lock_guard<std::mutex> lock(this->mutex_);
                this->counter_++;   
                RCLCPP_INFO(this->get_logger(), "Thread: %s Safely incremented counter to: %d", ss.str().c_str(), this->counter_);
            }

            std::this_thread::sleep_for(2000ms); // Simulate a long-running task
            RCLCPP_WARN(this->get_logger(), "[TIMER 3] [EXIT] Finished work on Thread: %s", ss.str().c_str());
        }

};

int main(int argc, char * argv[]) {
    auto log = rclcpp::get_logger("System");
    std::shared_ptr<HybridDemoNode> node_instance = nullptr;

    try {
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting a ROS2 Node...");
        node_instance = std::make_shared<HybridDemoNode>();
        
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