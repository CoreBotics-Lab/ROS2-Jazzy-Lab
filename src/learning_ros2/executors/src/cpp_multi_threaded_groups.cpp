#include "rclcpp/rclcpp.hpp"
#include <memory>

using namespace std::chrono_literals;

class MultiGroupDemoNode : public rclcpp::Node{
    public:
        MultiGroupDemoNode() : Node("multi_group_demo_node") {
            RCLCPP_INFO(this->get_logger(), "%s has been started!", this->get_name());
            this->group_a_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
            this->group_b_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

            this->timer_1_ = this->create_timer(
                500ms,
                [this]() -> void{this->timer_1_callback();},
                this->group_a_
            );
            this->timer_2_ = this->create_timer(
                1000ms,
                [this]() -> void{this->timer_2_callback();},
                this->group_a_
            );
            this->timer_3_ = this->create_timer(
                2000ms,
                [this]() -> void{this->timer_3_callback();},
                this->group_b_
            );

        }

    private:
        rclcpp::CallbackGroup::SharedPtr group_a_;
        rclcpp::CallbackGroup::SharedPtr group_b_;

        rclcpp::TimerBase::SharedPtr timer_1_;
        rclcpp::TimerBase::SharedPtr timer_2_;
        rclcpp::TimerBase::SharedPtr timer_3_;

        void timer_1_callback(){
            std::stringstream ss;
            ss << std::this_thread::get_id();
            RCLCPP_INFO(this->get_logger(), "[TIMER 1 - Grp A] Tick! (Thread: %s)", ss.str().c_str());
        }

        void timer_2_callback(){
            std::stringstream ss;
            ss << std::this_thread::get_id();
            RCLCPP_WARN(this->get_logger(), "[TIMER 2 - Grp A] Starting 2.0s sleep on Thread: %s...", ss.str().c_str());
            std::this_thread::sleep_for(2000ms);
            RCLCPP_WARN(this->get_logger(), "[TIMER 2 - Grp A] Woke up and finished on Thread: %s!", ss.str().c_str());
        }

        void timer_3_callback(){
            std::stringstream ss;
            ss << std::this_thread::get_id();
            RCLCPP_ERROR(this->get_logger(), "[TIMER 3 - Grp B] Starting 5.0s sleep on Thread: %s...", ss.str().c_str());
            std::this_thread::sleep_for(5000ms);
            RCLCPP_ERROR(this->get_logger(), "[TIMER 3 - Grp B] Woke up and finished on Thread: %s!", ss.str().c_str());
        }


};

int main(int argc, char * argv[]) {
    auto log = rclcpp::get_logger("System");
    std::shared_ptr<MultiGroupDemoNode> node_instance = nullptr;

    try {
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting Multi-Group Executor Demo...");
        node_instance = std::make_shared<MultiGroupDemoNode>();
        
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