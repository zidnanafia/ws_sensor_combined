#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy, HistoryPolicy
from geometry_msgs.msg import TransformStamped
from px4_msgs.msg import VehicleOdometry
from nav_msgs.msg import Odometry
from tf_transformations import quaternion_from_euler, euler_from_quaternion
from tf2_ros import TransformBroadcaster


class OdomBridge2D(Node):
    def __init__(self):
        super().__init__('odom_bridge_2d')

        # Parameters
        self.declare_parameter('odom_topic', '/odom')
        self.declare_parameter('odom_frame', 'odom')
        self.declare_parameter('base_frame', 'base_link')
        self.declare_parameter('publish_tf', True)  # Enable TF broadcasting

        self.odom_topic = self.get_parameter('odom_topic').value
        self.odom_frame = self.get_parameter('odom_frame').value
        self.base_frame = self.get_parameter('base_frame').value
        self.publish_tf = self.get_parameter('publish_tf').value

        # PX4 QoS (BEST_EFFORT)
        qos_profile = QoSProfile(
            reliability=ReliabilityPolicy.BEST_EFFORT,
            history=HistoryPolicy.KEEP_LAST,
            depth=10
        )

        # Subscribers and publishers
        self.odom_sub = self.create_subscription(
            VehicleOdometry,
            '/fmu/out/vehicle_odometry',
            self.odom_callback,
            qos_profile
        )

        self.odom_pub = self.create_publisher(Odometry, self.odom_topic, 10)
        self.tf_broadcaster = TransformBroadcaster(self)

        self.get_logger().info(
            f"Odom bridge (2D) started: PX4 → {self.odom_topic} "
            f"(TF: {self.odom_frame} -> {self.base_frame})"
        )

    def odom_callback(self, msg: VehicleOdometry):
        # PX4 NED → ROS ENU
        x = float(msg.position[0])
        y = float(-msg.position[1])
        z = 0.0  # Flatten for 2D

        qw = float(msg.q[0])
        qx = float(msg.q[1])
        qy = float(-msg.q[2])
        qz = float(-msg.q[3])

        # Extract yaw only
        roll, pitch, yaw = euler_from_quaternion((qx, qy, qz, qw))
        qyaw = quaternion_from_euler(0.0, 0.0, yaw)

        # Build Odometry message
        odom = Odometry()
        odom.header.stamp = self.get_clock().now().to_msg()
        odom.header.frame_id = self.odom_frame
        odom.child_frame_id = self.base_frame

        odom.pose.pose.position.x = x
        odom.pose.pose.position.y = y
        odom.pose.pose.position.z = z

        odom.pose.pose.orientation.x = qyaw[0]
        odom.pose.pose.orientation.y = qyaw[1]
        odom.pose.pose.orientation.z = qyaw[2]
        odom.pose.pose.orientation.w = qyaw[3]

        # Planar velocities
        odom.twist.twist.linear.x = float(msg.velocity[0])
        odom.twist.twist.linear.y = float(-msg.velocity[1])
        odom.twist.twist.linear.z = 0.0

        odom.twist.twist.angular.z = float(msg.angular_velocity[2])

        # Covariances
        odom.pose.covariance = [0.0]*36
        odom.pose.covariance[0] = 0.05
        odom.pose.covariance[7] = 0.05
        odom.pose.covariance[35] = 0.05

        odom.twist.covariance = [0.0]*36
        odom.twist.covariance[0] = 0.02
        odom.twist.covariance[7] = 0.02
        odom.twist.covariance[35] = 0.02

        # Publish Odometry
        self.odom_pub.publish(odom)

        # Publish TF: odom -> base_footprint
        if self.publish_tf:
            t = TransformStamped()
            t.header.stamp = odom.header.stamp
            t.header.frame_id = self.odom_frame
            t.child_frame_id = self.base_frame
            t.transform.translation.x = x
            t.transform.translation.y = y
            t.transform.translation.z = z
            t.transform.rotation.x = qyaw[0]
            t.transform.rotation.y = qyaw[1]
            t.transform.rotation.z = qyaw[2]
            t.transform.rotation.w = qyaw[3]
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