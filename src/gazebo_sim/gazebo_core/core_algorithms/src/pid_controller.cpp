#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"

using namespace std::chrono_literals;

// Type aliases for clarity
using Float64MultiArray = std_msgs::msg::Float64MultiArray;
using JointState = sensor_msgs::msg::JointState;

class MotorTestBenchPIDNode : public rclcpp::Node {
public:
  MotorTestBenchPIDNode(double setPoint_)
      : Node("MotorTestBenchPIDNode"), setPoint_(setPoint_), error_(0.0),
        kP_(10.0), is_position_reached_(false) {
    RCLCPP_INFO(this->get_logger(), "%s has been started.", this->get_name());

    pub_speed_ = this->create_publisher<Float64MultiArray>(
        "/velocity_controller/commands", 10);

    joint_state_sub_ = this->create_subscription<JointState>(
        "/joint_states", 10,
        [this](const JointState::ConstSharedPtr &msg) -> void {
          jointStateSub_callback(msg);
        });
  }

private:
  double setPoint_;
  double error_;
  double kP_;
  bool is_position_reached_;
  rclcpp::Publisher<Float64MultiArray>::SharedPtr pub_speed_;
  rclcpp::Subscription<JointState>::SharedPtr joint_state_sub_;

  void jointStateSub_callback(const JointState::ConstSharedPtr &msg) {
    (void)msg;
    (void)setPoint_;
    // (void)error_;
    (void)kP_;
    (void)is_position_reached_;
    auto pub_msg_ = Float64MultiArray();

    for (size_t i = 0; i < msg->name.size(); i++) {
      //   std::cout << msg->name[i] << " : " << msg->position[i] << "\n";
      if (msg->name[i] == "rotor_joint") {
        RCLCPP_INFO(this->get_logger(), "Current Position: %f",
                    msg->position[i]);
        this->error_ = msg->position[i];
        break;
      }
    }

    double error = this->setPoint_ - this->error_;
    if (this->is_position_reached_) {
      pub_msg_.data = {0.0};
    } else if (abs(error) < 0.01) {
      this->is_position_reached_ = true;
      RCLCPP_INFO(this->get_logger(), "Target Position Reached");
      pub_msg_.data = {0.0};
    } else {
      pub_msg_.data = {this->kP_ * error};
    }

    this->pub_speed_->publish(pub_msg_);

    if (this->is_position_reached_) {
      rclcpp::shutdown();
      return;
    }
  }
};

int main(int argc, char *argv[]) {
  auto log = rclcpp::get_logger("System");
  MotorTestBenchPIDNode::SharedPtr node_instance = nullptr;

  try {
    RCLCPP_INFO(log, "Initializing the ROS2 Client...");
    rclcpp::init(argc, argv);

    RCLCPP_INFO(log, "Starting a ROS2 Node...");
    node_instance = std::make_shared<MotorTestBenchPIDNode>(std::stod(argv[1]));

    // Use a MultiThreadedExecutor to allow callbacks to run in parallel.
    rclcpp::spin(node_instance);

    RCLCPP_INFO(log, "Shutting down the ROS2 Node...");

    RCLCPP_WARN(log, "[CTRL+C]>>> Interrupted by the User.");
    RCLCPP_INFO(log, "Destroying the ROS2 Node...");
  } catch (const std::exception &e) {
    RCLCPP_ERROR(log, "Critical Error: %s", e.what());
  }

  if (rclcpp::ok()) {
    // Manually shut down the ROS 2 client library.
    RCLCPP_INFO(log, "Manually shutting down the ROS2 client...");
    rclcpp::shutdown();
  }

  return 0;
}