import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch_ros.actions import Node
from launch.substitutions import Command, PathJoinSubstitution, LaunchConfiguration 
from launch_ros.parameter_descriptions import ParameterValue
from ament_index_python.packages import get_package_share_directory
from launch.conditions import IfCondition

def generate_launch_description():

    # =========================== Arguments ======================
    use_rviz_arg = DeclareLaunchArgument('rviz', default_value='true',
                          description='Whether to start RVIZ')
    rvi_config_arg = DeclareLaunchArgument('rviz_config', default_value='rviz_config.rviz',
                          description='rviz config file')
    robot_model_arg = DeclareLaunchArgument('urdf_model', default_value='motor_testbench.urdf.xacro',
                          description= 'The robot urdf.xacro file to spawn in rviz')
    use_joint_state_publisher_gui_arg = DeclareLaunchArgument('jsp_gui', default_value='true',
                          description='Whether to start Joint State Publisher GUI')

    # =========================== Paths ==========================
    urdf_path = PathJoinSubstitution([
        get_package_share_directory('core_description'),
        'urdf/motor_testbench',
        # 'robot_telenex.urdf.xacro'
        LaunchConfiguration('urdf_model')
    ])
    rviz_config_path = PathJoinSubstitution([
        get_package_share_directory('core_description'),
        'rviz',
        LaunchConfiguration('rviz_config')
    ])

    # ========================= Nodes ============================
    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        parameters=[{
            'robot_description': ParameterValue(Command(['xacro ', urdf_path]), value_type=str)
        }]
    )

    joint_state_publisher_node = Node(
        package='joint_state_publisher_gui',
        executable='joint_state_publisher_gui',
        output='screen',
        condition=IfCondition(LaunchConfiguration('jsp_gui'))
    )

    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        output='screen',
        arguments=['-d', rviz_config_path],
        condition=IfCondition(LaunchConfiguration('rviz'))
    )

    # ========================= Launch Description =========================
    return LaunchDescription([
        robot_model_arg,
        use_rviz_arg,        
        rvi_config_arg,
        use_joint_state_publisher_gui_arg,        
        robot_state_publisher_node,
        joint_state_publisher_node,
        rviz_node
    ])