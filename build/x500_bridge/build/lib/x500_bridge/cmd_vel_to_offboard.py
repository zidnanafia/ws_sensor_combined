#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from rclpy.qos import QoSProfile, ReliabilityPolicy, DurabilityPolicy, HistoryPolicy

class CmdVelToOffboard(Node):
    def __init__(self):
        super().__init__('cmd_vel_to_offboard_bridge')

        # Match the QoS used by the PX4 offboard nodes:
        #   reliability = BEST_EFFORT
        #   durability  = TRANSIENT_LOCAL
        # This avoids the "incompatible QoS" warning.
        qos_profile = QoSProfile(
            reliability=ReliabilityPolicy.RELIABLE,
            durability=DurabilityPolicy.TRANSIENT_LOCAL,
            history=HistoryPolicy.KEEP_LAST,
            depth=10
        )

        # Subscribe to the normal ROS cmd_vel (users/scripts/tools publish here)
        self.sub = self.create_subscription(
            Twist,
            '/cmd_vel',
            self.cmd_cb,
            10  # subscription queue (deprecated alias), callback QoS ignored here
        )

        # Publish to PX4 offboard topic with the QoS PX4 expects
        self.pub = self.create_publisher(Twist, '/offboard_velocity_cmd', qos_profile)

        self.get_logger().info("🔁 Bridging /cmd_vel → /offboard_velocity_cmd (with axis remap)")

    def cmd_cb(self, msg: Twist):
        out = Twist()

        # Remap axes so that a user sending linear.x forward behaves like keyboard:
        # The PX4 offboard node (your velocity_control.py) expects incoming Twist to be in
        # a frame that it converts with:
        #   velocity.x = -msg.linear.y
        #   velocity.y =  msg.linear.x
        # We therefore send out:
        #   out.linear.x = msg.linear.y
        #   out.linear.y = -msg.linear.x
        # So publishing /cmd_vel {x:0.5} -> results in offboard velocity.x = +0.5 (forward)
        out.linear.x = float(msg.linear.y)
        out.linear.y = float(msg.linear.x)
        out.linear.z = float(msg.linear.z)

        # Keep angular z (you may invert sign if you see clockwise/counterclockwise reversed)
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
