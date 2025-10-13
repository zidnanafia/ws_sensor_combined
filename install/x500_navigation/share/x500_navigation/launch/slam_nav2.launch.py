from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.conditions import IfCondition, UnlessCondition
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    use_sim_time = LaunchConfiguration('use_sim_time')
    params_file = LaunchConfiguration('params_file')
    slam = LaunchConfiguration('slam')
    map_yaml_file = LaunchConfiguration('map')

    # Declare arguments
    declare_use_sim_time = DeclareLaunchArgument(
        'use_sim_time', default_value='true', description='Use simulation time (Gazebo) if true')

    declare_params_file = DeclareLaunchArgument(
        'params_file',
        default_value=PathJoinSubstitution([
            FindPackageShare('x500_navigation'),
            'config',
            'nav2_params.yaml'
        ]),
        description='Full path to the ROS2 parameters file to use'
    )

    declare_slam = DeclareLaunchArgument(
        'slam', default_value='True', description='Whether to run SLAM'
    )

    declare_map_yaml = DeclareLaunchArgument(
        'map',
        default_value='',
        description='Full path to map yaml file (only used when slam=False)'
    )

    # Directories
    slam_toolbox_dir = FindPackageShare('slam_toolbox')
    nav2_bringup_dir = FindPackageShare('nav2_bringup')

    # SLAM Toolbox (online async mode)
    slam_toolbox_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([slam_toolbox_dir, 'launch', 'online_async_launch.py'])
        ),
        condition=IfCondition(slam),
        launch_arguments={'use_sim_time': use_sim_time}.items(),
    )

    # Nav2 navigation stack — directly use bringup_launch.py *only* if map mode
    nav2_with_map_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([nav2_bringup_dir, 'launch', 'bringup_launch.py'])
        ),
        condition=UnlessCondition(slam),
        launch_arguments={
            'use_sim_time': use_sim_time,
            'map': map_yaml_file,
            'params_file': params_file
        }.items(),
    )

    # Nav2 core without map argument (for SLAM)
    nav2_nodes_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            PathJoinSubstitution([nav2_bringup_dir, 'launch', 'navigation_launch.py'])
        ),
        condition=IfCondition(slam),
        launch_arguments={
            'use_sim_time': use_sim_time,
            'params_file': params_file
        }.items(),
    )

    return LaunchDescription([
        declare_use_sim_time,
        declare_params_file,
        declare_slam,
        declare_map_yaml,
        slam_toolbox_launch,
        nav2_with_map_launch,
        nav2_nodes_launch
    ])
