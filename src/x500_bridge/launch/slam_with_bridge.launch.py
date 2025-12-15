#!/usr/bin/env python3
import os
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, Command
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from ament_index_python.packages import get_package_share_directory
from launch.actions import TimerAction


def generate_launch_description():
    pkg_share = get_package_share_directory('x500_bridge')

    urdf_file = os.path.join(pkg_share, 'urdf', 'x500_lidar.urdf.xacro')
    ekf_config = os.path.join(pkg_share, 'config', 'ekf.yaml')

    use_sim_time = LaunchConfiguration('use_sim_time', default='true')
    rviz_config_dir = os.path.join(pkg_share, 'rviz', 'x500_slam.rviz')

    # === Nodes ===
    # 1️⃣ Robot State Publisher
    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
        parameters=[{
            'use_sim_time': use_sim_time,
            'robot_description': ParameterValue(
                Command(['xacro ', urdf_file]), value_type=str)
        }],
    )

    # 2️⃣ Gazebo Bridges
    lidar_bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        name='lidar_bridge',
        arguments=[
            '/world/warehouse/model/x500_lidar_2d_0/link/link/sensor/lidar_2d_v2/scan' +
            '@sensor_msgs/msg/LaserScan@gz.msgs.LaserScan'
        ],
        remappings=[(
            '/world/warehouse/model/x500_lidar_2d_0/link/link/sensor/lidar_2d_v2/scan',
            '/scan'
        )],
        parameters=[{'use_sim_time': use_sim_time}],
        output='screen'
    )

    clock_bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        name='clock_bridge',
        arguments=['/world/warehouse/clock@rosgraph_msgs/msg/Clock@gz.msgs.Clock'],
        remappings=[('/world/warehouse/clock', '/clock')],
        output='screen'
    )

    odom_bridge = Node(
        package='x500_bridge',
        executable='odom_bridge',
        name='odom_bridge',
        output='screen',
        parameters=[{'use_sim_time': use_sim_time}],
    )

    scan_relay = Node(
        package='x500_bridge',
        executable='scan_relay',
        name='scan_relay',
        output='screen',
        parameters=[{'use_sim_time': use_sim_time}],
    )

    # 3️⃣ EKF Node (robot_localization)
    ekf_node = Node(
        package='robot_localization',
        executable='ekf_node',
        name='ekf_filter_node',
        output='screen',
        parameters=[ekf_config, {'use_sim_time': use_sim_time}],
    )

    rviz = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_config_dir],
        parameters=[{'use_sim_time': use_sim_time}],
        output='screen',
    )

    # SLAM Toolbox
    slam_toolbox = TimerAction(
        period=5.0,
        actions=[
            Node(
                package='slam_toolbox',
                executable='async_slam_toolbox_node',
                name='slam_toolbox',
                parameters=[{
                    'use_sim_time': True,
                    'slam_params_file': '/opt/ros/humble/share/slam_toolbox/config/mapper_params_online_async.yaml',
                    'odom_frame': 'odom',
                    'map_frame': 'map',
                    'base_frame': 'base_link',
                    'scan_topic': '/scan_fixed',
                    'use_odometry': True,
                }],
                output='screen'
            )
        ]
    )


    return LaunchDescription([
        DeclareLaunchArgument(
            name='use_sim_time',
            default_value='true',
            description='Use simulation (Gazebo) clock if true'
        ),
        robot_state_publisher,
        lidar_bridge,
        clock_bridge,
        odom_bridge,
        scan_relay,
        ekf_node,
        # slam_toolbox,
        # rviz,
    ])