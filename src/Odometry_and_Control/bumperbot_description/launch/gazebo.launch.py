import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, SetEnvironmentVariable
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution, PythonExpression, Command

def generate_launch_description():

    launch_arg_headless = DeclareLaunchArgument('headless', default_value='False', description='Run Gazebo without the GUI')

    bumperbot_package_dir = get_package_share_directory('bumperbot_description')
    ros_gz_package_dir = get_package_share_directory('ros_gz_sim')
    
    # Configure Gazebo resources to find meshes and models
    bumperbot_parent_dir = os.path.dirname(bumperbot_package_dir)

    env_gz_resource_path = SetEnvironmentVariable(
        name='GZ_SIM_RESOURCE_PATH', 
        value=[
            os.environ.get('GZ_SIM_RESOURCE_PATH', ''),
            os.pathsep,
            bumperbot_parent_dir
        ]
    )

    xacro_file_path = os.path.join(bumperbot_package_dir, 'urdf', 'bumperbot.urdf.xacro')

    robot_description = Command([
        'xacro ', 
        xacro_file_path
    ])

    robot_state_publisher = Node(
        package = 'robot_state_publisher',
        executable = 'robot_state_publisher',
        name = 'robot_state_publisher',
        output = 'screen',
        parameters = [
            {'robot_description': robot_description}
        ]
    )
    
    rviz2 = Node(
        package = 'rviz2',
        executable = 'rviz2',
        name = 'rviz2',
        output = 'screen',
        arguments = ['-d', os.path.join(bumperbot_package_dir, 'rviz', 'display.rviz')]
    )

    gazebo_clock_bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        output='screen',
        name='clock_bridge',
        arguments=[
            '/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock'
        ]
    )

    # Pass '-s ' (server only) if headless is True, otherwise pass empty string to run both by default.
    gz_sim = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([ros_gz_package_dir, 'launch', 'gz_sim.launch.py'])
        ),
        launch_arguments={
            'gz_args': [
                '-r ', 
                PythonExpression(["'-s ' if '", LaunchConfiguration('headless'), "'.lower() == 'true' else ''"]), 
                'empty.sdf'
            ],
            'on_exit_shutdown': 'true'
        }.items()
    )

    gz_spawn_entity = Node(
        package='ros_gz_sim',
        executable='create',
        arguments=[
            '-name', 'bumperbot',
            '-x', '0.0',
            '-y', '0.0',
            '-z', '0.1',
            '-topic', 'robot_description',
            '-world', 'empty' 
        ],
        output='screen'
    )

    return LaunchDescription([
        launch_arg_headless,
        env_gz_resource_path,
        gazebo_clock_bridge,
        gz_sim,
        gz_spawn_entity,
        robot_state_publisher,
        rviz2,
    ])