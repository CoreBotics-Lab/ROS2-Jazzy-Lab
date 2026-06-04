import os
from launch import LaunchDescription
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory

def generate_launch_description():

    config_file = os.path.join(
        get_package_share_directory('bumperbot_controllers'),
        'config',
        'bumperbot_controllers.yaml'
    )

    joint_state_broadcaster_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=[
            'joint_state_broadcaster',
            '--controller-manager', '/controller_manager',
            ],
    )
    
    velocity_controller_spawner = Node(
        package='controller_manager',
        executable='spawner',
        arguments=[
            'velocity_controller',
            '--controller-manager', '/controller_manager',
        ],
    )

    diff_drive_kinematics = Node(
        package='bumperbot_controllers',
        executable='diff_drive_kinematics_cpp',
        name='diff_drive_kinematics',
        output='screen',
        parameters=[config_file]
    )
    
    return LaunchDescription([
        joint_state_broadcaster_spawner,
        velocity_controller_spawner,
        diff_drive_kinematics
    ])