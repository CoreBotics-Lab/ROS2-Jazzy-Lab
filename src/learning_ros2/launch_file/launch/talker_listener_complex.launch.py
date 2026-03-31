from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration

def generate_launch_description():
    """
    Generate a launch description to start a C++ publisher and subscriber.
    """
    node_name_arg = DeclareLaunchArgument(
        'talker_name',
        default_value='my_cpp_talker',
        description='Name for the talker node'
    )

    return LaunchDescription([
        node_name_arg,
        # Start the C++ minimal publisher from the 'topics' package
        Node(
            package='topics',
            executable='minimal_publisher',
            name=LaunchConfiguration('talker_name'), # using launch argument, you can set the node name from terminal or from another launch file.
            namespace='my_robot', # using namespace
            output='screen'
        ),
        # Start the C++ minimal subscriber from the 'topics' package
        Node(
            package='topics',
            executable='minimal_subscriber',
            name='my_cpp_listener',
            output='screen'
        ),
        # Start the C++ minimal subscriber from the 'topics' package
        Node(
            package='topics',
            executable='minimal_subscriber',
            name='my_cpp_listener',
            namespace='my_robot_2', # using namespace
            output='screen'
        )

    ])


# Terminal Output
"""
ros2 launch launch_file talker_listener_complex.launch.py talker_name:=custom_talker

[minimal_subscriber-2] [INFO] [1774969375.837751711] [my_cpp_listener]: my_cpp_listener has been started
[minimal_publisher-1] [INFO] [1774969375.839216791] [my_robot.custom_talker]: custom_talker has been started
[minimal_subscriber-3] [INFO] [1774969375.840325154] [my_robot_2.my_cpp_listener]: my_cpp_listener has been started
[minimal_publisher-1] [INFO] [1774969376.340680127] [my_robot.custom_talker]: Counter: 0
[minimal_subscriber-2] [INFO] [1774969376.340792984] [my_cpp_listener]: Counter: 0
[minimal_subscriber-3] [INFO] [1774969376.340824079] [my_robot_2.my_cpp_listener]: Counter: 0
[minimal_publisher-1] [INFO] [1774969376.840671914] [my_robot.custom_talker]: Counter: 1
[minimal_subscriber-2] [INFO] [1774969376.840763412] [my_cpp_listener]: Counter: 1
[minimal_subscriber-3] [INFO] [1774969376.840833486] [my_robot_2.my_cpp_listener]: Counter: 1

"""
"""
ros2 node list

/my_cpp_listener
/my_robot/custom_talker
/my_robot_2/my_cpp_listener  
"""