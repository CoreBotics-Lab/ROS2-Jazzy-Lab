from setuptools import find_packages, setup

package_name = 'ros2_utilities'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='root',
    maintainer_email='hayisyed@gmail.com',
    description='A collection of reusable ROS 2 utility nodes and PyQt6 GUIs (e.g., virtual joystick) for testing, teleoperation, and system debugging.',
    license='Apache-2.0',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
            'joy_gui = ros2_utilities.joy_gui:main',
        ],
    },
)
