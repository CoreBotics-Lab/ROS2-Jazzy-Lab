#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "visualization_msgs/msg/marker.hpp"
#include "tf2_ros/buffer.h"
#include "tf2_ros/transform_listener.h"

// Registers the math operations to allow the Buffer to transform PoseStamped messages.
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"

using namespace std::chrono_literals;

class TF2DataTransformerNode : public rclcpp::Node
{
public:
    TF2DataTransformerNode() : Node("tf2_data_transformer_node")
    {
        RCLCPP_INFO(this->get_logger(), "%s has been started!", this->get_name());

        // 1. Create the Buffer and Listener (Pass the clock for use_sim_time compatibility)
        tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
        tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

        // 2. Create the Publisher for the RViz marker
        marker_pub_ = this->create_publisher<visualization_msgs::msg::Marker>("detected_object", 10);
        
        // 3. Create a timer at 10 Hz (every 100ms)
        timer_ = this->create_timer(
            100ms,
            [this]() -> void { this->timer_callback(); }
        );
    }

private:
    void timer_callback()
    {
        // --- Simulating Sensor Data ---
        auto sensor_data = std::make_shared<geometry_msgs::msg::PoseStamped>();
        sensor_data->header.frame_id = "my_dynamic_frame";
        // Using Time(0) to request the most recent available transform
        sensor_data->header.stamp = rclcpp::Time(0); 
        
        sensor_data->pose.position.x = 1.0;
        sensor_data->pose.position.y = 0.0;
        sensor_data->pose.position.z = 0.0;
        sensor_data->pose.orientation.w = 1.0;

        try
        {
            // --- Core Transformation ---
            // Ask the buffer to transform our data into the 'world' frame
            auto world_pose = std::make_shared<geometry_msgs::msg::PoseStamped>();
            *world_pose = tf_buffer_->transform(*sensor_data, "world");

            RCLCPP_INFO(this->get_logger(), "Object in World Frame -> X: %.2f, Y: %.2f, Z: %.2f",
                        world_pose->pose.position.x,
                        world_pose->pose.position.y,
                        world_pose->pose.position.z);

            // --- RViz Visualization ---
            publish_marker(world_pose->pose);
        }
        catch (const tf2::TransformException & ex)
        {
            RCLCPP_WARN(this->get_logger(), "Could not transform data: %s", ex.what());
        }
    }

    void publish_marker(const geometry_msgs::msg::Pose & pose)
    {
        auto marker = std::make_shared<visualization_msgs::msg::Marker>();
        marker->header.frame_id = "world";
        marker->header.stamp = this->get_clock()->now();
        marker->ns = "sensor_data";
        marker->id = 0;
        marker->type = visualization_msgs::msg::Marker::SPHERE;
        marker->action = visualization_msgs::msg::Marker::ADD;
        marker->pose = pose;
        
        marker->scale.x = 0.2;
        marker->scale.y = 0.2;
        marker->scale.z = 0.2;
        marker->color.r = 1.0f;
        marker->color.a = 1.0f;
        
        marker_pub_->publish(*marker);
    }

    std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
    std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
    rclcpp::Publisher<visualization_msgs::msg::Marker>::SharedPtr marker_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char * argv[])
{
    auto log = rclcpp::get_logger("System");
    TF2DataTransformerNode::SharedPtr node_instance = nullptr;

    try {
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting a ROS2 Node...");
        node_instance = std::make_shared<TF2DataTransformerNode>();
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