from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        # 1. Start the Turtlesim simulator
        Node(
            package='turtlesim',
            executable='turtlesim_node',
            name='turtlesim_node'
        ),
        
        # 2. Start the TF2 broadcaster for the leader (turtle1)
        Node(
            package='turtle_tf2_py',
            executable='turtle_tf2_broadcaster',
            name='broadcaster1',
            parameters=[{'turtlename': 'turtle1'}]
        ),
        
        # 3. Start the TF2 broadcaster for the follower (turtle2)
        Node(
            package='turtle_tf2_py',
            executable='turtle_tf2_broadcaster',
            name='broadcaster2',
            parameters=[{'turtlename': 'turtle2'}]
        ),
        
        # 4. Start your custom C++ TF2 Follower Node
        Node(
            package='ros2_playground',
            executable='turtle_follower_cpp',
            name='turtle_follower_cpp_node'
        ),
        
        # 5. Start your Joystick GUI to drive turtle1
        Node(
            package='ros2_utilities',
            executable='joy_gui',
            name='joy_gui_node',
            parameters=[{
                'topic_name': 'turtle1/cmd_vel'
            }]
        )
    ])