#!/usr/bin/env python3
"""
Launch a Gazebo simulation for the CoreBotics Motor Testbench.
This script initializes the simulation environment, bridges ROS 2 and Gazebo,
and spawns the robot with its associated controllers.
"""
import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, SetEnvironmentVariable, TimerAction
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution, PythonExpression

def generate_launch_description():

    # ================== Workspace Paths =================== #
    
    pkg_gazebo = get_package_share_directory('core_gazebo')
    pkg_description = get_package_share_directory('core_description')
    pkg_ros_gz = get_package_share_directory('ros_gz_sim')

    # ================== Declare Launch Arguments =================== #

    launch_arg_robot_name = DeclareLaunchArgument(
        'robot_name', 
        default_value='motor_testbench',
        description='Entity name for the robot in Gazebo')

    launch_arg_world = DeclareLaunchArgument(
        'world',
        default_value='empty.sdf',
        description='Simulation world file')

    launch_arg_headless = DeclareLaunchArgument(
        'headless',
        default_value='False',
        description='Run Gazebo without the GUI')
    
    launch_arg_sim_time = DeclareLaunchArgument(
        'use_sim_time',
        default_value='true',
        description='Use simulation (Gazebo) clock if true')
    

    # Path to the ROS-Gazebo bridge parameter file
    bridge_config_file = os.path.join(pkg_gazebo, 'config', 'ros_gz_bridge.yaml')

    # ================== Simulation Environment Setup =================== #
    
    # Configure Gazebo resources to find meshes and models
    description_parent_dir = os.path.dirname(pkg_description)

    env_gz_resource_path = SetEnvironmentVariable(
        name='GZ_SIM_RESOURCE_PATH', 
        value=[
            os.environ.get('GZ_SIM_RESOURCE_PATH', ''),
            os.pathsep,
            description_parent_dir
        ]
    )

    # ================== Simulation Backend (Server) =================== #

    world_full_path = PathJoinSubstitution([pkg_gazebo, 'worlds', LaunchConfiguration('world')])

    # Initialize Gazebo Sim server
    action_gazebo_server = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([pkg_ros_gz, 'launch', 'gz_sim.launch.py'])
        ),
        launch_arguments={
            'gz_args': ['-r -s ', world_full_path],
            'on_exit_shutdown': 'true'
        }.items()
    )

    # Initialize Gazebo Sim GUI
    action_gazebo_gui = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([pkg_ros_gz, 'launch', 'gz_sim.launch.py'])
        ),
        launch_arguments={
            'gz_args': '-g ',
            'on_exit_shutdown': 'true'
        }.items(),
        condition=IfCondition(PythonExpression(['not ', LaunchConfiguration('headless')]))
    )

    # ================== Communication Bridge =================== #
    
    # Parameter bridge for message exchange between ROS 2 and Gazebo
    node_parameter_bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        parameters=[{
            'config_file': bridge_config_file,
            'use_sim_time': LaunchConfiguration('use_sim_time')
        }],
        output='screen'
    )

    # ================== Start Robot State Publisher & RViz =================== #
    
    # Launch RViz and Robot State Publisher
    start_rviz_rsp = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_description, 'launch', 'rviz.launch.py')
        ),
        launch_arguments={
            'use_sim_time': LaunchConfiguration('use_sim_time'),
            'rviz': 'true',
            'jsp_gui': 'false'
        }.items(),
    )

    # ================== Spawn Robot into Gazebo =================== #
    
    # Spawn the robot model into the world
    # We specify the world 'empty' to match the name in empty.sdf and avoid free-fall/spawning issues
    node_spawn_entity = Node(
        package='ros_gz_sim',
        executable='create',
        arguments=[
            '-name', LaunchConfiguration('robot_name'),
            '-x', '0.0',
            '-y', '0.0',
            '-z', '0.1',
            '-topic', 'robot_description',
            '-world', 'empty' 
        ],
        output='screen'
    )

   # ================== Control Systems ==================== #

    node_spawner_joint_state = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['joint_state_broadcaster', '--controller-manager', '/controller_manager'],
    )

    node_spawner_velocity_ctrl = Node(
        package='controller_manager',
        executable='spawner',
        arguments=['velocity_controller', '--controller-manager', '/controller_manager'],
    )

    # Delay controller activation to ensure the simulation is ready
    action_delayed_controllers = TimerAction(
        period=3.0,
        actions=[
            node_spawner_joint_state,
            node_spawner_velocity_ctrl,
        ]
    )

    # ================== Build Launch Description =================== #
    
    ld = LaunchDescription()

    # Register arguments
    ld.add_action(launch_arg_world)
    ld.add_action(launch_arg_robot_name)
    ld.add_action(launch_arg_headless)
    ld.add_action(launch_arg_sim_time)

    # Register setup actions
    ld.add_action(env_gz_resource_path)
    ld.add_action(action_gazebo_server)
    ld.add_action(action_gazebo_gui)
    ld.add_action(node_parameter_bridge)
    ld.add_action(start_rviz_rsp)
    ld.add_action(node_spawn_entity)
    ld.add_action(action_delayed_controllers)

    return ld