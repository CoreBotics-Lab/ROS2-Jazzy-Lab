cmake_minimum_required(VERSION 3.8)
project(tf2_pkg)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)

find_package(rclcpp REQUIRED)
find_package(rclpy REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2 REQUIRED)

install(
  DIRECTORY launch urdf rviz
  DESTINATION share/${PROJECT_NAME}
 )
 
# ________________________________PYTHON____________________________________________

#This installs your scripts so they are available to 'ros2 run'
install(PROGRAMS
  tf2_pkg/static_tf_broadcaster.py
  tf2_pkg/dynamic_tf_broadcaster.py
  
  DESTINATION lib/${PROJECT_NAME}
)
# This tells ROS 2 that your folder contains a Python module
ament_python_install_package(${PROJECT_NAME})

# ________________________________PYTHON____________________________________________

# ________________________________CPP________________________________________________
#     START, LINK and INSTALL

#  Create the Executable
# Format: add_executable(target_name source_file)
#Step 1: Create the Executable
add_executable(static_tf_broadcaster src/static_tf_broadcaster.cpp)
add_executable(dynamic_tf_broadcaster src/dynamic_tf_broadcaster.cpp)

# Step 2: Link the Libraries
# This tells the compiler where the "shared memory" and "string" code is
ament_target_dependencies(static_tf_broadcaster
  rclcpp
  tf2_ros
  geometry_msgs
  tf2
)
ament_target_dependencies(dynamic_tf_broadcaster
  rclcpp
  tf2_ros
  geometry_msgs
  tf2
)


# Step 3:. Installation logic
# Without this, 'ros2 run' will say the package is found but the node is missing
install(TARGETS
  static_tf_broadcaster
  dynamic_tf_broadcaster
  
  DESTINATION lib/${PROJECT_NAME})


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
