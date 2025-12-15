from setuptools import find_packages, setup

package_name = 'x500_bridge'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        ('share/' + package_name + '/launch', ['launch/slam_with_bridge.launch.py']),
        ('share/' + package_name + '/urdf', ['urdf/x500_lidar.urdf.xacro']), 
        ('share/' + package_name + '/config', ['config/ekf.yaml']),
        ('share/' + package_name + '/rviz', ['rviz/x500_slam.rviz']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='imamalghifary',
    maintainer_email='imamalghifary@todo.todo',
    description='TODO: Package description',
    license='TODO: License declaration',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'odom_bridge = x500_bridge.odom_bridge:main',
            'scan_relay = x500_bridge.scan_relay:main',
        ],
    },
)
