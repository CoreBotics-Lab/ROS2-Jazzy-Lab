# from launch_ros.actions import Node
from launch_ros.actions.node import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch.substitutions import Command
from ament_index_python.packages import get_package_share_directory
import os
from launch import LaunchDescription


def generate_launch_description():

    urdf_file = os.path.join(get_package_share_directory("core_description"), "urdf/tutorial_robot", "robot_2wd.urdf.xacro")
    rviz2_config = os.path.join(get_package_share_directory("core_description"), "rviz", "rviz_config.rviz")
    robot_desc = ParameterValue(Command(['xacro ', urdf_file]), value_type=str)
    rsp_node = Node(
        package="robot_state_publisher",
        executable="robot_state_publisher",
        name="rsp",
        parameters=[{"robot_description": robot_desc}]
    )
    jsp_gui_node = Node(
        package="joint_state_publisher_gui",
        executable="joint_state_publisher_gui",
        name="jsp_gui",
    )
    rviz2_node = Node(
        package="rviz2",
        executable="rviz2",
        name="rviz2",
        arguments=['-d', rviz2_config]
    )

    return LaunchDescription([
        rsp_node,
        jsp_gui_node, 
        rviz2_node       

    ])