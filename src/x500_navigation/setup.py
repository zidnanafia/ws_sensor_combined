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
        ('share/' + package_name + '/params', ['params/nav2_bt_navigator.yaml']),
        ('share/' + package_name + '/maps', [
            'maps/navigation_map3.yaml',
            'maps/navigation_map3.pgm'
        ]),
        ('share/' + package_name + '/rviz', ['rviz/x500_nav.rviz']),
        ('share/' + package_name + '/behavior_trees', ['behavior_trees/simple_uav_test.xml']),
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
            'cmd_vel_to_offboard = x500_navigation.cmd_vel_to_offboard:main',
            'nav2_goal_monitor = x500_navigation.nav2_goal_monitor:main',
            'send_waypoints = x500_navigation.send_waypoints:main',
            'bt_takeoff = x500_navigation.bt_takeoff:main',
            'bt_change_altitude = x500_navigation.bt_change_altitude:main',
            'bt_land = x500_navigation.bt_land:main',
            'bt_stop_offboard = x500_navigation.bt_stop_offboard:main',
            'bt_set_offboard = x500_navigation.bt_stop_offboard:main',
        ],
    },
)
