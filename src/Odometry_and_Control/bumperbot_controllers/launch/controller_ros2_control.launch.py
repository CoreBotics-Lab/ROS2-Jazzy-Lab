"""
Controller-only launch file — ros2_control DiffDriveController stack.

Spawns the ros2_control controllers only. Gazebo must already be running
(started via bumperbot_bringup or gazebo.launch.py directly).

For the complete one-command simulation bringup, use:
    ros2 launch bumperbot_bringup sim_ros2_control.launch.py

Nodes spawned:
  - joint_state_broadcaster
  - bumperbot_controller     (DiffDriveController)
  - twist_to_twist_stamped   (bridges /cmd_vel → stamped TwistStamped)
  - joy_gui (optional)

Launch arguments:
  use_sim_time : Use simulated (Gazebo) clock (default: True)
  use_joy      : Launch the virtual joystick GUI (default: False)

Example (with Gazebo already running):
    ros2 launch bumperbot_controllers controller_ros2_control.launch.py
"""

import os
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from ament_index_python.packages import get_package_share_directory
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration
from launch.actions import DeclareLaunchArgument, TimerAction


def generate_launch_description():

    # ── Launch Arguments ──────────────────────────────────────────────────────
    use_sim_time_arg = DeclareLaunchArgument(
        "use_sim_time",
        default_value="True",
        description="Use simulated (Gazebo) clock."
    )
    use_joy_arg = DeclareLaunchArgument(
        "use_joy",
        default_value="False",
        description="Launch the virtual joystick GUI for manual teleoperation."
    )

    use_sim_time = LaunchConfiguration("use_sim_time")
    use_joy      = LaunchConfiguration("use_joy")

    # ── Controller Manager Spawners ───────────────────────────────────────────
    joint_state_broadcaster_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=[
            'joint_state_broadcaster',
            '--controller-manager', '/controller_manager',
            "--controller-manager-timeout", "60",
        ],
    )

    bumperbot_controller_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=[
            'bumperbot_controller',
            '--controller-manager', '/controller_manager',
            "--controller-manager-timeout", "60",
        ],
    )

    # Delay spawner execution by 5 seconds so Gazebo Sim, Ogre2 render engine,
    # and /clock bridge finish initializing before activating controllers.
    delayed_spawners = TimerAction(
        period=5.0,
        actions=[
            joint_state_broadcaster_spawner,
            bumperbot_controller_spawner,
        ]
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
        use_joy_arg,
        delayed_spawners,
        twist_to_twist_stamped_node,
        joy_gui_node,
    ])