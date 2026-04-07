#include "rclcpp/rclcpp.hpp"
#include "ros2_interfaces/srv/greetings.hpp"

using Greetings = ros2_interfaces::srv::Greetings;
using namespace std::chrono_literals;
class Greeting_client_node_class : public rclcpp::Node{
    public:
        Greeting_client_node_class() : Node("Greeting_client"){
            RCLCPP_INFO(this->get_logger(), "%s has been started!", this->get_name());
            request = std::make_shared<Greetings::Request>();
            response = std::make_shared<Greetings::Response>();
            client_ = this->create_client<Greetings>("greetings");
            timer_ = this->create_timer(500ms,
                [this](){this->timer_callback();}
            );

        }
    private:
        Greetings::Request::SharedPtr request;
        Greetings::Response::SharedPtr response;
        rclcpp::Client<Greetings>::SharedPtr client_;
        rclcpp::TimerBase::SharedPtr timer_;

        void timer_callback(){
            this->send_request("Hello from %s" + std::string(this->get_name()));
        }
        
        void send_request(const std::string &value){
            if(this->client_->service_is_ready()){
                this->request->greetings = value;
                this->client_->async_send_request(this->request,
                [this](rclcpp::Client<Greetings>::SharedFuture future) -> void { this->response_callback(future); });
            }
            else{
                RCLCPP_WARN(this->get_logger(), "Service is not found. Waiting for Service server...");
            }
                
        }

        
        void response_callback(rclcpp::Client<Greetings>::SharedFuture future){
            
            try{
                this->response = future.get();
                if(this->response->success){
                    RCLCPP_INFO(this->get_logger(), "Received response: %s", this->response->response_greetings.c_str());
                }
                else{
                    RCLCPP_ERROR(this->get_logger(), "Service call failed: %s", this->response->response_greetings.c_str());
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
    Greeting_client_node_class::SharedPtr node_instance = nullptr;
    
    try{
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting a ROS2 Node...");
        node_instance = std::make_shared<Greeting_client_node_class>();
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