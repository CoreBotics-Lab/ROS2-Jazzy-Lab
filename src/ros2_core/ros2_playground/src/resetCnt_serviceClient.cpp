#include "rclcpp/rclcpp.hpp"
#include "ros2_interfaces/srv/reset_counter.hpp"

using namespace std::chrono_literals;
using ResetCounter = ros2_interfaces::srv::ResetCounter;

class resetCnt_node_class: public rclcpp::Node{
    public:
        resetCnt_node_class(int _value) : Node("reset_counter_node"), value_(_value){
            RCLCPP_INFO(this->get_logger(), "%s has been started!", this->get_name());
            resetCounter_client = this->create_client<ResetCounter>("reset_counter");
            timer_ = this->create_timer(
                500ms,
                [this]()->void{this->send_request();}
            );
        }

    private:
        int value_;
        rclcpp::Client<ResetCounter>::SharedPtr resetCounter_client;
        rclcpp::TimerBase::SharedPtr timer_;

        void send_request(){
            if(this->resetCounter_client->service_is_ready()){
                auto request = std::make_shared<ResetCounter::Request>();
                request->reset_value = value_;
                this->resetCounter_client->async_send_request(request,
                    [this](rclcpp::Client<ResetCounter>::SharedFuture future) -> void { this->response_callback(future); });

            }
        }
        void response_callback(rclcpp::Client<ResetCounter>::SharedFuture future){
            try{
                auto response = future.get();
                if(response->success){
                    RCLCPP_INFO(this->get_logger(), "Received response: %s", response->message.c_str());
                }
                else{
                    RCLCPP_ERROR(this->get_logger(), "Service call failed: %s", response->message.c_str());
                }
                this->timer_->cancel();
                rclcpp::shutdown();
            }
            catch(const std::exception & e){
                RCLCPP_ERROR(this->get_logger(), "Service call failed: %s", e.what());
            }

        }
};

int main(int argc, char * argv[]){
    auto log = rclcpp::get_logger("System");
    resetCnt_node_class::SharedPtr node_instance = nullptr;
    
    try{
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting a ROS2 Node...");
        node_instance = std::make_shared<resetCnt_node_class>(std::stoi(argv[1]));
        rclcpp::spin(node_instance);
        RCLCPP_WARN(log, "[CTRL+C]>>> Interrupted by the User.");
        RCLCPP_INFO(log, "Destroying the ROS2 Node...");
    }
    catch(const std::exception & e){
        RCLCPP_ERROR(log, "Critical Error: %s", e.what());
    }

    if(rclcpp::ok()){
        RCLCPP_INFO(log, "Manually shutting down the ROS2 client...");
        rclcpp::shutdown();
    }


    return 0;
}