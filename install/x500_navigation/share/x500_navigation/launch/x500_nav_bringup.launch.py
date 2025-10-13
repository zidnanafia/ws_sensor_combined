#!/usr/bin/env python3
import os
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    IncludeLaunchDescription,
    TimerAction,
    LogInfo
)
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory


def generate_launch_description():
    pkg_x500_nav = get_package_share_directory('x500_navigation')
    pkg_nav2 = get_package_share_directory('nav2_bringup')

    # === Arguments ===
    map_file = LaunchConfiguration(
        'map',
        default=os.path.join(pkg_x500_nav, 'maps', 'navigation_map.yaml')
    )

    params_file = LaunchConfiguration(
        'params_file',
        default=os.path.join(pkg_x500_nav, 'params', 'nav2_params.yaml')
    )

    use_sim_time = LaunchConfiguration('use_sim_time', default='true')
    rviz_config_dir = os.path.join(pkg_x500_nav, 'rviz', 'x500_nav.rviz')

    # === Nav2 Bringup ===
    nav2_bringup = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_nav2, 'launch', 'bringup_launch.py')
        ),
        launch_arguments={
            'map': map_file,
            'use_sim_time': use_sim_time,
            'params_file': params_file,
        }.items(),
    )

    # Delay Nav2 bringup to avoid TF / clock mismatch with Gazebo or PX4
    delayed_nav2_bringup = TimerAction(
        period=5.0,  # seconds
        actions=[
            LogInfo(msg="⏳ Waiting for simulation clock and TF to stabilize..."),
            nav2_bringup,
        ],
    )

    # === RViz Node ===
    rviz = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_config_dir],
        parameters=[{'use_sim_time': use_sim_time}],
        output='screen',
    )

    # === Optional cmd_vel bridge ===
    cmd_vel_bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        name='cmd_vel_bridge',
        arguments=['/cmd_vel@geometry_msgs/msg/Twist@gz.msgs.Twist'],
        parameters=[{'use_sim_time': use_sim_time}],
        output='screen',
    )

    cmd_vel_to_offboard = Node(
        package='x500_navigation',
        executable='cmd_vel_to_offboard',
        name='cmd_vel_to_offboard',
        output='screen'
    )


    return LaunchDescription([
        DeclareLaunchArgument('map', default_value=map_file),
        DeclareLaunchArgument('params_file', default_value=params_file),
        DeclareLaunchArgument('use_sim_time', default_value='true'),
        LogInfo(msg="🚀 Launching X500 Navigation Stack with simulated time..."),
        delayed_nav2_bringup,
        rviz,
        # Uncomment if you want the /cmd_vel bridge active:
        # cmd_vel_bridge,
        # cmd_vel_to_offboard,
    ])
