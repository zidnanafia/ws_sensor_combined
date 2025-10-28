#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from rclpy.qos import QoSProfile, ReliabilityPolicy, DurabilityPolicy, HistoryPolicy

class CmdVelToOffboard(Node):
    def __init__(self):
        super().__init__('cmd_vel_to_offboard_bridge')

        qos_profile = QoSProfile(
            reliability=ReliabilityPolicy.RELIABLE,
            durability=DurabilityPolicy.TRANSIENT_LOCAL,
            history=HistoryPolicy.KEEP_LAST,
            depth=10
        )

        self.sub = self.create_subscription(
            Twist,
            '/cmd_vel_nav',
            self.cmd_cb,
            10  
        )

        self.pub = self.create_publisher(Twist, '/offboard_velocity_cmd', qos_profile)

        self.get_logger().info("Bridging /cmd_vel to /offboard_velocity_cmd (with axis remap)")

    def cmd_cb(self, msg: Twist):
        out = Twist()

        out.linear.x = float(msg.linear.y)
        out.linear.y = float(msg.linear.x)
        out.linear.z = float(msg.linear.z)

        out.angular.z = float(-msg.angular.z)

        self.pub.publish(out)


def main(args=None):
    rclpy.init(args=args)
    node = CmdVelToOffboard()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
