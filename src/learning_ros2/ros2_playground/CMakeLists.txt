cmake_minimum_required(VERSION 3.8)
project(ros2_playground)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(rclpy REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(ros2_interfaces REQUIRED)
find_package(turtlesim REQUIRED)
find_package(geometry_msgs REQUIRED)


# ________________________________PYTHON____________________________________________

#This installs your scripts so they are available to 'ros2 run'
install(PROGRAMS
  ros2_playground/pub_cnt_reset_server.py
  
  DESTINATION lib/${PROJECT_NAME}
)
# This tells ROS 2 that your folder contains a Python module
ament_python_install_package(${PROJECT_NAME})

# ________________________________PYTHON____________________________________________


# ________________________________CPP________________________________________________

add_executable(cpp__server src/server.cpp)
add_executable(resetCounterClient src/resetCnt_client.cpp)
add_executable(setZeroClient src/setZero_client.cpp)
add_executable(turtleGoToGoalTopic src/turtle_go_to_goal_topics.cpp)

ament_target_dependencies(cpp__server
  rclcpp
  ros2_interfaces
)

ament_target_dependencies(resetCounterClient
  rclcpp
  ros2_interfaces
)

ament_target_dependencies(setZeroClient
  rclcpp
  ros2_interfaces
)

ament_target_dependencies(turtleGoToGoalTopic
  rclcpp
  ros2_interfaces
  geometry_msgs
  turtlesim
)

install(TARGETS
  cpp__server
  resetCounterClient
  setZeroClient
  turtleGoToGoalTopic

  DESTINATION lib/${PROJECT_NAME}
)


# ________________________________CPP________________________________________________

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
