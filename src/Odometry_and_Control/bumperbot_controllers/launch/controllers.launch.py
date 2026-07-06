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
    use_joy_arg = DeclareLaunchArgument(
        "use_joy",
        default_value="False",
    )

    add_noise_to_odom_arg = DeclareLaunchArgument(
        "add_noise_to_odom",
        default_value="False",
        description="Add noise to odometry data to simulate real-world imperfections."
    )

    use_sim_time = LaunchConfiguration("use_sim_time")
    use_simple_controller = LaunchConfiguration("use_simple_controller")
    use_joy = LaunchConfiguration("use_joy")
    add_noise_to_odom = LaunchConfiguration("add_noise_to_odom")

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

    diff_drivecontroller = GroupAction(
        condition=UnlessCondition(use_simple_controller),
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
            executable='diff_drive_kinematics_matrix_method.py',
            name='diff_drive_kinematics',
            output='screen',
            parameters=[config_file]
        )]      

    )
    
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
        use_simple_controller_arg,
        use_joy_arg,
        add_noise_to_odom_arg,
        joint_state_broadcaster_spawner,
        diff_drivecontroller,
        simple_controller,
        joy_gui_node,
        add_noise_to_odometry_node

    ])