#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from std_msgs.msg import Bool
from rcl_interfaces.msg import Log

class GoalReachedBridge(Node):
    def __init__(self):
        super().__init__('goal_reached_bridge')
        self.pub = self.create_publisher(Bool, '/goal_reached', 10)

        # Subscribe to /rosout for logs
        self.sub = self.create_subscription(
            Log,
            '/rosout',
            self.rosout_callback,
            10
        )
        self.get_logger().info("Listening to /rosout for 'Goal succeeded' messages...")

    def rosout_callback(self, msg: Log):
        # Check if this log is from bt_navigator or controller_server
        if msg.name in ["bt_navigator", "controller_server"]:
            if "Goal succeeded" in msg.msg or "Reached the goal!" in msg.msg:
                self.get_logger().info("Detected goal success! Publishing /goal_reached=True")
                self.pub.publish(Bool(data=True))

def main(args=None):
    rclpy.init(args=args)
    node = GoalReachedBridge()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()

if __name__ == '__main__':
    main()
