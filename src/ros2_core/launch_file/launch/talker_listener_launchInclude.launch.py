from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
import os
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():
    # Declare a launch argument for the talker_name, which will be passed to the included launch file
    talker_name_arg = DeclareLaunchArgument(
        'included_talker_name',
        default_value='included_custom_talker',
        description='Name for the talker node in the included launch file'
    )

    # Get the path to the 'launch_file' package share directory
    launch_file_share_dir = get_package_share_directory('launch_file')
    complex_launch_file_path = os.path.join(launch_file_share_dir, 'launch', 'talker_listener_complex.launch.py')

    return LaunchDescription([
        talker_name_arg,
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(complex_launch_file_path),
            launch_arguments={'talker_name': LaunchConfiguration('included_talker_name')}.items()
        )
    ])



# Terminal Output
"""
ros2 launch launch_file talker_listener_launchInclude.launch.py

[minimal_subscriber-2] [INFO] [1774972115.943157014] [my_cpp_listener]: my_cpp_listener has been started
[minimal_publisher-1] [INFO] [1774972115.943208278] [my_robot.included_custom_talker]: included_custom_talker has been started
[minimal_subscriber-3] [INFO] [1774972115.945630742] [my_robot_2.my_cpp_listener]: my_cpp_listener has been started
[minimal_publisher-1] [INFO] [1774972116.444965685] [my_robot.included_custom_talker]: Counter: 0
[minimal_subscriber-3] [INFO] [1774972116.445103784] [my_robot_2.my_cpp_listener]: Counter: 0
[minimal_subscriber-2] [INFO] [1774972116.445121111] [my_cpp_listener]: Counter: 0
[minimal_publisher-1] [INFO] [1774972116.945094557] [my_robot.included_custom_talker]: Counter: 1
[minimal_subscriber-3] [INFO] [1774972116.945237910] [my_robot_2.my_cpp_listener]: Counter: 1
[minimal_subscriber-2] [INFO] [1774972116.945265128] [my_cpp_listener]: Counter: 1

"""
"""
ros2 node list

/my_cpp_listener
/my_robot/included_custom_talker
/my_robot_2/my_cpp_listener 
"""


# Terminal Output
"""
ros2 launch launch_file talker_listener_launchInclude.launch.py included_talker_name:=custom_talker_from_terminal


[minimal_subscriber-3] [INFO] [1774972245.418828925] [my_robot_2.my_cpp_listener]: my_cpp_listener has been started
[minimal_subscriber-2] [INFO] [1774972245.419976874] [my_cpp_listener]: my_cpp_listener has been started
[minimal_publisher-1] [INFO] [1774972245.420285569] [my_robot.custom_talker_from_terminal]: custom_talker_from_terminal has been started
[minimal_publisher-1] [INFO] [1774972245.921897838] [my_robot.custom_talker_from_terminal]: Counter: 0
[minimal_subscriber-2] [INFO] [1774972245.922068151] [my_cpp_listener]: Counter: 0
[minimal_subscriber-3] [INFO] [1774972245.922072417] [my_robot_2.my_cpp_listener]: Counter: 0
[minimal_publisher-1] [INFO] [1774972246.421687703] [my_robot.custom_talker_from_terminal]: Counter: 1
[minimal_subscriber-2] [INFO] [1774972246.421838038] [my_cpp_listener]: Counter: 1
[minimal_subscriber-3] [INFO] [1774972246.421838052] [my_robot_2.my_cpp_listener]: Counter: 1

"""
"""
ros2 node list

/my_cpp_listener
/my_robot/custom_talker_from_terminal
/my_robot_2/my_cpp_listener 
"""