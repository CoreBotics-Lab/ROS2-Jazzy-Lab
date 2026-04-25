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

        // Pre-allocate messages to avoid heap allocation in high-frequency loops
        cmd_vel_ = std::make_shared<Twist>();
        feedback_ = std::make_shared<GoToGoalAction::Feedback>();
        result_ = std::make_shared<GoToGoalAction::Result>();
        stop_cmd_ = std::make_shared<Twist>();

        // Create the action server
        action_server_ = rclcpp_action::create_server<GoToGoalAction>(
            this,
            "go_to_goal",
            [this](const rclcpp_action::GoalUUID & uuid, std::shared_ptr<const GoToGoalAction::Goal> goal) {
                return this->goal_callback(uuid, goal);
            },
            [this](const std::shared_ptr<GoalHandleGoToGoal> goal_handle) {
                return this->cancel_callback(goal_handle);
            },
            [this](const std::shared_ptr<GoalHandleGoToGoal> goal_handle) {
                {
                    std::lock_guard<std::mutex> lock(active_goal_mutex_);
                    if (active_goal_handle_ && active_goal_handle_->is_active()) {
                        RCLCPP_INFO(this->get_logger(), "New goal received, aborting previous active goal.");
                        result_->success = false;
                        result_->message = "New goal preempted the old one.";
                        active_goal_handle_->abort(result_);
                    }
                    active_goal_handle_ = goal_handle;
                }
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

    // Pre-allocated messages
    Twist::SharedPtr cmd_vel_;
    GoToGoalAction::Feedback::SharedPtr feedback_;
    GoToGoalAction::Result::SharedPtr result_;
    Twist::SharedPtr stop_cmd_;

    // For preemptive goal handling
    std::mutex active_goal_mutex_;
    std::shared_ptr<GoalHandleGoToGoal> active_goal_handle_;

    // --- Action Server Callbacks ---

    rclcpp_action::GoalResponse goal_callback(
        const rclcpp_action::GoalUUID & uuid,
        std::shared_ptr<const GoToGoalAction::Goal> goal)
    {
        (void)uuid;
        RCLCPP_INFO(this->get_logger(), "Received goal request to navigate to (x:%.2f, y:%.2f, theta:%.2f)",
            goal->x, goal->y, goal->theta);
        return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
    }

    rclcpp_action::CancelResponse cancel_callback(
        const std::shared_ptr<GoalHandleGoToGoal> goal_handle)
    {
        (void)goal_handle;
        RCLCPP_INFO(this->get_logger(), "Received request to cancel goal.");
        return rclcpp_action::CancelResponse::ACCEPT;
    }

    void execute_callback(const std::shared_ptr<GoalHandleGoToGoal> goal_handle) {
        RCLCPP_INFO(this->get_logger(), "Executing goal...");

        const auto goal = goal_handle->get_goal();
        const double goal_x = goal->x;
        const double goal_y = goal->y;
        const double goal_theta = goal->theta;

        rclcpp::Rate rate(10.0);
        bool is_first_phase_done = false;
        bool is_goal_reached = false;
        
        const double kp_dist = 5.0;
        const double kp_angle = 10.0;
        const double threshold = 0.01;

        while (rclcpp::ok()) {
            if (!goal_handle->is_active()) {
                RCLCPP_INFO(this->get_logger(), "Goal aborted by server, stopping execution thread.");
                return;
            }

            if (goal_handle->is_canceling()) {
                stop_turtle();
                result_->success = false;
                result_->message = "Goal was canceled.";
                goal_handle->canceled(result_);
                RCLCPP_INFO(this->get_logger(), "Goal canceled.");
                return;
            }

            Pose::SharedPtr local_pose;
            {
                std::lock_guard<std::mutex> lock(pose_mutex_);
                local_pose = this->current_pose_;
            }

            if (!local_pose) {
                RCLCPP_WARN(this->get_logger(), "Pose data not yet available. Waiting...");
                rate.sleep();
                continue;
            }

            double dx = goal_x - local_pose->x;
            double dy = goal_y - local_pose->y;
            double distance = std::sqrt(dx * dx + dy * dy);
            
            cmd_vel_->linear.x = 0.0;
            cmd_vel_->angular.z = 0.0;

            if (!is_goal_reached && !is_first_phase_done) {
                double angle_to_goal = std::atan2(dy, dx);
                double angle_error = normalize_angle(angle_to_goal - local_pose->theta);

                if (std::abs(angle_error) > threshold) {
                    cmd_vel_->angular.z = kp_angle * angle_error;
                } else {
                    cmd_vel_->angular.z = 0.0;
                    is_first_phase_done = true;
                    RCLCPP_INFO(this->get_logger(), "Phase 0 complete. Angle aligned.");
                }
            }
            else if (!is_goal_reached) {
                if (distance > threshold) {
                    double angle_to_goal = std::atan2(dy, dx);
                    double angle_error = normalize_angle(angle_to_goal - local_pose->theta);
                    cmd_vel_->linear.x = kp_dist * distance;
                    cmd_vel_->angular.z = kp_angle * angle_error;
                } else {
                    cmd_vel_->linear.x = 0.0;
                    cmd_vel_->angular.z = 0.0;
                    is_goal_reached = true;
                    RCLCPP_INFO(this->get_logger(), "Phase 1 complete. Goal position reached.");
                }
            }
            else {
                double final_angle_error = normalize_angle(goal_theta - local_pose->theta);
                if (std::abs(final_angle_error) > threshold) {
                    cmd_vel_->angular.z = kp_angle * final_angle_error;
                } else {
                    cmd_vel_->angular.z = 0.0;
                    RCLCPP_INFO(this->get_logger(), "Phase 2 complete. Final orientation correct.");
                    break;
                }
            }

            pub_cmd_vel_->publish(*cmd_vel_);

            feedback_->current_x = local_pose->x;
            feedback_->current_y = local_pose->y;
            feedback_->distance_remaining = distance;
            goal_handle->publish_feedback(feedback_);

            rate.sleep();
        }

        if (rclcpp::ok()) {
            stop_turtle();
            result_->success = true;
            result_->message = "Goal reached successfully!";
            goal_handle->succeed(result_);
            RCLCPP_INFO(this->get_logger(), "Goal succeeded!");
        }
    }

    void callback_sub_pose(const Pose::SharedPtr msg) {
        std::lock_guard<std::mutex> lock(pose_mutex_);
        this->current_pose_ = msg;
    }

    void stop_turtle() {
        stop_cmd_->linear.x = 0.0;
        stop_cmd_->angular.z = 0.0;
        pub_cmd_vel_->publish(*stop_cmd_);
    }
    
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
        RCLCPP_INFO(log, "Manually shutting down the ROS2 client...");
        rclcpp::shutdown();
    }

    return 0;
}