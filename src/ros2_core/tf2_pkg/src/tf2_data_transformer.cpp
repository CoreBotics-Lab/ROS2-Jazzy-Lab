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

        // Allocate memory buckets exactly once!
        sensor_data_ = std::make_shared<geometry_msgs::msg::PoseStamped>();
        world_pose_ = std::make_shared<geometry_msgs::msg::PoseStamped>();
        marker_ = std::make_shared<visualization_msgs::msg::Marker>();

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
        sensor_data_->header.frame_id = "my_dynamic_frame";
        // Using Time(0) to request the most recent available transform
        sensor_data_->header.stamp = rclcpp::Time(0); 
        
        sensor_data_->pose.position.x = 1.0;
        sensor_data_->pose.position.y = 0.0;
        sensor_data_->pose.position.z = 0.0;
        sensor_data_->pose.orientation.w = 1.0;

        try
        {
            // --- Core Transformation ---
            // Ask the buffer to transform our data into the 'world' frame
            *world_pose_ = tf_buffer_->transform(*sensor_data_, "world");

            RCLCPP_INFO(this->get_logger(), "Object in World Frame -> X: %.2f, Y: %.2f, Z: %.2f",
                        world_pose_->pose.position.x,
                        world_pose_->pose.position.y,
                        world_pose_->pose.position.z);

            // --- RViz Visualization ---
            publish_marker(world_pose_->pose);
        }
        catch (const tf2::TransformException & ex)
        {
            RCLCPP_WARN(this->get_logger(), "Could not transform data: %s", ex.what());
        }
    }

    void publish_marker(const geometry_msgs::msg::Pose & pose)
    {
        marker_->header.frame_id = "world";
        marker_->header.stamp = this->get_clock()->now();
        marker_->ns = "sensor_data";
        marker_->id = 0;
        marker_->type = visualization_msgs::msg::Marker::SPHERE;
        marker_->action = visualization_msgs::msg::Marker::ADD;
        marker_->pose = pose;
        
        marker_->scale.x = 0.2;
        marker_->scale.y = 0.2;
        marker_->scale.z = 0.2;
        marker_->color.r = 1.0f;
        marker_->color.a = 1.0f;
        
        marker_pub_->publish(*marker_);
    }

    std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
    std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
    geometry_msgs::msg::PoseStamped::SharedPtr sensor_data_;
    geometry_msgs::msg::PoseStamped::SharedPtr world_pose_;
    visualization_msgs::msg::Marker::SharedPtr marker_;
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