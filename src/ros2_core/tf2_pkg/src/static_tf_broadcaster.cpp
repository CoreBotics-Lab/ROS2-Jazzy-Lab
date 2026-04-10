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

            this->publish_static_transform();
            
        }
    private:
        std::unique_ptr<tf2_ros::StaticTransformBroadcaster> tf_static_broadcaster_;

        void publish_static_transform()
        {
            geometry_msgs::msg::TransformStamped t;

            // Set the header information
            t.header.stamp = this->get_clock()->now();
            t.header.frame_id = "world"; // Parent frame
            t.child_frame_id = "my_static_frame"; // Child frame

            // Define the translation (position) of the child frame relative to the parent
            t.transform.translation.x = 1.0;
            t.transform.translation.y = 2.0;
            t.transform.translation.z = 0.5;

            // Use tf2::Quaternion to easily set Roll, Pitch, Yaw
            tf2::Quaternion q;
            q.setRPY(M_PI/2, 0, 0); // 90 degrees (pi/2) roll, 0 pitch, 0 yaw
            
            t.transform.rotation.x = q.x();
            t.transform.rotation.y = q.y();
            t.transform.rotation.z = q.z();
            t.transform.rotation.w = q.w();


            // Broadcast the static transform
            tf_static_broadcaster_->sendTransform(t);
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