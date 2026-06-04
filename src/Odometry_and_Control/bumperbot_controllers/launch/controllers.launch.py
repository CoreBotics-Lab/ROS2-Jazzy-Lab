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
    use_simple_controller_arg = DeclareLaunchArgument(
        "use_simple_controller",
        default_value="True",
    )

    use_sim_time = LaunchConfiguration("use_sim_time")
    use_simple_controller = LaunchConfiguration("use_simple_controller")

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

    diff_drive_controller_spawner = Node(
        package="controller_manager",
        executable="spawner",
        arguments=[
            'bumperbot_controller',
            '--controller-manager', '/controller_manager'
        ],
        condition=UnlessCondition(use_simple_controller)
    )

    simple_controller = GroupAction(
        condition=IfCondition(use_simple_controller),
        actions=[
        Node(
            package='controller_manager',
            executable='spawner',
            arguments=[
                'simple_velocity_controller',
                '--controller-manager', '/controller_manager',
            ],
        ),

        Node(
            package='bumperbot_controllers',
            executable='diff_drive_kinematics_cpp',
            name='diff_drive_kinematics',
            output='screen',
            parameters=[config_file]
        )]      

    )
    
    
    return LaunchDescription([
        use_sim_time_arg,
        use_simple_controller_arg,
        joint_state_broadcaster_spawner,
        diff_drive_controller_spawner,
        simple_controller,
    ])