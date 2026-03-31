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
            output='screen'
        ),
        # Start the C++ minimal subscriber from the 'topics' package
        Node(
            package='topics',
            executable='minimal_subscriber',
            name='my_cpp_listener',
            output='screen'
        )
    ])