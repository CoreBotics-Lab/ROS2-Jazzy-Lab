#include "rclcpp/rclcpp.hpp"
#include "tf2_ros/static_transform_broadcaster.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include <cmath>

using namespace std::chrono_literals;

class StaticTFBroadcasterNode : public rclcpp::Node
{
    public:
        StaticTFBroadcasterNode() : Node("static_tf_broadcaster_node")
        {
            RCLCPP_INFO(this->get_logger(), "%s has been started!", this->get_name());
            tf_static_broadcaster_ = std::make_unique<tf2_ros::StaticTransformBroadcaster>(this);

            // Pre-allocate the message memory
            t_ = std::make_shared<geometry_msgs::msg::TransformStamped>();

            this->publish_static_transform();
            
        }
    private:
        std::unique_ptr<tf2_ros::StaticTransformBroadcaster> tf_static_broadcaster_;
        geometry_msgs::msg::TransformStamped::SharedPtr t_;

        void publish_static_transform()
        {
            // Set the header information
            t_->header.stamp = this->get_clock()->now();
            t_->header.frame_id = "world"; // Parent frame
            t_->child_frame_id = "my_static_frame"; // Child frame

            // Define the translation (position) of the child frame relative to the parent
            t_->transform.translation.x = 1.0;
            t_->transform.translation.y = 2.0;
            t_->transform.translation.z = 0.5;

            // Use tf2::Quaternion to easily set Roll, Pitch, Yaw
            tf2::Quaternion q;
            q.setRPY(M_PI/2, 0, 0); // 90 degrees (pi/2) roll, 0 pitch, 0 yaw
            
            t_->transform.rotation.x = q.x();
            t_->transform.rotation.y = q.y();
            t_->transform.rotation.z = q.z();
            t_->transform.rotation.w = q.w();


            // Broadcast the static transform
            tf_static_broadcaster_->sendTransform(*t_);
        }
};

int main(int argc, char * argv[])
{
    auto log = rclcpp::get_logger("System");
    StaticTFBroadcasterNode::SharedPtr node_instance = nullptr;

    try {
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting a ROS2 Node...");
        node_instance = std::make_shared<StaticTFBroadcasterNode>();
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