cmake_minimum_required(VERSION 3.8)
project(bumperbot_controllers)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)

find_package(rclpy REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)  
find_package(rcl_interfaces REQUIRED)

# ________________________________PYTHON____________________________________________

#This installs your scripts so they are available to 'ros2 run'
install(PROGRAMS
  bumperbot_controllers/simple_wheel_speed_controller.py
  bumperbot_controllers/diff_drive_kinematics.py  
  DESTINATION lib/${PROJECT_NAME}
)
# This tells ROS 2 that your folder contains a Python module
ament_python_install_package(${PROJECT_NAME})

# ________________________________PYTHON____________________________________________

include_directories(include)

install(
  DIRECTORY config launch
  DESTINATION share/${PROJECT_NAME}
)

# ________________________________CPP_______________________________________________

add_executable(diff_drive_kinematics_cpp src/diff_drive_kinematics_cpp.cpp)
ament_target_dependencies(diff_drive_kinematics_cpp rclcpp std_msgs geometry_msgs)

install(TARGETS
  diff_drive_kinematics_cpp
  DESTINATION lib/${PROJECT_NAME}
)

# ________________________________CPP_______________________________________________




if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
