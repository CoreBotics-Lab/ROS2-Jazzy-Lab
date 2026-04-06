cmake_minimum_required(VERSION 3.8)
project(executors)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(rclpy REQUIRED)
find_package(rclcpp REQUIRED)

# ________________________________PYTHON____________________________________________

#This installs your scripts so they are available to 'ros2 run'
install(PROGRAMS
  executors/py_single_threaded_executor.py
  executors/py_multi_threaded_executor.py
  executors/py_multi_threaded_groups.py
  executors/py_reentrant_group.py
  executors/py_hybrid_group.py
  
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
add_executable(cpp_single_threaded_executor src/cpp_single_threaded_executor.cpp)
add_executable(cpp_multi_threaded_executor src/cpp_multi_threaded_executor.cpp)
add_executable(cpp_multi_threaded_groups src/cpp_multi_threaded_groups.cpp)
add_executable(cpp_reentrant_group src/cpp_reentrant_group.cpp)

# Step 2: Link the Libraries
# This tells the compiler where the "shared memory" and "string" code is
ament_target_dependencies(cpp_single_threaded_executor
  rclcpp
)

ament_target_dependencies(cpp_multi_threaded_executor
  rclcpp
)

ament_target_dependencies(cpp_multi_threaded_groups
  rclcpp
)

ament_target_dependencies(cpp_reentrant_group
  rclcpp
)

# Step 3:. Installation logic
# Without this, 'ros2 run' will say the package is found but the node is missing
install(TARGETS
  cpp_single_threaded_executor
  cpp_multi_threaded_executor
  cpp_multi_threaded_groups
  cpp_reentrant_group

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
