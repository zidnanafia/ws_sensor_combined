#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from sensor_msgs.msg import LaserScan
import copy

class ScanRelay(Node):
    def __init__(self):
        super().__init__('scan_relay')

        # Subscriber for raw scan
        self.scan_sub = self.create_subscription(
            LaserScan,
            '/scan',
            self.scan_callback,
            100
        )

        # Publisher for fixed scan topic
        self.scan_pub = self.create_publisher(LaserScan, '/scan_fixed', 100)

        self.get_logger().info("Scan relay started: /scan → /scan_fixed (frame_id = lidar_link)")

    def scan_callback(self, msg: LaserScan):
        # Copy message
        fixed_scan = copy.deepcopy(msg)

        # Overwrite frame_id and timestamp
        fixed_scan.header.frame_id = 'lidar_link'
        fixed_scan.header.stamp = self.get_clock().now().to_msg()

        # Publish fixed scan
        self.scan_pub.publish(fixed_scan)


def main(args=None):
    rclpy.init(args=args)
    node = ScanRelay()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()