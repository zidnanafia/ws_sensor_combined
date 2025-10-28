#!/usr/bin/env python3
import time
import rclpy
from rclpy.node import Node

class DummyLand(Node):
    def __init__(self):
        super().__init__('bt_land_dummy')
        self.get_logger().info('🛬 Landing...')
        time.sleep(2)
        self.get_logger().info('✅ Landed successfully!')

def main():
    rclpy.init()
    node = DummyLand()
    rclpy.spin_once(node, timeout_sec=1)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
