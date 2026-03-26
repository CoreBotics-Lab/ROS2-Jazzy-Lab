#include "rclcpp/rclcpp.hpp"
#include "turtlesim/msg/pose.hpp"
#include "geometry_msgs/msg/twist.hpp"

#include "ros2_interfaces/action/go_to_goal.hpp"

using namespace std::chrono_literals;

using Pose = turtlesim::msg::Pose;
using Twist = geometry_msgs::msg::Twist;

class GoToGoalTurtle : public rclcpp::Node{
    public:
        GoToGoalTurtle(double goal_x = 0.0, double goal_y = 0.0, double goal_theta = 0.0) : Node("go_to_goal_turtle"), is_goal_reached(false), is_first_phase_done(false), 
        phase{  "Adjusting the angle", "Trying to reach goal", "Goal Reached"}, goal_x(goal_x), goal_y(goal_y), goal_theta(goal_theta)
        {
            RCLCPP_INFO(this->get_logger(), "%s has been started", this->get_name());

            sub_pose_ = this->create_subscription<Pose>(
                    "/turtle1/pose",
                    10,
                    [this](const Pose::SharedPtr msg)-> void{this->callback_sub_pose(msg);}
            );

            pub_cmd_vel_ = this->create_publisher<Twist>("/turtle1/cmd_vel", 10);

            timer_ = this->create_timer(
                100ms,
                [this]()->void{this->go_to_goal_logic();}
            );

        }


    private:
        /// @brief Subscriber to the turtle's current pose from `/turtle1/pose`.
        rclcpp::Subscription<Pose>::SharedPtr sub_pose_;
        /// @brief Publisher for sending velocity commands to `/turtle1/cmd_vel`.
        rclcpp::Publisher<Twist>::SharedPtr pub_cmd_vel_;
        /// @brief Timer to trigger the main control logic at a fixed rate.
        rclcpp::TimerBase::SharedPtr timer_;

        // --- State and Goal Management ---
        double kp_angle = 10;
        double kp_dist = 5;
        double threshold = 0.0001;

        /// @brief Flag indicating if the turtle has reached its final target pose.
        bool is_goal_reached;
        /// @brief Flag indicating if the initial orientation adjustment is complete.
        bool is_first_phase_done;
        /// @brief Stores descriptive names for the phases of movement.
        std::vector<std::string> phase;
        /// @brief The X-coordinate of the target goal.
        double goal_x, goal_y, goal_theta;
        /// @brief Stores the most recently received pose message from the turtle.
        Pose::SharedPtr msg_pose;
        /// @brief Stores the target pose for the current goal.
        Pose::SharedPtr goal_pose = std::make_shared<Pose>();
        Twist::SharedPtr cmd_vel = std::make_shared<Twist>();



        void callback_sub_pose(const Pose::SharedPtr msg){
            this->msg_pose = msg;
            RCLCPP_INFO(this->get_logger(), "Msg Pose --> x: %.3f\t y: %.3f\t theta: %.3f",
                    msg_pose->x, msg_pose->y, msg_pose->theta);

            //go_to_goal_logic(msg_pose);

        }

        void go_to_goal_logic(){
            // (void)msg_pose;
            // (void)goal_pose;
            // Safety Gate: If no pose data exists yet, exit early!
            if (!this->msg_pose) {
                return; 
            }

            this->goal_pose->x = this->goal_x;
            this->goal_pose->y = this->goal_y;
            this->goal_pose->theta = this->goal_theta;

            // 1. Calculate distance to goal using msg_pose (current position)
            double dx = this->goal_pose->x  - this->msg_pose->x;
            double dy = this->goal_pose->y - this->msg_pose->y;
            double distance = std::sqrt(dx * dx + dy * dy);

            // 2. Calculate target angle
            double angle = std::atan2(dy, dx);
            
            double angle_error = angle - this->msg_pose->theta;

            // Normalize!
            while (angle_error > M_PI)  angle_error -= 2.0 * M_PI;
            while (angle_error < -M_PI) angle_error += 2.0 * M_PI;

            // PHASE 0: Rotation Only
            if (!this->is_goal_reached && !this->is_first_phase_done) {
                if (std::abs(angle_error) > this->threshold) {
                    this->cmd_vel->angular.z = kp_angle * angle_error;
                    RCLCPP_INFO(this->get_logger(), "Phase 0: Rotating. Error: %f", angle_error);
                } else {
                    this->is_first_phase_done = true;
                    RCLCPP_INFO(this->get_logger(), "Angle Aligned. Switching to Phase 1.");
                }
            } 
            // PHASE 1: Move + Steer
            else if (!this->is_goal_reached) {
                // Keep steering toward the goal while moving!
                this->cmd_vel->angular.z = kp_angle * angle_error;
                this->cmd_vel->linear.x = kp_dist * distance;

                if (distance < this->threshold) {
                    this->is_goal_reached = true;
                    RCLCPP_INFO(this->get_logger(), "Goal Reached! Switching to Phase 2.");
                }
                RCLCPP_INFO(this->get_logger(), "Phase 1: Driving. Dist: %f, Angle Err: %f", distance, angle_error);
            }
            // PHASE 2: Final Orientation
            else {
                double final_angle_error = this->goal_theta - this->msg_pose->theta;

                // --- ADD NORMALIZATION HERE ---
                while (final_angle_error > M_PI)  final_angle_error -= 2.0 * M_PI;
                while (final_angle_error < -M_PI) final_angle_error += 2.0 * M_PI;
                // ------------------------------

                if (std::abs(final_angle_error) > this->threshold) {
                    this->cmd_vel->angular.z = kp_angle * final_angle_error;
                    RCLCPP_INFO(this->get_logger(), "Phase 2: Final Turn. Error: %f", final_angle_error);
                } else {
                    // Stop the rotation once aligned
                    this->cmd_vel->angular.z = 0.0;
                    RCLCPP_INFO(this->get_logger(), "Task Complete. Orientation Correct.");
                    rclcpp::shutdown();
                }
            }
            this->pub_cmd_vel_->publish(*cmd_vel);
        }

};


int main(int argc, char * argv[]){
    auto log = rclcpp::get_logger("System");
    GoToGoalTurtle::SharedPtr node_instance = nullptr;

    try{
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        // Default values
        double x = 0.0, y = 0.0, theta = 0.0;
        // Convert arguments if they exist
        if (argc > 1) x = std::stod(argv[1]);
        if (argc > 2) y = std::stod(argv[2]);
        if (argc > 3) theta = std::stod(argv[3]);

        RCLCPP_INFO(log, "Starting a ROS2 Node...");
        node_instance = std::make_shared<GoToGoalTurtle>(x, y, theta);
        rclcpp::spin(node_instance);

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