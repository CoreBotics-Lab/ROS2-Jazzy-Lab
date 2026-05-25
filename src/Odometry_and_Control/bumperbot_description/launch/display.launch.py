from launch.substitutions import Command
from launch import LaunchDescription
from ament_index_python.packages import get_package_share_directory
from launch_ros.actions import Node
import os

def generate_launch_description():


    bumperbot_package_dir = get_package_share_directory('bumperbot_description')

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

    joint_state_publisher_gui = Node(
        package = 'joint_state_publisher_gui',
        executable = 'joint_state_publisher_gui',
        name = 'joint_state_publisher_gui',
        output = 'screen'
    )
    
    return LaunchDescription([
        robot_state_publisher,
        rviz2,
        joint_state_publisher_gui
    ])