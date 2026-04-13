#include "rclcpp/rclcpp.hpp"
#include "tf2_ros/buffer.h"
#include "tf2_ros/transform_listener.h"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2/exceptions.h"

using namespace std::chrono_literals;

class TF2ListenerNode : public rclcpp::Node
{
    public:
        TF2ListenerNode() : Node("tf2_listener_node")
        {
            RCLCPP_INFO(this->get_logger(), "%s has been started!", this->get_name());

            // 1. Create the Buffer (The Memory Cache)
            // In C++, the Buffer explicitly requires the node's clock to track time accurately.
            tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());

            // 2. Create the Listener (The Network Subscriber)
            // We pass a reference to the buffer so the listener can feed incoming data into it.
            tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

            // 3. Create a timer to query the buffer every 1 second (1.0 Hz)
            timer_ = this->create_timer(
                1000ms,
                [this]() -> void { this->timer_callback(); }
            );
        }

    private:
        std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
        std::shared_ptr<tf2_ros::TransformListener> tf_listener_;
        rclcpp::TimerBase::SharedPtr timer_;

        void timer_callback()
        {
            std::string target_frame = "world";
            std::string source_frame = "my_dynamic_frame";

            try {
                // 4. Query the Buffer
                // tf2::TimePointZero is the C++ equivalent of asking for the "newest available transform"
                geometry_msgs::msg::TransformStamped t = tf_buffer_->lookupTransform(
                    target_frame,
                    source_frame,
                    tf2::TimePointZero
                );

                // Extract the coordinates and print them!
                double x = t.transform.translation.x;
                double y = t.transform.translation.y;
                double z = t.transform.translation.z;

                RCLCPP_INFO(this->get_logger(), "Dynamic Frame Location -> X: %.2f, Y: %.2f, Z: %.2f", x, y, z);
            }
            catch (const tf2::TransformException & ex) {
                // If the frames don't exist yet, or aren't connected, lookupTransform throws an exception.
                // We MUST catch it so our node doesn't crash!
                RCLCPP_WARN(
                    this->get_logger(), 
                    "Could not transform %s to %s: %s", 
                    target_frame.c_str(), 
                    source_frame.c_str(), 
                    ex.what()
                );
            }
        }
};

int main(int argc, char * argv[])
{
    auto log = rclcpp::get_logger("System");
    TF2ListenerNode::SharedPtr node_instance = nullptr;

    try {
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting a ROS2 Node...");
        node_instance = std::make_shared<TF2ListenerNode>();
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