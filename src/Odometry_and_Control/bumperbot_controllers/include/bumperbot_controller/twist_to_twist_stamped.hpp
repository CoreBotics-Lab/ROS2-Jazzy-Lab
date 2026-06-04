#ifndef TWIST_TO_TWIST_STAMPED_HPP
#define TWIST_TO_TWIST_STAMPED_HPP

#include <geometry_msgs/msg/twist.hpp>
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <memory>
#include <rclcpp/rclcpp.hpp>
#include <string>

class TwistToTwistStamped : public rclcpp::Node {
public:
  TwistToTwistStamped();

private:
  // Parameters
  std::string base_frame_id_;

  // Communication Interfaces
  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr twist_subscriber_;
  rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr
      twist_stamped_publisher_;

  // Callback signature
  void callback_twist(const geometry_msgs::msg::Twist::SharedPtr msg);
};

#endif // TWIST_TO_TWIST_STAMPED_HPP
