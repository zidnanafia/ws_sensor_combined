#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist

class CmdVelBridge(Node):
    def __init__(self):
        super().__init__('cmd_vel_to_offboard_bridge')
        self.publisher = self.create_publisher(Twist, '/offboard_velocity_cmd', 10)
        self.subscriber = self.create_subscription(Twist, '/cmd_vel', self.callback, 10)
        self.get_logger().info('Bridging /cmd_vel → /offboard_velocity_cmd')

    def callback(self, msg):
        self.publisher.publish(msg)

def main(args=None):
    rclpy.init(args=args)
    node = CmdVelBridge()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
