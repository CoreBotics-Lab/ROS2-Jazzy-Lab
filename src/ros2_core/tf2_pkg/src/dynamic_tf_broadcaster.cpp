#include "rclcpp/rclcpp.hpp"
#include "tf2_ros/transform_broadcaster.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include <cmath>

using namespace std::chrono_literals;

class DynamicTFBroadcasterNode : public rclcpp::Node
{
    public:
        DynamicTFBroadcasterNode() : Node("dynamic_tf_broadcaster_node")
        {
            RCLCPP_INFO(this->get_logger(), "%s has been started!", this->get_name());
            
            // 1. Initialize the dynamic broadcaster
            tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(this);
            
            // 2. Create a timer to periodically publish the transform (approx 30 Hz)
            double timer_period = 1.0 / 30.0; // 30 Hz

            timer_ = this->create_timer(
                std::chrono::duration<double>(timer_period),
                [this]() -> void { this->timer_callback(); }
            );
        }
    private:
        std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
        rclcpp::TimerBase::SharedPtr timer_;

        void timer_callback()
        {
            geometry_msgs::msg::TransformStamped t;
            rclcpp::Time now = this->get_clock()->now();

            // Set the header information
            t.header.stamp = now;
            t.header.frame_id = "world";
            t.child_frame_id = "my_dynamic_frame";

            // Get current time in seconds for our math functions
            double time_now = now.seconds();

            // Define the dynamic translation (oscillating in a 2-meter circle)
            t.transform.translation.x = 2.0 * std::sin(time_now);
            t.transform.translation.y = 2.0 * std::cos(time_now);
            t.transform.translation.z = 0.5;

            // Define the dynamic rotation (spinning around the Z axis)
            tf2::Quaternion q;
            q.setRPY(0.0, 0.0, time_now); // yaw increases over time
            
            t.transform.rotation.x = q.x();
            t.transform.rotation.y = q.y();
            t.transform.rotation.z = q.z();
            t.transform.rotation.w = q.w();

            // Broadcast the dynamic transform
            tf_broadcaster_->sendTransform(t);
        }
};

int main(int argc, char * argv[])
{
    auto log = rclcpp::get_logger("System");
    DynamicTFBroadcasterNode::SharedPtr node_instance = nullptr;

    try {
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting a ROS2 Node...");
        node_instance = std::make_shared<DynamicTFBroadcasterNode>();
        rclcpp::spin(node_instance);

        RCLCPP_WARN(log, "[CTRL+C]>>> Interrupted by the User.");
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