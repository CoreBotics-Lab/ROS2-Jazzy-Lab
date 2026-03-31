from launch import LaunchDescription
from launch_ros.actions import Node
def generate_launch_description():
    """
    Generate a launch description to start a C++ publisher and subscriber.
    """
    return LaunchDescription([
        # Start the C++ minimal publisher from the 'topics' package
        Node(
            package='topics',
            executable='minimal_publisher',
            name='my_cpp_talker',
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
ros2 launch launch_file talker_listener_complex.launch.py

[minimal_subscriber-2] [INFO] [1774969055.180211151] [my_cpp_listener]: my_cpp_listener has been started
[minimal_subscriber-3] [INFO] [1774969055.180357831] [my_robot_2.my_cpp_listener]: my_cpp_listener has been started
[minimal_publisher-1] [INFO] [1774969055.180780309] [my_robot.my_cpp_talker]: my_cpp_talker has been started
[minimal_publisher-1] [INFO] [1774969055.682422078] [my_robot.my_cpp_talker]: Counter: 0
[minimal_subscriber-2] [INFO] [1774969055.682585856] [my_cpp_listener]: Counter: 0
[minimal_subscriber-3] [INFO] [1774969055.682617884] [my_robot_2.my_cpp_listener]: Counter: 0
[minimal_publisher-1] [INFO] [1774969056.182205494] [my_robot.my_cpp_talker]: Counter: 1
[minimal_subscriber-3] [INFO] [1774969056.182303817] [my_robot_2.my_cpp_listener]: Counter: 1
[minimal_subscriber-2] [INFO] [1774969056.182303695] [my_cpp_listener]: Counter: 1

"""
"""
ros2 node list

/my_cpp_listener
/my_robot/my_cpp_talker
/my_robot_2/my_cpp_listener
  
"""