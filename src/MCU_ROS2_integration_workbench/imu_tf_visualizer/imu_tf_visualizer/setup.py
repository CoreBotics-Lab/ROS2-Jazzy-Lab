from setuptools import find_packages, setup

package_name = 'imu_tf_visualizer'

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
    maintainer='Syed Abdul Hayi M',
    maintainer_email='hayisyed@gmail.com',
    description='Visualizes real-time MPU6050 IMU orientation from physical hardware by publishing TF transforms, driving a URDF model in RViz2.',
    license='Apache-2.0',
    extras_require={
        'test': [
            'pytest',
        ],
    },
    entry_points={
        'console_scripts': [
            'imu_republisher_node = imu_tf_visualizer.imu_republisher_node:main',
        ],
    },
)
