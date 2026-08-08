"""
Launch file for bumperbot custom controllers (course / learning reference).

This file is dedicated to the hand-written simple velocity controller that was
built during the Udemy course to understand differential-drive kinematics from
scratch. It is NOT intended for real-robot use — use controller_ros2_control.launch.py
for production deployments with the battle-tested ros2_control stack.

This launch file is self-contained: it brings up Gazebo (with the correct
controller config pre-loaded) AND spawns all custom controller nodes in a
single command:

    ros2 launch bumperbot_controllers custom_controllers.launch.py

Nodes / actions launched:
  - gazebo.launch.py        : Gazebo sim loaded with bumperbot_controllers.yaml
  - joint_state_broadcaster : publishes joint states from the controller manager
  - simple_velocity_controller: custom JointGroupVelocityController spawner
  - diff_drive_kinematics   : converts /cmd_vel → individual wheel velocity commands
  - joy_gui (optional)      : virtual joystick GUI for manual teleoperation
  - adding_noise_to_odometry: injects noise into odom to simulate real-world drift

Launch arguments:
  use_sim_time     : Use simulated (Gazebo) clock (default: True)
  headless         : Run Gazebo without the GUI (default: False)
  use_joy          : Launch the joystick GUI (default: False)
  add_noise_to_odom: Inject noise into odometry (default: False)

Example:
    ros2 launch bumperbot_controllers custom_controllers.launch.py use_joy:=true
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
    controllers_pkg   = get_package_share_directory('bumperbot_controllers')

    # YAML loaded by gz_ros2_control to initialise the controller_manager.
    # Must declare simple_velocity_controller type so the CM knows how to load it.
    controller_config = os.path.join(controllers_pkg, 'config', 'bumperbot_controllers.yaml')

    # YAML used by individual nodes (diff_drive_kinematics, noise injection).
    config_file = controller_config

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
    add_noise_to_odom_arg = DeclareLaunchArgument(
        "add_noise_to_odom",
        default_value="False",
        description="Add noise to odometry data to simulate real-world imperfections."
    )

    use_sim_time      = LaunchConfiguration("use_sim_time")
    use_joy           = LaunchConfiguration("use_joy")
    add_noise_to_odom = LaunchConfiguration("add_noise_to_odom")

    # ── Gazebo (pre-loaded with the custom controller config) ─────────────────
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(controllers_pkg, 'launch', 'gazebo.launch.py')
        ),
        launch_arguments={
            'headless': LaunchConfiguration('headless'),
            # Inject bumperbot_controllers.yaml so the controller_manager
            # registers simple_velocity_controller on startup.
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

    simple_velocity_controller_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=[
            'simple_velocity_controller',
            '--controller-manager', '/controller_manager',
            '--param-file', config_file,
        ],
    )

    # ── Custom Kinematics Node ────────────────────────────────────────────────
    diff_drive_kinematics_node = Node(
        package='bumperbot_controllers',
        executable='diff_drive_kinematics_matrix_method.py',
        name='diff_drive_kinematics',
        output='screen',
        parameters=[config_file]
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

    # ── Optional: Odometry Noise Injection ────────────────────────────────────
    add_noise_to_odometry_node = Node(
        package='bumperbot_controllers',
        executable='adding_noise_to_odometry.py',
        name='adding_noise_to_odometry',
        output='screen',
        parameters=[config_file],
        condition=IfCondition(add_noise_to_odom)
    )

    return LaunchDescription([
        use_sim_time_arg,
        headless_arg,
        use_joy_arg,
        add_noise_to_odom_arg,
        gazebo,
        joint_state_broadcaster_spawner,
        simple_velocity_controller_spawner,
        diff_drive_kinematics_node,
        joy_gui_node,
        add_noise_to_odometry_node,
    ])