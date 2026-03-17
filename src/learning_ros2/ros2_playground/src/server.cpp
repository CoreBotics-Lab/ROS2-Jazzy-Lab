#include "rclcpp/rclcpp.hpp"
#include "ros2_interfaces/srv/reset_counter.hpp"
#include "ros2_interfaces/msg/string.hpp"
#include "ros2_interfaces/srv/trigger.hpp"

using namespace std::chrono_literals;
using ResetCounter = ros2_interfaces::srv::ResetCounter;
using String = ros2_interfaces::msg::String;
using Trigger = ros2_interfaces::srv::Trigger;

class Counter_server_node_class : public rclcpp::Node{
    public: 
        Counter_server_node_class() : Node("resetCounter_node"), cnt_(0){
            RCLCPP_INFO(this->get_logger(), "%s has been started!", this->get_name());
            //topics
            publisher_ = this->create_publisher<String>("/counter", 10);
            msg_cnt = std::make_shared<String>();
            //timer
            timer_ = this->create_timer(500ms,
                [this]()-> void{this->timer_callback();}
            );
            //services
            service_ = this->create_service<ResetCounter>("reset_counter", 
                [this](const ResetCounter::Request::SharedPtr request,
                        ResetCounter::Response::SharedPtr response) ->
                        void {this->resetCounter_callback(request, response);}
            );

            service_setZero = this->create_service<Trigger>("set_zero",
                [this](const Trigger::Request::SharedPtr request,
                    Trigger::Response::SharedPtr response) ->
                        void {this->setZero_callback(request, response);}
            );
        }

    private:
        int cnt_;
        String::SharedPtr msg_cnt;
        rclcpp::Publisher<String>::SharedPtr publisher_;
        rclcpp::TimerBase::SharedPtr timer_;
        rclcpp::Service<ResetCounter>::SharedPtr service_;
        rclcpp::Service<Trigger>::SharedPtr service_setZero;

        void timer_callback(){
            this->msg_cnt->data = "Counter: " + std::to_string(this->cnt_);
            this->publisher_->publish(*msg_cnt);
            RCLCPP_INFO(this->get_logger(), "%s", this->msg_cnt->data.c_str());
            this->cnt_++;
        }
        void resetCounter_callback(const ResetCounter::Request::SharedPtr request,
                    ResetCounter::Response::SharedPtr response){

            try{
                this->cnt_ = request->reset_value;
                RCLCPP_INFO(this->get_logger(), "Request received: %ld", request->reset_value);
                response->message = "Counter value has been reset to " + std::to_string(this->cnt_);
                response->success = true;

            }
            catch(const std::exception & e){
                RCLCPP_ERROR(this->get_logger(), "Service Error: %s", e.what());
                response->success = false;
            }
            
        }

        void setZero_callback(const Trigger::Request::SharedPtr request,
                    Trigger::Response::SharedPtr response){
            try{
                (void)request;
                this->cnt_ = 0;
                response->message = "Counter value has been set " + std::to_string(this->cnt_);
                response->success = true;
            }
            catch(const std::exception & e){
                RCLCPP_ERROR(this->get_logger(), "Service Error: %s", e.what());
                response->success = false;
            }
        }

};



int main(int argc, char * argv[]){
    auto log = rclcpp::get_logger("System");
    Counter_server_node_class::SharedPtr node_instance = nullptr;
    
    try{
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting a ROS2 Node...");
        node_instance = std::make_shared<Counter_server_node_class>();
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