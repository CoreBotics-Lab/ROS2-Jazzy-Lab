"""
Launch file for bumperbot controllers.

This script manages the launch of various control and simulation nodes for the bumperbot,
including the joint state broadcaster, velocity controllers, and optional utility nodes.

Launch arguments:
  use_sim_time: Whether to use simulated time (default: True)
  use_simple_controller: If True, launches the custom simple velocity controller. If False, launches the pre-made ros2_control diff_drive_controller (default: True)
  use_joy: Whether to launch the joystick GUI node for manual control (default: False)
  add_noise_to_odom: Whether to launch the node that injects noise into odometry (default: False)

for example:
    ros2 launch bumperbot_controllers controllers.launch.py use_joy:=true add_noise_to_odom:=true
"""
import os
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import GroupAction, DeclareLaunchArgument
from ament_index_python.packages import get_package_share_directory
from launch.conditions import UnlessCondition, IfCondition
from launch.substitutions import LaunchConfiguration

def generate_launch_description():

    use_sim_time_arg = DeclareLaunchArgument(
        "use_sim_time",
        default_value="True",
    )
    use_joy_arg = DeclareLaunchArgument(
        "use_joy",
        default_value="False",
    )

    use_sim_time = LaunchConfiguration("use_sim_time")
    use_joy = LaunchConfiguration("use_joy")

    config_file = os.path.join(
        get_package_share_directory('bumperbot_controllers'),
        'config',
        'bumperbot_ros2_control.yaml'
    )

    joint_state_broadcaster_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=[
            'joint_state_broadcaster',
            '--controller-manager', '/controller_manager',
            ],
    )

    diff_drivecontroller = GroupAction(
        actions=[
            Node(
            package="controller_manager",
            executable="spawner",
            arguments=[
                'bumperbot_controller',
                '--controller-manager', '/controller_manager'
            ]
            ),
            Node(
                package='bumperbot_controllers',
                executable='twist_to_twist_stamped',
                name='twist_to_twist_stamped',
                output='screen',
                parameters=[{
                    'base_frame_id': 'base_link', 
                    'subscribe_topic': 'cmd_vel',
                    'publish_topic': '/bumperbot_controller/cmd_vel'
                }]
            )
        ])

    
    joy_gui_node = Node(
        package='ros2_utilities',
        executable='joy_gui',
        name='joy_gui',
        output='screen',
        condition=IfCondition(use_joy),
        parameters=[{
            'max_angular': 3.14,
            'max_linear': 0.5,
            'topic_name': '/cmd_vel',
            'publish_rate_hz': 10.0,            
            'use_sim_time': use_sim_time,
        }]
    )
    
    
    return LaunchDescription([
        use_sim_time_arg,
        use_joy_arg,
        joint_state_broadcaster_spawner,
        diff_drivecontroller,
        joy_gui_node,

    ])