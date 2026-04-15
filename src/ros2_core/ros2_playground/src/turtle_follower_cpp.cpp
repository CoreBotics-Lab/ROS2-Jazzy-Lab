#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "turtlesim/srv/spawn.hpp"
#include "tf2_ros/transform_listener.h"
#include "tf2_ros/buffer.h"
#include "tf2/exceptions.h"

#include <cmath>
#include <memory>

using namespace std::chrono_literals;

class TurtleFollowerCpp : public rclcpp::Node {
public:
    TurtleFollowerCpp() : Node("turtle_follower_cpp_node"), turtle_spawning_state_(false), turtle_spawned_(false) {
        RCLCPP_INFO(this->get_logger(), "%s has been started!", this->get_name());

        // 1. Initialize the TF2 Buffer and Listener
        tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
        tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

        // 2. Setup the Publisher to drive turtle2
        publisher_ = this->create_publisher<geometry_msgs::msg::Twist>("/turtle2/cmd_vel", 10);

        // 3. Setup the Service Client to spawn turtle2
        spawner_ = this->create_client<turtlesim::srv::Spawn>("spawn");

        // 4. Pre-allocate the Twist message to avoid runtime memory allocation
        twist_msg_ = std::make_shared<geometry_msgs::msg::Twist>();
        transform_msg_ = std::make_shared<geometry_msgs::msg::TransformStamped>();

        // 5. Timer for the control loop
        timer_ = this->create_timer(100ms, [this]() { this->timer_callback(); });
    }

private:
    bool turtle_spawning_state_;
    bool turtle_spawned_;

    std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
    std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
    
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr publisher_;
    rclcpp::Client<turtlesim::srv::Spawn>::SharedPtr spawner_;
    rclcpp::TimerBase::SharedPtr timer_;
    geometry_msgs::msg::Twist::SharedPtr twist_msg_;
    geometry_msgs::msg::TransformStamped::SharedPtr transform_msg_;

    void timer_callback() {
        // --- Phase 1: Wait for turtle2 to spawn ---
        if (!turtle_spawned_) {
            if (!turtle_spawning_state_ && spawner_->service_is_ready()) {
                auto request = std::make_shared<turtlesim::srv::Spawn::Request>();
                request->x = 2.0;
                request->y = 2.0;
                request->theta = 0.0;
                request->name = "turtle2";
                
                spawner_->async_send_request(request, [this](rclcpp::Client<turtlesim::srv::Spawn>::SharedFuture future) {
                    (void)future; // Acknowledge response
                    RCLCPP_INFO(this->get_logger(), "Spawned turtle2 successfully!");
                    turtle_spawned_ = true;
                });
                turtle_spawning_state_ = true;
            }
            return; // Skip TF2 math until the spawn is confirmed
        }

        // --- Phase 2: TF2 Math and Control Loop ---
        try {
            // Lookup transform FROM turtle2 (target) TO turtle1 (source)
            // Asking: "Where is turtle1 relative to turtle2's perspective?"
            *transform_msg_ = tf_buffer_->lookupTransform("turtle2", "turtle1", tf2::TimePointZero);
        } catch (const tf2::TransformException & ex) {
            // Do not crash if the transform isn't ready yet
            RCLCPP_DEBUG(this->get_logger(), "Could not transform: %s", ex.what());
            return;
        }

        // Extract translation vectors
        double dx = transform_msg_->transform.translation.x;
        double dy = transform_msg_->transform.translation.y;

        // Calculate actual distance and relative angle
        double distance = std::sqrt(dx * dx + dy * dy);
        double angle = std::atan2(dy, dx);

        // Calculate the error based on our desired following distance
        double target_distance = 1.0;
        double distance_error = distance - target_distance;

        // Simple Proportional (P) Controller
        twist_msg_->linear.x = 1.5 * distance_error;
        twist_msg_->angular.z = 4.0 * angle;

        publisher_->publish(*twist_msg_);
    }
};

int main(int argc, char * argv[]) {
    auto log = rclcpp::get_logger("System");
    TurtleFollowerCpp::SharedPtr node_instance = nullptr;
    
    try {
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting a ROS2 Node...");
        node_instance = std::make_shared<TurtleFollowerCpp>();
        rclcpp::spin(node_instance);
        RCLCPP_WARN(log, "[CTRL+C]>>> Interrupted by the User.");
        RCLCPP_INFO(log, "Destroying the ROS2 Node...");
    } catch(const std::exception & e) {
        RCLCPP_ERROR(log, "Critical Error: %s", e.what());
    }

    if(rclcpp::ok()) {
        RCLCPP_INFO(log, "Manually shutting down the ROS2 client...");
        rclcpp::shutdown();
    }

    return 0;
}