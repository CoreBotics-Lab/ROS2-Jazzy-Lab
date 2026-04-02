cmake_minimum_required(VERSION 3.8)
project(parameters)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclpy REQUIRED)

# ________________________________PYTHON____________________________________________

#This installs your scripts so they are available to 'ros2 run'
install(PROGRAMS
  parameters/py_parameters_polling.py
  parameters/py_parameters_event_driven.py
  parameters/py_parameters_from_config.py
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
add_executable(cpp_parameters_polling src/cpp_parameters_polling.cpp)

# Step 2: Link the Libraries
# This tells the compiler where the "shared memory" and "string" code is
ament_target_dependencies(cpp_parameters_polling
  rclcpp
)

# Step 3:. Installation logic
# Without this, 'ros2 run' will say the package is found but the node is missing
install(TARGETS
  cpp_parameters_polling

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
