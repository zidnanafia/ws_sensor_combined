# #!/usr/bin/env python3
# import rclpy
# from rclpy.node import Node
# from rclpy.qos import QoSProfile, ReliabilityPolicy, HistoryPolicy

# from px4_msgs.msg import VehicleOdometry
# from nav_msgs.msg import Odometry
# from geometry_msgs.msg import TransformStamped
# import tf2_ros


# class OdomBridge(Node):
#     def __init__(self):
#         super().__init__('odom_bridge')

#         # Match PX4 QoS
#         qos_profile = QoSProfile(
#             reliability=ReliabilityPolicy.BEST_EFFORT,
#             history=HistoryPolicy.KEEP_LAST,
#             depth=10
#         )

#         # Subscriber: PX4 odometry
#         self.odom_sub = self.create_subscription(
#             VehicleOdometry,
#             '/fmu/out/vehicle_odometry',
#             self.odom_callback,
#             qos_profile
#         )

#         # Publisher: ROS2 Odometry
#         self.odom_pub = self.create_publisher(Odometry, '/odom', 10)

#         # TF broadcaster
#         self.tf_broadcaster = tf2_ros.TransformBroadcaster(self)

#         self.get_logger().info("Odom bridge started (PX4 → ROS2 + TF)")

#     def odom_callback(self, msg: VehicleOdometry):
#         odom = Odometry()
#         odom.header.stamp = self.get_clock().now().to_msg()
#         odom.header.frame_id = 'odom'
#         odom.child_frame_id = 'base_link'

#         # --- PX4 NED → ROS ENU conversion ---
#         # Position
#         odom.pose.pose.position.x = float(msg.position[0])
#         odom.pose.pose.position.y = float(-msg.position[1])   # flip Y
#         odom.pose.pose.position.z = float(-msg.position[2])   # flip Z

#         # Orientation (PX4 gives [w, x, y, z] in NED)
#         # Convert to ROS ENU
#         odom.pose.pose.orientation.w = float(msg.q[0])
#         odom.pose.pose.orientation.x = float(msg.q[1])
#         odom.pose.pose.orientation.y = float(-msg.q[2])
#         odom.pose.pose.orientation.z = float(-msg.q[3])

#         # Velocities
#         odom.twist.twist.linear.x = float(msg.velocity[0])
#         odom.twist.twist.linear.y = float(-msg.velocity[1])
#         odom.twist.twist.linear.z = float(-msg.velocity[2])

#         odom.twist.twist.angular.x = float(msg.angular_velocity[0])
#         odom.twist.twist.angular.y = float(msg.angular_velocity[1])
#         odom.twist.twist.angular.z = float(msg.angular_velocity[2])

#         # Publish Odometry
#         self.odom_pub.publish(odom)

#         # # Broadcast TF (odom -> base_link)
#         # t = TransformStamped()
#         # t.header.stamp = odom.header.stamp
#         # t.header.frame_id = 'odom'
#         # t.child_frame_id = 'base_link'
#         # t.transform.translation.x = odom.pose.pose.position.x
#         # t.transform.translation.y = odom.pose.pose.position.y
#         # t.transform.translation.z = odom.pose.pose.position.z
#         # t.transform.rotation = odom.pose.pose.orientation

#         # self.tf_broadcaster.sendTransform(t)


# def main(args=None):
#     rclpy.init(args=args)
#     node = OdomBridge()
#     try:
#         rclpy.spin(node)
#     except KeyboardInterrupt:
#         pass
#     finally:
#         node.destroy_node()
#         rclpy.shutdown()


# if __name__ == '__main__':
#     main()

#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy, HistoryPolicy

from px4_msgs.msg import VehicleOdometry
from nav_msgs.msg import Odometry
from geometry_msgs.msg import TransformStamped
import tf2_ros
import math
from tf_transformations import quaternion_from_euler, euler_from_quaternion

class OdomBridge2D(Node):
    def __init__(self):
        super().__init__('odom_bridge_2d')
        # allow use_sim_time param
        # self.declare_parameter('use_sim_time', True)

        # Match PX4 QoS
        qos_profile = QoSProfile(
            reliability=ReliabilityPolicy.BEST_EFFORT,
            history=HistoryPolicy.KEEP_LAST,
            depth=10
        )

        # Subscriber: PX4 odometry
        self.odom_sub = self.create_subscription(
            VehicleOdometry,
            '/fmu/out/vehicle_odometry',
            self.odom_callback,
            qos_profile
        )

        # Publisher: ROS2 Odometry (planar)
        self.odom_pub = self.create_publisher(Odometry, '/odom', 10)

        # TF broadcaster
        self.tf_broadcaster = tf2_ros.TransformBroadcaster(self)

        self.get_logger().info("Odom bridge (2D) started (PX4 → planar /odom + TF)")

    def odom_callback(self, msg: VehicleOdometry):
        # Convert position: PX4 uses NED; convert to ROS ENU
        x = float(msg.position[0])
        y = float(-msg.position[1])
        z = float(-msg.position[2])  # keep but we'll flatten

        # Convert quaternion: PX4 q = [w, x, y, z] (NED) -> convert signs for ENU
        qw = float(msg.q[0])
        qx = float(msg.q[1])
        qy = float(-msg.q[2])
        qz = float(-msg.q[3])

        # Extract yaw from quaternion and keep only yaw (flatten roll/pitch)
        # euler: roll, pitch, yaw
        roll, pitch, yaw = euler_from_quaternion((qx, qy, qz, qw))
        # Build quaternion for yaw only
        qyaw = quaternion_from_euler(0.0, 0.0, yaw)  # returns (x,y,z,w)

        # Prepare Odometry message (planar)
        odom = Odometry()
        odom.header.stamp = self.get_clock().now().to_msg()
        odom.header.frame_id = 'odom'
        odom.child_frame_id = 'base_link'

        odom.pose.pose.position.x = x
        odom.pose.pose.position.y = y
        odom.pose.pose.position.z = 0.0  # flattened to ground

        odom.pose.pose.orientation.x = qyaw[0]
        odom.pose.pose.orientation.y = qyaw[1]
        odom.pose.pose.orientation.z = qyaw[2]
        odom.pose.pose.orientation.w = qyaw[3]

        # Velocities: keep planar linear x,y and yaw rate as angular z
        odom.twist.twist.linear.x = float(msg.velocity[0])
        odom.twist.twist.linear.y = float(-msg.velocity[1])
        odom.twist.twist.linear.z = 0.0

        # For angular velocity, set only z (yaw rate). PX4 angular_velocity is body rates (p,q,r)
        # Approximate yaw_rate by r (msg.angular_velocity[2]) but check sign/prefix if needed
        odom.twist.twist.angular.x = 0.0
        odom.twist.twist.angular.y = 0.0
        odom.twist.twist.angular.z = float(msg.angular_velocity[2])

        # Publish Odometry
        self.odom_pub.publish(odom)

        # Broadcast TF (odom -> base_link) using same planar pose
        t = TransformStamped()
        t.header.stamp = odom.header.stamp
        t.header.frame_id = 'odom'
        t.child_frame_id = 'base_link'
        t.transform.translation.x = odom.pose.pose.position.x
        t.transform.translation.y = odom.pose.pose.position.y
        t.transform.translation.z = 0.0
        t.transform.rotation = odom.pose.pose.orientation

        self.tf_broadcaster.sendTransform(t)


def main(args=None):
    rclpy.init(args=args)
    node = OdomBridge2D()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
