"""
Full simulation bringup — bumperbot with custom simple velocity controller.

Single-command entry point for the course/learning controller stack.
Composes:
  1. gazebo.launch.py      → Gazebo sim + robot spawned + ros2_control plugin
                             initialised with bumperbot_controllers.yaml
  2. custom_controllers.launch.py → controller spawners + kinematics node

Usage:
    ros2 launch bumperbot_bringup sim_custom_controller.launch.py
    ros2 launch bumperbot_bringup sim_custom_controller.launch.py use_joy:=true
    ros2 launch bumperbot_bringup sim_custom_controller.launch.py add_noise_to_odom:=true
    ros2 launch bumperbot_bringup sim_custom_controller.launch.py headless:=true
"""

import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():

    controllers_pkg = get_package_share_directory('bumperbot_controllers')

    # Controller config yaml that initialises the controller_manager via
    # the gz_ros2_control Gazebo plugin. Must declare simple_velocity_controller
    # type so the CM can load it.
    controller_config = os.path.join(
        controllers_pkg, 'config', 'bumperbot_controllers.yaml'
    )

    # ── Launch Arguments ──────────────────────────────────────────────────────
    headless_arg = DeclareLaunchArgument(
        'headless',
        default_value='False',
        description='Run Gazebo without the GUI.'
    )
    use_joy_arg = DeclareLaunchArgument(
        'use_joy',
        default_value='False',
        description='Launch the virtual joystick GUI for manual teleoperation.'
    )
    add_noise_to_odom_arg = DeclareLaunchArgument(
        'add_noise_to_odom',
        default_value='False',
        description='Inject noise into odometry to simulate real-world imperfections.'
    )

    # RViz2 config from bumperbot_bringup
    rviz_config = os.path.join(
        get_package_share_directory('bumperbot_bringup'), 'rviz', 'bumperbot.rviz'
    )

    # ── 1. Gazebo simulation ──────────────────────────────────────────────────
    # Injects bumperbot_controllers.yaml so the controller_manager registers
    # the simple_velocity_controller type at startup, and passes bumperbot.rviz config to RViz2.
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(controllers_pkg, 'launch', 'gazebo.launch.py')
        ),
        launch_arguments={
            'headless':          LaunchConfiguration('headless'),
            'controller_config': controller_config,
            'rviz_config':       rviz_config,
        }.items()
    )

    # ── 2. Custom controller stack ────────────────────────────────────────────
    controllers = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(controllers_pkg, 'launch', 'custom_controllers.launch.py')
        ),
        launch_arguments={
            'use_joy':           LaunchConfiguration('use_joy'),
            'add_noise_to_odom': LaunchConfiguration('add_noise_to_odom'),
        }.items()
    )

    return LaunchDescription([
        headless_arg,
        use_joy_arg,
        add_noise_to_odom_arg,
        gazebo,
        controllers,
    ])
