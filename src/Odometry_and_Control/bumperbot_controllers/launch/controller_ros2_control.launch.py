"""
Launch file for bumperbot ros2_control diff_drive_controller (production path).

This is the primary launch file for real-robot / production deployments.
It uses the battle-tested ros2_control diff_drive_controller which was
recommended in the Udemy course for all real projects.

This launch file is self-contained: it brings up Gazebo (with the correct
controller config pre-loaded) AND spawns all ros2_control nodes in a
single command:

    ros2 launch bumperbot_controllers controller_ros2_control.launch.py

Nodes / actions launched:
  - gazebo.launch.py         : Gazebo sim loaded with bumperbot_ros2_control.yaml
  - joint_state_broadcaster  : publishes joint states from the controller manager
  - bumperbot_controller     : ros2_control DiffDriveController spawner
  - twist_to_twist_stamped   : converts unstamped /cmd_vel → stamped for the controller
  - joy_gui (optional)       : virtual joystick GUI for manual teleoperation

Launch arguments:
  use_sim_time : Use simulated (Gazebo) clock (default: True)
  headless     : Run Gazebo without the GUI (default: False)
  use_joy      : Launch the joystick GUI (default: False)

Example:
    ros2 launch bumperbot_controllers controller_ros2_control.launch.py use_joy:=true
"""

import os
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration


def generate_launch_description():

    # ── Config paths ──────────────────────────────────────────────────────────
    controllers_pkg  = get_package_share_directory('bumperbot_controllers')
    description_pkg  = get_package_share_directory('bumperbot_description')

    # YAML loaded by gz_ros2_control to initialise the controller_manager.
    controller_config = os.path.join(controllers_pkg, 'config', 'bumperbot_ros2_control.yaml')

    # ── Launch Arguments ──────────────────────────────────────────────────────
    use_sim_time_arg = DeclareLaunchArgument(
        "use_sim_time",
        default_value="True",
        description="Use simulated (Gazebo) clock."
    )
    headless_arg = DeclareLaunchArgument(
        "headless",
        default_value="False",
        description="Run Gazebo without the GUI."
    )
    use_joy_arg = DeclareLaunchArgument(
        "use_joy",
        default_value="False",
        description="Launch the virtual joystick GUI for manual teleoperation."
    )

    use_sim_time = LaunchConfiguration("use_sim_time")
    use_joy      = LaunchConfiguration("use_joy")

    # ── Gazebo (pre-loaded with the ros2_control controller config) ───────────
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(description_pkg, 'launch', 'gazebo.launch.py')
        ),
        launch_arguments={
            'headless': LaunchConfiguration('headless'),
            # Inject bumperbot_ros2_control.yaml so the controller_manager
            # registers the DiffDriveController on startup.
            'controller_config': controller_config,
        }.items()
    )

    # ── Controller Manager Spawners ───────────────────────────────────────────
    joint_state_broadcaster_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=[
            'joint_state_broadcaster',
            '--controller-manager', '/controller_manager',
        ],
    )

    bumperbot_controller_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=[
            'bumperbot_controller',
            '--controller-manager', '/controller_manager',
        ],
    )

    # ── Twist Adapter ─────────────────────────────────────────────────────────
    # DiffDriveController expects a stamped TwistStamped; this node bridges the
    # standard unstamped /cmd_vel topic published by teleoperation tools.
    twist_to_twist_stamped_node = Node(
        package='bumperbot_controllers',
        executable='twist_to_twist_stamped',
        name='twist_to_twist_stamped',
        output='screen',
        parameters=[{
            'base_frame_id': 'base_link',
            'subscribe_topic': 'cmd_vel',
            'publish_topic': '/bumperbot_controller/cmd_vel',
        }]
    )

    # ── Optional: Virtual Joystick GUI ────────────────────────────────────────
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
        headless_arg,
        use_joy_arg,
        gazebo,
        joint_state_broadcaster_spawner,
        bumperbot_controller_spawner,
        twist_to_twist_stamped_node,
        joy_gui_node,
    ])