"""
Full simulation bringup — bumperbot with ros2_control DiffDriveController.

Single-command entry point for the production controller stack.
Composes:
  1. gazebo.launch.py      → Gazebo sim + robot spawned + ros2_control plugin
                             initialised with bumperbot_ros2_control.yaml
  2. controller_ros2_control.launch.py → controller spawners + twist adapter

Usage:
    ros2 launch bumperbot_bringup sim_ros2_control.launch.py
    ros2 launch bumperbot_bringup sim_ros2_control.launch.py use_joy:=true
    ros2 launch bumperbot_bringup sim_ros2_control.launch.py headless:=true
"""

import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
from launch.substitutions import LaunchConfiguration


def generate_launch_description():

    controllers_pkg = get_package_share_directory('bumperbot_controllers')

    # Controller config yaml that initialises the controller_manager via
    # the gz_ros2_control Gazebo plugin.
    controller_config = os.path.join(
        controllers_pkg, 'config', 'bumperbot_ros2_control.yaml'
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

    # ── 1. Gazebo simulation ──────────────────────────────────────────────────
    # Injects bumperbot_ros2_control.yaml so the controller_manager registers
    # the DiffDriveController type at startup.
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(controllers_pkg, 'launch', 'gazebo.launch.py')
        ),
        launch_arguments={
            'headless':         LaunchConfiguration('headless'),
            'controller_config': controller_config,
        }.items()
    )

    # ── 2. ros2_control controller stack ─────────────────────────────────────
    controllers = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(controllers_pkg, 'launch', 'controller_ros2_control.launch.py')
        ),
        launch_arguments={
            'use_joy': LaunchConfiguration('use_joy'),
        }.items()
    )

    return LaunchDescription([
        headless_arg,
        use_joy_arg,
        gazebo,
        controllers,
    ])
