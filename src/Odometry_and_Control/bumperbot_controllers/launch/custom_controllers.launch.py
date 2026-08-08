"""
Controller-only launch file — custom simple velocity controller stack (course / learning).

Spawns the custom controllers only. Gazebo must already be running
(started via bumperbot_bringup or gazebo.launch.py directly).

For the complete one-command simulation bringup, use:
    ros2 launch bumperbot_bringup sim_custom_controller.launch.py

Nodes spawned:
  - joint_state_broadcaster
  - simple_velocity_controller  (JointGroupVelocityController)
  - diff_drive_kinematics        (converts /cmd_vel → wheel velocity commands)
  - joy_gui (optional)
  - adding_noise_to_odometry (optional)

Launch arguments:
  use_sim_time     : Use simulated (Gazebo) clock (default: True)
  use_joy          : Launch the virtual joystick GUI (default: False)
  add_noise_to_odom: Inject noise into odometry (default: False)

Example (with Gazebo already running):
    ros2 launch bumperbot_controllers custom_controllers.launch.py
"""

import os
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument
from ament_index_python.packages import get_package_share_directory
from launch.conditions import IfCondition
from launch.substitutions import LaunchConfiguration


def generate_launch_description():

    config_file = os.path.join(
        get_package_share_directory('bumperbot_controllers'),
        'config',
        'bumperbot_controllers.yaml'
    )

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
    add_noise_to_odom_arg = DeclareLaunchArgument(
        "add_noise_to_odom",
        default_value="False",
        description="Add noise to odometry data to simulate real-world imperfections."
    )

    use_sim_time      = LaunchConfiguration("use_sim_time")
    use_joy           = LaunchConfiguration("use_joy")
    add_noise_to_odom = LaunchConfiguration("add_noise_to_odom")

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
        use_joy_arg,
        add_noise_to_odom_arg,
        joint_state_broadcaster_spawner,
        simple_velocity_controller_spawner,
        diff_drive_kinematics_node,
        joy_gui_node,
        add_noise_to_odometry_node,
    ])