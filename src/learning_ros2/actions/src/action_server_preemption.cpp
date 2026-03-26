#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "ros2_interfaces/action/counter.hpp"
#include <thread>
#include <sstream>
#include <mutex>

using Executors = rclcpp::executors::MultiThreadedExecutor;
using CounterAction = ros2_interfaces::action::Counter;
using GoalHandle = rclcpp_action::ServerGoalHandle<CounterAction>;

class action_server_node_class : public rclcpp::Node{
    public:
        action_server_node_class() : Node("counter_action_server_preemptive"){
            RCLCPP_INFO(this->get_logger(), "%s has been started", this->get_name());

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
                            active_goal_handle_->abort(std::make_shared<CounterAction::Result>());
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

        // STEP 2: The "Gatekeeper" for new goals.
        // This function validates incoming goals and decides whether to accept or reject them.
        rclcpp_action::GoalResponse goal_callback(
            const rclcpp_action::GoalUUID & uuid,
            std::shared_ptr<const CounterAction::Goal> goal)
        {
            (void)uuid;
            RCLCPP_INFO(this->get_logger(), "Received goal request to count to %d", goal->target_number);

            // Perform validation.
            if (goal->target_number < 0) {
                RCLCPP_WARN(this->get_logger(), "Rejecting goal: target_number cannot be negative.");
                return rclcpp_action::GoalResponse::REJECT; // Reject the goal.
            }
            // If validation passes, accept the goal and start executing it.
            return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
        }

        // STEP 3: The "Gatekeeper" for cancellation.
        // This function approves or denies cancellation requests.
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
            // These messages are created as local variables to ensure that each goal execution
            // has its own independent messages, making the process thread-safe.
            auto feedback = std::make_shared<CounterAction::Feedback>();
            auto result = std::make_shared<CounterAction::Result>();

            for (int i = 0; (i <= goal->target_number) && rclcpp::ok(); ++i) {
                // First, check if the goal is still active. If not, it means it was
                // aborted by the server (due to a new goal arriving). In this case,
                // the state is already terminal, so we just need to stop this thread.
                if (!goal_handle->is_active()) {
                    RCLCPP_INFO(this->get_logger(), "Goal aborted by server, stopping execution thread.");
                    return;
                }

                // The Cooperative Cancellation Model: Periodically check if cancellation was requested.
                // This handles cancellation requests from the client.
                if (goal_handle->is_canceling()) {
                    result->final_sequence = feedback->current_sequence;
                    goal_handle->canceled(result);
                    RCLCPP_INFO(this->get_logger(), "Goal canceled");
                    return; // Exit the execute function immediately.
                }

                feedback->current_sequence.push_back(i);
                goal_handle->publish_feedback(feedback);
                RCLCPP_INFO(this->get_logger(), "Publishing feedback: Current Value is %d", i);

                rate.sleep();
            }

            // If the loop completes without cancellation, the goal has succeeded.
            if (rclcpp::ok()) {
                result->final_sequence = feedback->current_sequence;
                goal_handle->succeed(result);
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
        // Manually shut down the ROS 2 client library.
        RCLCPP_INFO(log, "Manually shutting down the ROS2 client...");
        rclcpp::shutdown();
    }

    return 0;
}