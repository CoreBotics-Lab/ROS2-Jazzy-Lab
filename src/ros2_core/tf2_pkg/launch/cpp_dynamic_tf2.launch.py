import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch.substitutions import Command

def generate_launch_description():
    # 1. Get the path to your package and URDF file
    pkg_share = get_package_share_directory('tf2_pkg')
    urdf_file = os.path.join(pkg_share, 'urdf', 'dynamic_tf.urdf.xacro')
    rviz_config_file = os.path.join(pkg_share, 'rviz', 'rviz_config.rviz')

    # 2. Tell ROS 2 to run the xacro command to parse your URDF into XML
    robot_description = ParameterValue(Command(['xacro ', urdf_file]), value_type=str)

    # 3. Define the Robot State Publisher Node
    robot_state_publisher_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        parameters=[{'robot_description': robot_description}],
        output='screen'
    )

    # 4. Define your Python Dynamic TF Broadcaster Node
    dynamic_tf_node = Node(
        package='tf2_pkg',
        executable='dynamic_tf_broadcaster',
        name='dynamic_tf_broadcaster_node',
        output='screen'
    )

    # 5. Define the RViz2 Node
    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        output='screen',
        arguments=['-d', rviz_config_file]
    )

    # 6. Return the LaunchDescription to start them all
    return LaunchDescription([
        robot_state_publisher_node,
        dynamic_tf_node,
        rviz_node
        ])