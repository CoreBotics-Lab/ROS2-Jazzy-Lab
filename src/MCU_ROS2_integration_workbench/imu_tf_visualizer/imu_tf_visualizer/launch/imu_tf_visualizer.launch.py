#!/usr/bin/env python3

from launch import LaunchDescription
from launch.substitutions import Command, FindExecutable, PathJoinSubstitution
from launch_ros.actions import Node
from launch_ros.substitutions import FindPackageShare


def generate_launch_description() -> LaunchDescription:

    pkg_share = FindPackageShare('imu_tf_visualizer')

    # ── URDF / Robot Description ─────────────────────────────────────────────────
    urdf_file = PathJoinSubstitution([pkg_share, 'urdf', 'imu_tf_visualizer.urdf.xacro'])
    rviz_file = PathJoinSubstitution([pkg_share, 'rviz', 'flight_visualizer.rviz'])

    robot_description = Command([
        FindExecutable(name='xacro'), ' ', urdf_file
    ])

    # ── Nodes ───────────────────────────────────────────────────────────────────
    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
        parameters=[{
            'robot_description': robot_description,
            'use_sim_time': False,
        }]
    )

    imu_tf_broadcaster_node = Node(
        package='imu_tf_visualizer',
        executable='imu_tf_broadcaster_node',
        name='imu_tf_broadcaster_node',
        output='screen',
    )

    rviz2_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        output='screen',
        arguments=['-d', rviz_file],
    )

    return LaunchDescription([
        robot_state_publisher_node,
        imu_tf_broadcaster_node,
        rviz2_node,
    ])
