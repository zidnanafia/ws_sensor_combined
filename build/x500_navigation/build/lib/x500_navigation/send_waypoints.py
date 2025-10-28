#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from rclpy.action import ActionClient
from geometry_msgs.msg import PoseStamped
from nav2_msgs.action import NavigateThroughPoses

class SendWaypoints(Node):
    def __init__(self):
        super().__init__('send_waypoints')
        self.client = ActionClient(self, NavigateThroughPoses, 'navigate_through_poses')
        self.get_logger().info("🚀 Waiting for NavigateThroughPoses action server...")
        self.client.wait_for_server()
        self.send_waypoints()

    def send_waypoints(self):
        # Create waypoint list (PoseStamped)
        waypoints = []

        # 🟩 Example Waypoint 1
        pose1 = PoseStamped()
        pose1.header.frame_id = 'map'
        pose1.pose.position.x = 1.0
        pose1.pose.position.y = 0.0
        pose1.pose.orientation.w = 1.0
        waypoints.append(pose1)

        # 🟩 Example Waypoint 2
        pose2 = PoseStamped()
        pose2.header.frame_id = 'map'
        pose2.pose.position.x = 2.0
        pose2.pose.position.y = -1.0
        pose2.pose.orientation.w = 1.0
        waypoints.append(pose2)

        goal_msg = NavigateThroughPoses.Goal()
        goal_msg.poses = waypoints

        self.get_logger().info("📡 Sending waypoints to Nav2...")
        send_goal_future = self.client.send_goal_async(goal_msg)
        send_goal_future.add_done_callback(self.goal_response_callback)

    def goal_response_callback(self, future):
        goal_handle = future.result()
        if not goal_handle.accepted:
            self.get_logger().error("❌ Waypoint goal rejected.")
            return
        self.get_logger().info("✅ Waypoint goal accepted.")
        result_future = goal_handle.get_result_async()
        result_future.add_done_callback(self.result_callback)

    def result_callback(self, future):
        result = future.result().result
        self.get_logger().info(f"🏁 Mission complete! Result: {result.result}")
        rclpy.shutdown()

def main(args=None):
    rclpy.init(args=args)
    node = SendWaypoints()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
