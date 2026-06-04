#include "bumperbot_controller/twist_to_twist_stamped.hpp"

using namespace std::chrono_literals;

using Twist = geometry_msgs::msg::Twist;
using TwistStamped = geometry_msgs::msg::TwistStamped;

TwistToTwistStamped::TwistToTwistStamped() : Node("twist_to_twist_stamped") {
  RCLCPP_INFO(this->get_logger(), "%s node has been successfully initialized.",
              this->get_name());

  rcl_interfaces::msg::ParameterDescriptor desc;
  desc.dynamic_typing = true;

  this->declare_parameter("base_frame_id", "base_link", desc);
  this->base_frame_id_ = this->get_parameter("base_frame_id").as_string();

  RCLCPP_INFO(this->get_logger(), "-> base_frame_id: %s",
              this->base_frame_id_.c_str());

  this->twist_subscriber_ = this->create_subscription<Twist>(
      "cmd_vel", 10, [this](const Twist::SharedPtr msg) -> void {
        this->callback_twist(msg);
      });

  this->twist_stamped_publisher_ =
      this->create_publisher<TwistStamped>("/bumperbot_controller/cmd_vel", 10);
}

void TwistToTwistStamped::callback_twist(const Twist::SharedPtr msg) {

  auto twist_stamped_msg = std::make_shared<TwistStamped>();

  twist_stamped_msg->header.stamp = this->get_clock()->now();
  twist_stamped_msg->header.frame_id = this->base_frame_id_;
  twist_stamped_msg->twist = *msg;

  this->twist_stamped_publisher_->publish(*twist_stamped_msg);
}

int main(int argc, char *argv[]) {
  auto log = rclcpp::get_logger("System");
  TwistToTwistStamped::SharedPtr node_instance = nullptr;

  try {
    RCLCPP_INFO(log, "Initializing the ROS 2 Client...");
    rclcpp::init(argc, argv);

    RCLCPP_INFO(log, "Starting the ROS 2 Node...");
    node_instance = std::make_shared<TwistToTwistStamped>();
    rclcpp::spin(node_instance);

    RCLCPP_WARN(log, "[CTRL+C]>>> Interrupted by the user.");
    RCLCPP_INFO(log, "Destroying the ROS 2 Node...");
  } catch (const std::exception &e) {
    RCLCPP_ERROR(log, "Critical Error: %s", e.what());
  }

  if (rclcpp::ok()) {
    RCLCPP_INFO(log, "Manually shutting down the ROS 2 client...");
    rclcpp::shutdown();
  }

  return 0;
}
