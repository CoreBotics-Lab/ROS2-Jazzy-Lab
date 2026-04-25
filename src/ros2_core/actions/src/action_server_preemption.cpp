#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "ros2_interfaces/action/counter.hpp"
#include <thread>
#include <mutex>

using Executors = rclcpp::executors::MultiThreadedExecutor;
using CounterAction = ros2_interfaces::action::Counter;
using GoalHandle = rclcpp_action::ServerGoalHandle<CounterAction>;

class action_server_node_class : public rclcpp::Node{
    public:
        action_server_node_class() : Node("counter_action_server_preemptive"){
            RCLCPP_INFO(this->get_logger(), "%s has been started", this->get_name());

            // Pre-allocate messages to ensure flat RAM overhead
            feedback_ = std::make_shared<CounterAction::Feedback>();
            result_ = std::make_shared<CounterAction::Result>();

            // STEP 1: Create the action server, defining the three core callbacks.
            this->action_server_ = rclcpp_action::create_server<CounterAction>(
                this,
                "counter",

                // Callback for validating and accepting new goal requests.
                [this](const rclcpp_action::GoalUUID & uuid, std::shared_ptr<const CounterAction::Goal> goal) {
                    return this->goal_callback(uuid, goal);
                },
                // Callback for handling cancellation requests.
                [this](const std::shared_ptr<GoalHandle> goal_handle) {
                    return this->cancel_callback(goal_handle);
                },
                // This callback is the key to enforcing a single-goal policy.
                [this](const std::shared_ptr<GoalHandle> goal_handle) {
                    // Abort any existing goal before starting the new one.
                    {
                        std::lock_guard<std::mutex> lock(active_goal_mutex_);
                        if (active_goal_handle_ && active_goal_handle_->is_active()) {
                            RCLCPP_INFO(this->get_logger(), "New goal received, aborting previous active goal.");
                            result_->final_sequence.clear();
                            active_goal_handle_->abort(result_);
                        }
                        active_goal_handle_ = goal_handle;
                    }
                    // Start the execution in a new thread as before.
                    std::thread([this, goal_handle]() {
                        this->execute_callback(goal_handle);
                    }).detach();
                });
        }


    private:
        rclcpp_action::Server<CounterAction>::SharedPtr action_server_;
        std::mutex active_goal_mutex_;
        std::shared_ptr<GoalHandle> active_goal_handle_;

        // Pre-allocated messages
        CounterAction::Feedback::SharedPtr feedback_;
        CounterAction::Result::SharedPtr result_;

        // STEP 2: The "Gatekeeper" for new goals.
        rclcpp_action::GoalResponse goal_callback(
            const rclcpp_action::GoalUUID & uuid,
            std::shared_ptr<const CounterAction::Goal> goal)
        {
            (void)uuid;
            RCLCPP_INFO(this->get_logger(), "Received goal request to count to %d", goal->target_number);

            if (goal->target_number < 0) {
                RCLCPP_WARN(this->get_logger(), "Rejecting goal: target_number cannot be negative.");
                return rclcpp_action::GoalResponse::REJECT;
            }
            return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
        }

        // STEP 3: The "Gatekeeper" for cancellation.
        rclcpp_action::CancelResponse cancel_callback(
            const std::shared_ptr<GoalHandle> goal_handle)
        {
            (void)goal_handle;
            RCLCPP_INFO(this->get_logger(), "Received request to cancel goal");
            return rclcpp_action::CancelResponse::ACCEPT;
        }

        // It runs in a separate thread for each accepted goal.
        void execute_callback(const std::shared_ptr<GoalHandle> goal_handle){
            RCLCPP_INFO(this->get_logger(), "Executing goal...");
            rclcpp::Rate rate(5.0);

            const auto goal = goal_handle->get_goal();
            
            // Reset pre-allocated messages for the new goal
            feedback_->current_sequence.clear();
            result_->final_sequence.clear();

            for (int i = 0; (i <= goal->target_number) && rclcpp::ok(); ++i) {
                if (!goal_handle->is_active()) {
                    RCLCPP_INFO(this->get_logger(), "Goal aborted by server, stopping execution thread.");
                    return;
                }

                if (goal_handle->is_canceling()) {
                    result_->final_sequence = feedback_->current_sequence;
                    goal_handle->canceled(result_);
                    RCLCPP_INFO(this->get_logger(), "Goal canceled");
                    return;
                }

                feedback_->current_sequence.push_back(i);
                goal_handle->publish_feedback(feedback_);
                RCLCPP_INFO(this->get_logger(), "Publishing feedback: Current Value is %d", i);

                rate.sleep();
            }

            if (rclcpp::ok()) {
                result_->final_sequence = feedback_->current_sequence;
                goal_handle->succeed(result_);
                RCLCPP_INFO(this->get_logger(), "Goal succeeded");
            }
        }
};

int main(int argc, char * argv[]){
    auto log = rclcpp::get_logger("System");
    action_server_node_class::SharedPtr node_instance = nullptr;
    Executors::SharedPtr executors = nullptr;
    try{
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting a ROS2 Node...");
        node_instance = std::make_shared<action_server_node_class>();
        executors = std::make_shared<Executors>();
        executors->add_node(node_instance);
        executors->spin();

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