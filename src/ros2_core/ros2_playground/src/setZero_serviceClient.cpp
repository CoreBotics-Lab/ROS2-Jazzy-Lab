#include "rclcpp/rclcpp.hpp"
#include "ros2_interfaces/srv/trigger.hpp"

using namespace std::chrono_literals;
using Trigger = ros2_interfaces::srv::Trigger;

class setZero_node_class: public rclcpp::Node{
    public:
        setZero_node_class() : Node("setZero_node"){
            RCLCPP_INFO(this->get_logger(), "%s has been started!", this->get_name());
            setZero_client_ = this->create_client<Trigger>("set_zero");
            timer_ = this->create_timer(
                500ms,
                [this]()->void{this->send_request();}
            );

        }

    private:
        rclcpp::Client<Trigger>::SharedPtr setZero_client_;
        rclcpp::TimerBase::SharedPtr timer_;


        void send_request(){
            if(this->setZero_client_->service_is_ready()){
                auto request = std::make_shared<Trigger::Request>();
                this->setZero_client_->async_send_request(request,
                    [this](rclcpp::Client<Trigger>::SharedFuture future) -> void { this->response_callback(future); });
            }
        }

        void response_callback(const rclcpp::Client<Trigger>::SharedFuture future){
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
    setZero_node_class::SharedPtr node_instance = nullptr;
    
    try{
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting a ROS2 Node...");
        node_instance = std::make_shared<setZero_node_class>();
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