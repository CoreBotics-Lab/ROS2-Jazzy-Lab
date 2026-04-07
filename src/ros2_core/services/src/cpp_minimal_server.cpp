#include "rclcpp/rclcpp.hpp"
#include "ros2_interfaces/srv/greetings.hpp"

using Greetings = ros2_interfaces::srv::Greetings;

class Greeting_server_node_class : public rclcpp::Node{
    public:
        Greeting_server_node_class() : Node("Greeting_server"){
            RCLCPP_INFO(this->get_logger(), "%s has been started!", this->get_name());
            service_ = this->create_service<Greetings>("greetings",
                
                [this](const Greetings::Request::SharedPtr request,
                       Greetings::Response::SharedPtr      response) ->
                void{this->greetings_callback(request, response);}
            );
        }
        
    private:
        rclcpp::Service<Greetings>::SharedPtr service_;
       
        void greetings_callback(const Greetings::Request::SharedPtr request,
                                Greetings::Response::SharedPtr      response){
            try{
                RCLCPP_INFO(this->get_logger(), "Incoming request: %s", request->greetings.c_str());
                response->response_greetings = "Hello from " + std::string(this->get_name());
                response->success = true;
                RCLCPP_INFO(this->get_logger(), "Sending response: %s", response->response_greetings.c_str());
            }
            catch(const std::exception & e){
                RCLCPP_ERROR(this->get_logger(), "An error occurred in the service callback: %s", e.what());
                response->response_greetings = "Error: Service failed to process request.";
                response->success = false;
            }
            
        }
};


int main(int argc, char * argv[]){
    auto log = rclcpp::get_logger("System");
    Greeting_server_node_class::SharedPtr node_instance = nullptr;
    
    try{
        RCLCPP_INFO(log, "Initializing the ROS2 Client...");
        rclcpp::init(argc, argv);

        RCLCPP_INFO(log, "Starting a ROS2 Node...");
        node_instance = std::make_shared<Greeting_server_node_class>();
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