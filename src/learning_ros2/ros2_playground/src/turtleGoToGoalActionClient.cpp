#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "ros2_interfaces/action/go_to_goal.hpp"
#include "action_msgs/msg/goal_status.hpp"

#include <memory>
#include <string>
#include <thread>

using GoToGoalAction = ros2_interfaces::action::GoToGoal;
using GoalHandleGoToGoal = rclcpp_action::ClientGoalHandle<GoToGoalAction>;

class GoToGoalTurtleActionClient : public rclcpp::Node
{
public:
    GoToGoalTurtleActionClient()
    : Node("go_to_goal_turtle_action_client")
    {
        this->goal_done_ = false;
        RCLCPP_INFO(this->get_logger(), "%s has been started.", this->get_name());

        this->action_client_ = rclcpp_action::create_client<GoToGoalAction>(
            this,
            "go_to_goal"
        );
    }

    bool is_goal_done() const {
        return this->goal_done_;
    }

    void send_goal(double x, double y, double theta)
    {
        this->goal_done_ = false;

        if (!this->action_client_->wait_for_action_server(std::chrono::seconds(10))) {
            RCLCPP_ERROR(this->get_logger(), "Action server not available after waiting.");
            this->goal_done_ = true;
            return;
        }

        auto goal_msg = std::make_shared<GoToGoalAction::Goal>();
        goal_msg->x = x;
        goal_msg->y = y;
        goal_msg->theta = theta;

        RCLCPP_INFO(this->get_logger(), "Sending goal request: (x:%.2f, y:%.2f, theta:%.2f)", goal_msg->x, goal_msg->y, goal_msg->theta);

        auto send_goal_options = std::make_shared<rclcpp_action::Client<GoToGoalAction>::SendGoalOptions>();

        send_goal_options->goal_response_callback = 
            [this](std::shared_ptr<GoalHandleGoToGoal> goal_handle) {
                this->goal_response_callback(goal_handle);
            };

        send_goal_options->feedback_callback = 
            [this](std::shared_ptr<GoalHandleGoToGoal> goal_handle, const std::shared_ptr<const GoToGoalAction::Feedback> feedback) {
                this->feedback_callback(goal_handle, feedback);
            };

        // We assign the result callback directly into the SendGoalOptions struct
        // rather than using async_get_result, which is the most modern ROS 2 C++ practice.
        send_goal_options->result_callback = 
            [this](const GoalHandleGoToGoal::WrappedResult & result) {
                this->result_callback(result);
            };

        // The ROS 2 API requires const references for these arguments, so we dereference the smart pointers
        this->action_client_->async_send_goal(*goal_msg, *send_goal_options);
    }

    void cancel_goal()
    {
        if (this->goal_handle_) {
            auto current_status = std::make_shared<int8_t>(this->goal_handle_->get_status());
            
            if (*current_status == action_msgs::msg::GoalStatus::STATUS_ACCEPTED ||
                *current_status == action_msgs::msg::GoalStatus::STATUS_EXECUTING)
            {
                RCLCPP_WARN(this->get_logger(), "Canceling goal...");
                this->action_client_->async_cancel_goal(this->goal_handle_);
            } else {
                RCLCPP_INFO(this->get_logger(), "Goal already finished or not cancellable.");
            }
        } else {
            RCLCPP_INFO(this->get_logger(), "No active goal to cancel.");
        }
    }

private:
    rclcpp_action::Client<GoToGoalAction>::SharedPtr action_client_;
    bool goal_done_;
    std::shared_ptr<GoalHandleGoToGoal> goal_handle_;

    void goal_response_callback(std::shared_ptr<GoalHandleGoToGoal> goal_handle)
    {
        if (!goal_handle) { 
            RCLCPP_ERROR(this->get_logger(), "Goal was rejected by server.");
            this->goal_done_ = true;
            return;
        }

        RCLCPP_INFO(this->get_logger(), "Goal accepted by server.");
        this->goal_handle_ = goal_handle;
    }

    void feedback_callback(
        std::shared_ptr<GoalHandleGoToGoal> /*goal_handle*/,
        const std::shared_ptr<const GoToGoalAction::Feedback> feedback)
    {
        RCLCPP_INFO(this->get_logger(),
            "Feedback: At (x:%.2f, y:%.2f), distance to goal: %.2f",
            feedback->current_x, feedback->current_y, feedback->distance_remaining);
    }

    void result_callback(const GoalHandleGoToGoal::WrappedResult & result)
    {
        auto result_ptr = std::make_shared<GoalHandleGoToGoal::WrappedResult>(result);

        switch (result_ptr->code) {
            case rclcpp_action::ResultCode::SUCCEEDED:
                RCLCPP_INFO(this->get_logger(), "Goal succeeded!");
                RCLCPP_INFO(this->get_logger(), "Result: %s", result_ptr->result->message.c_str());
                break;
            case rclcpp_action::ResultCode::CANCELED:
                RCLCPP_INFO(this->get_logger(), "Goal was canceled.");
                break;
            case rclcpp_action::ResultCode::ABORTED:
                RCLCPP_ERROR(this->get_logger(), "Goal was aborted by the server: %s", result_ptr->result->message.c_str());
                break;
            default:
                RCLCPP_WARN(this->get_logger(), "Unknown result code: %d", static_cast<int>(result_ptr->code));
                break;
        }
        this->goal_done_ = true; 
    }
};

int main(int argc, char * argv[])
{
    auto log = rclcpp::get_logger("System");
    std::shared_ptr<std::thread> spin_thread_ptr = nullptr;
    std::shared_ptr<GoToGoalTurtleActionClient> action_client_node = nullptr;
    // GoToGoalTurtleActionClient::SharedPtr action_client_node = nullptr;

    if (argc != 4) {
        RCLCPP_ERROR(log, "Incorrect number of arguments. Usage: ros2 run <pkg> <node> x y theta");
        return 1;
    }

    try {
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting the Action Client Node...");
        action_client_node = std::make_shared<GoToGoalTurtleActionClient>();
        
        // CRITICAL FIX: Run the executor in a separate background thread.
        // This prevents Best-Effort feedback packets from being dropped while sleeping,
        // and drastically speeds up the initial Action Server discovery!
        spin_thread_ptr = std::make_shared<std::thread>([action_client_node]() {
            rclcpp::spin(action_client_node);
        });

        action_client_node->send_goal(std::stod(argv[1]), std::stod(argv[2]), std::stod(argv[3]));

        while (rclcpp::ok() && !(action_client_node->is_goal_done())) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (!(action_client_node->is_goal_done())) {
            RCLCPP_WARN(log, "[CTRL+C]>>> Interrupted by the User. Canceling goal...");
            action_client_node->cancel_goal();
            
            auto timeout = std::chrono::steady_clock::now() + std::chrono::seconds(2);
            while (!(action_client_node->is_goal_done()) && std::chrono::steady_clock::now() < timeout) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

    } catch (const std::exception & e) {
        RCLCPP_ERROR(log, "Critical Error: %s", e.what());
    }

    if (rclcpp::ok()) {
        RCLCPP_INFO(log, "Manually shutting down the ROS2 client...");
        rclcpp::shutdown(); // Safely unblocks the background rclcpp::spin()
    }

    // Join the background thread before exiting to prevent memory leaks
    if (spin_thread_ptr && spin_thread_ptr->joinable()) {
        spin_thread_ptr->join();
    }
    return 0;
}