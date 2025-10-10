from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription, DeclareLaunchArgument, ExecuteProcess
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch_ros.actions import Node
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    # Paths
    gazebo_ros = get_package_share_directory('gazebo_ros')
    model_path = os.path.expanduser('~/PX4-Autopilot/Tools/simulation/gazebo-classic/models/iris_rplidar/iris_rplidar.sdf'),
    urdf_path = os.path.expanduser('~/PX4-Autopilot/Tools/simulation/gazebo-classic/models/iris_rplidar/iris_rplidar.urdf')

    # Launch Gazebo with warehouse world
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(gazebo_ros, 'launch', 'gazebo.launch.py')
        ),
        launch_arguments={
            'world': os.path.expanduser('~/PX4-Autopilot/Tools/simulation/gazebo-classic/worlds/warehouse.world'),
        }.items()
    )

    # Spawn entity in Gazebo
    spawn_entity = Node(
        package='gazebo_ros',
        executable='spawn_entity.py',
        arguments=['-file', model_path,
                   '-entity', 'iris_rplidar',
                   '-x', '0', '-y', '0', '-z', '0.1'],
        output='screen'
    )

    # Robot state publisher
    robot_state_pub = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
        parameters=[{'use_sim_time': True}],
        arguments=[urdf_path]
    )

    # (Optional) Static TF odom->base_link (if PX4 does not publish odom)
    static_tf = Node(
        package='tf2_ros',
        executable='static_transform_publisher',
        arguments=['0', '0', '0', '0', '0', '0', 'odom', 'base_link'],
        parameters=[{'use_sim_time': True}],
        output='screen'
    )

    # RViz with config (you can save your own RViz config as .rviz)
    rviz_config = os.path.expanduser('~/iris_rplidar.rviz')
    rviz = Node(
        package='rviz2',
        executable='rviz2',
        arguments=['-d', rviz_config],
        parameters=[{'use_sim_time': True}],
        output='screen'
    )

    return LaunchDescription([
        gazebo,
        spawn_entity,
        robot_state_pub,
        static_tf,
        rviz
    ])
