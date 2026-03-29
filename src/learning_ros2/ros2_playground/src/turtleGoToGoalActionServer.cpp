#include "rclcpp/rclcpp.hpp"
#include "turtlesim/msg/pose.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "ros2_interfaces/action/go_to_goal.hpp"

#include <thread>
#include <mutex>

using namespace std::chrono_literals;

// Type aliases for clarity
using Pose = turtlesim::msg::Pose;
using Twist = geometry_msgs::msg::Twist;
using GoToGoalAction = ros2_interfaces::action::GoToGoal;
using GoalHandleGoToGoal = rclcpp_action::ServerGoalHandle<GoToGoalAction>;
using Executors = rclcpp::executors::MultiThreadedExecutor;
class GoToGoalTurtleActionServer : public rclcpp::Node {
public:
    GoToGoalTurtleActionServer() : Node("go_to_goal_turtle_action_server") {
        RCLCPP_INFO(this->get_logger(), "%s has been started.", this->get_name());

        // Subscription to get the turtle's current position
        sub_pose_ = this->create_subscription<Pose>(
            "/turtle1/pose",
            10,
            [this](const Pose::SharedPtr msg) { this->callback_sub_pose(msg); }
        );

        // Publisher to send velocity commands to the turtle
        pub_cmd_vel_ = this->create_publisher<Twist>("/turtle1/cmd_vel", 10);

        // Create the action server
        action_server_ = rclcpp_action::create_server<GoToGoalAction>(
            this,
            "go_to_goal",
            // Goal callback: validates and accepts/rejects new goals
            [this](const rclcpp_action::GoalUUID & uuid, std::shared_ptr<const GoToGoalAction::Goal> goal) {
                return this->goal_callback(uuid, goal);
            },
            // Cancel callback: accepts/rejects cancellation requests
            [this](const std::shared_ptr<GoalHandleGoToGoal> goal_handle) {
                return this->cancel_callback(goal_handle);
            },
            // Accepted callback: starts the execution of an accepted goal
            [this](const std::shared_ptr<GoalHandleGoToGoal> goal_handle) {
                // This is the key to enforcing a single-goal policy (preemption).
                {
                    std::lock_guard<std::mutex> lock(active_goal_mutex_);
                    if (active_goal_handle_ && active_goal_handle_->is_active()) {
                        RCLCPP_INFO(this->get_logger(), "New goal received, aborting previous active goal.");
                        auto result = std::make_shared<GoToGoalAction::Result>();
                        result->success = false;
                        result->message = "New goal preempted the old one.";
                        active_goal_handle_->abort(result);
                    }
                    active_goal_handle_ = goal_handle;
                }
                // Execute the long-running task in a separate thread to avoid blocking the executor
                std::thread([this, goal_handle]() {
                    this->execute_callback(goal_handle);
                }).detach();
            }
        );
    }

private:
    // ROS 2 components
    rclcpp::Subscription<Pose>::SharedPtr sub_pose_;
    rclcpp::Publisher<Twist>::SharedPtr pub_cmd_vel_;
    rclcpp_action::Server<GoToGoalAction>::SharedPtr action_server_;

    // State and Concurrency
    Pose::SharedPtr current_pose_;
    std::mutex pose_mutex_;

    // For preemptive goal handling
    std::mutex active_goal_mutex_;
    std::shared_ptr<GoalHandleGoToGoal> active_goal_handle_;

    // --- Action Server Callbacks ---

    // 1. The "Gatekeeper" for new goals.
    rclcpp_action::GoalResponse goal_callback(
        const rclcpp_action::GoalUUID & uuid,
        std::shared_ptr<const GoToGoalAction::Goal> goal)
    {
        (void)uuid;
        RCLCPP_INFO(this->get_logger(), "Received goal request to navigate to (x:%.2f, y:%.2f, theta:%.2f)",
            goal->x, goal->y, goal->theta);
        
        // For this simple example, we accept all goals.
        // In a real robot, you might reject a goal if it's outside the map boundaries.
        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    }

    // 2. The "Gatekeeper" for cancellation.
    rclcpp_action::CancelResponse cancel_callback(
        const std::shared_ptr<GoalHandleGoToGoal> goal_handle)
    {
        (void)goal_handle;
        RCLCPP_INFO(this->get_logger(), "Received request to cancel goal.");
        // For this simple example, we accept all cancellation requests.
        return rclcpp_action::CancelResponse::ACCEPT;
    }

    // 3. The "Worker" that executes the goal.
    void execute_callback(const std::shared_ptr<GoalHandleGoToGoal> goal_handle) {
        RCLCPP_INFO(this->get_logger(), "Executing goal...");

        // Get goal details from the handle
        const auto goal = goal_handle->get_goal();
        const double goal_x = goal->x;
        const double goal_y = goal->y;
        const double goal_theta = goal->theta;

        // Create messages for feedback and result
        auto feedback = std::make_shared<GoToGoalAction::Feedback>();
        auto result = std::make_shared<GoToGoalAction::Result>();
        
        // Control loop variables
        rclcpp::Rate rate(10.0); // 10 Hz control loop
        bool is_first_phase_done = false;
        bool is_goal_reached = false;
        
        // Proportional gains
        const double kp_dist = 5.0;
        const double kp_angle = 10.0;
        const double threshold = 0.01; // Increased threshold for practical convergence

        // Main execution loop
        while (rclcpp::ok()) {
            // --- Preemption Check ---
            // Check if the goal is still the active one. If not, it was aborted by the server
            // when a new goal arrived. The state is already terminal, so just exit the thread.
            if (!goal_handle->is_active()) {
                RCLCPP_INFO(this->get_logger(), "Goal aborted by server, stopping execution thread.");
                return;
            }

            // --- Cooperative Cancellation Check ---
            if (goal_handle->is_canceling()) {
                stop_turtle();
                result->success = false;
                result->message = "Goal was canceled.";
                goal_handle->canceled(result);
                RCLCPP_INFO(this->get_logger(), "Goal canceled.");
                return;
            }

            // --- Get current pose safely ---
            Pose::SharedPtr local_pose;
            {
                std::lock_guard<std::mutex> lock(pose_mutex_);
                local_pose = this->current_pose_;
            }

            // Safety check: ensure pose data is available before proceeding.
            if (!local_pose) {
                RCLCPP_WARN(this->get_logger(), "Pose data not yet available. Waiting...");
                rate.sleep();
                continue; // Skip this loop iteration and try again.
            }

            // --- Control Logic ---
            double dx = goal_x - local_pose->x;
            double dy = goal_y - local_pose->y;
            double distance = std::sqrt(dx * dx + dy * dy);
            
            Twist::SharedPtr cmd_vel = std::make_shared<Twist>();

            // PHASE 0: Rotation Only
            if (!is_goal_reached && !is_first_phase_done) {
                double angle_to_goal = std::atan2(dy, dx);
                double angle_error = normalize_angle(angle_to_goal - local_pose->theta);

                if (std::abs(angle_error) > threshold) {
                    cmd_vel->angular.z = kp_angle * angle_error;
                } else {
                    cmd_vel->angular.z = 0.0;
                    is_first_phase_done = true;
                    RCLCPP_INFO(this->get_logger(), "Phase 0 complete. Angle aligned.");
                }
            }
            // PHASE 1: Move + Steer
            else if (!is_goal_reached) {
                if (distance > threshold) {
                    double angle_to_goal = std::atan2(dy, dx);
                    double angle_error = normalize_angle(angle_to_goal - local_pose->theta);
                    cmd_vel->linear.x = kp_dist * distance;
                    cmd_vel->angular.z = kp_angle * angle_error;
                } else {
                    cmd_vel->linear.x = 0.0;
                    cmd_vel->angular.z = 0.0;
                    is_goal_reached = true;
                    RCLCPP_INFO(this->get_logger(), "Phase 1 complete. Goal position reached.");
                }
            }
            // PHASE 2: Final Orientation
            else {
                double final_angle_error = normalize_angle(goal_theta - local_pose->theta);
                if (std::abs(final_angle_error) > threshold) {
                    cmd_vel->angular.z = kp_angle * final_angle_error;
                } else {
                    // Task is complete!
                    cmd_vel->angular.z = 0.0;
                    RCLCPP_INFO(this->get_logger(), "Phase 2 complete. Final orientation correct.");
                    break; // Exit the while loop
                }
            }

            // Publish velocity command
            pub_cmd_vel_->publish(*cmd_vel);

            // --- Publish Feedback ---
            feedback->current_x = local_pose->x;
            feedback->current_y = local_pose->y;
            feedback->distance_remaining = distance;
            goal_handle->publish_feedback(feedback);

            rate.sleep();
        }

        // --- Finalize the Goal ---
        if (rclcpp::ok()) {
            stop_turtle();
            result->success = true;
            result->message = "Goal reached successfully!";
            goal_handle->succeed(result);
            RCLCPP_INFO(this->get_logger(), "Goal succeeded!");
        }
    }

    // --- Helper Functions ---

    // Subscriber callback to update the turtle's current pose
    void callback_sub_pose(const Pose::SharedPtr msg) {
        std::lock_guard<std::mutex> lock(pose_mutex_);
        this->current_pose_ = msg;
    }

    // Utility to stop the turtle
    void stop_turtle() {
        // Create a shared pointer to a new Twist message to avoid segfault.
        Twist::SharedPtr stop_cmd = std::make_shared<Twist>();
        stop_cmd->linear.x = 0.0;
        stop_cmd->angular.z = 0.0;
        pub_cmd_vel_->publish(*stop_cmd);
    }
    
    // Utility to normalize an angle to the range [-PI, PI]
    double normalize_angle(double angle) {
        while (angle > M_PI)  angle -= 2.0 * M_PI;
        while (angle < -M_PI) angle += 2.0 * M_PI;
        return angle;
    }
};

int main(int argc, char * argv[]){
    auto log = rclcpp::get_logger("System");
    GoToGoalTurtleActionServer::SharedPtr node_instance = nullptr;

    try{
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting a ROS2 Node...");
        node_instance = std::make_shared<GoToGoalTurtleActionServer>();

        // Use a MultiThreadedExecutor to allow callbacks to run in parallel.
        Executors executor;
        executor.add_node(node_instance);
        executor.spin();

        RCLCPP_INFO(log, "Shutting down the ROS2 Node...");

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