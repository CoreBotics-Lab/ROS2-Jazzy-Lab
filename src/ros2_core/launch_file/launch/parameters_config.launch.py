import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    # Find the path to the 'parameters' package share directory
    parameters_pkg_share_dir = get_package_share_directory('parameters')
    
    # Construct the full path to the YAML configuration file
    config_file_path = os.path.join(parameters_pkg_share_dir, 'config', 'param_config.yaml')

    return LaunchDescription([
        Node(
            package='parameters',
            executable='py_parameters_from_config.py',
            name='parameter_from_config_node', # This must exactly match the top-level key in your YAML file!
            output='screen',
            parameters=[config_file_path] # This is the Python equivalent of --params-file
        )
    ])