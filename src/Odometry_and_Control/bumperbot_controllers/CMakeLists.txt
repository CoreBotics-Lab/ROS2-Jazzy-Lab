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
find_package(nav_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)

# Locate the standard system Eigen library
find_package(Eigen3 REQUIRED)
# ________________________________PYTHON____________________________________________

#This installs your scripts so they are available to 'ros2 run'
install(PROGRAMS
  bumperbot_controllers/simple_wheel_speed_controller.py
  bumperbot_controllers/diff_drive_kinematics.py
  bumperbot_controllers/diff_drive_kinematics_matrix_method.py
  bumperbot_controllers/adding_noise_to_odometry.py
  bumperbot_controllers/custom_kalman_filter_1D.py
  DESTINATION lib/${PROJECT_NAME}
)
# This tells ROS 2 that your folder contains a Python module
ament_python_install_package(${PROJECT_NAME})

# ________________________________PYTHON____________________________________________

include_directories(include)
include_directories(${EIGEN3_INCLUDE_DIR})

install(
  DIRECTORY config launch
  DESTINATION share/${PROJECT_NAME}
)

# ________________________________CPP_______________________________________________

add_executable(diff_drive_kinematics_cpp src/diff_drive_kinematics_cpp.cpp)
add_executable(diff_drive_kinematics_matrix_method src/diff_drive_kinematics_matrix_method.cpp)

ament_target_dependencies(diff_drive_kinematics_cpp
                          rclcpp
                          std_msgs
                          geometry_msgs
                          nav_msgs
                          sensor_msgs
                          tf2
                          tf2_ros
                          tf2_geometry_msgs)

ament_target_dependencies(diff_drive_kinematics_matrix_method
                          rclcpp
                          std_msgs
                          geometry_msgs
                          nav_msgs
                          sensor_msgs
                          tf2
                          tf2_ros
                          tf2_geometry_msgs
                          ${EIGEN_LIBRARIES})

add_executable(twist_to_twist_stamped src/twist_to_twist_stamped.cpp)
ament_target_dependencies(twist_to_twist_stamped rclcpp geometry_msgs)

install(TARGETS
  diff_drive_kinematics_cpp
  diff_drive_kinematics_matrix_method
  twist_to_twist_stamped
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
