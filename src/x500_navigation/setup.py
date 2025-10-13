from setuptools import find_packages, setup

package_name = 'x500_navigation'

setup(
    name=package_name,
    version='0.0.0',
    packages=find_packages(exclude=['test']),
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        ('share/' + package_name + '/launch', [
            'launch/x500_nav_bringup.launch.py',
            'launch/slam_nav2.launch.py',
        ]),
        ('share/' + package_name + '/params', ['params/nav2_params.yaml']),
        ('share/' + package_name + '/maps', [
            'maps/navigation_map2.yaml',
            'maps/navigation_map2.pgm'
        ]),
        ('share/' + package_name + '/rviz', ['rviz/x500_nav.rviz']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='imamalghifary',
    maintainer_email='imamalghifary@todo.todo',
    description='Navigation setup for X500 with Nav2 and PX4 odometry bridge',
    license='TODO: License declaration',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'cmd_vel_to_offboard = scripts.cmd_vel_to_offboard:main',
        ],
    },
)
