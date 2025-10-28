#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from std_msgs.msg import Bool
from time import sleep
from rclpy.qos import QoSProfile, QoSReliabilityPolicy, QoSDurabilityPolicy, QoSHistoryPolicy


class BtTakeoff(Node):
    def __init__(self):
        super().__init__('bt_takeoff_node')

        qos_profile = QoSProfile(
            reliability=QoSReliabilityPolicy.BEST_EFFORT,
            durability=QoSDurabilityPolicy.TRANSIENT_LOCAL,
            history=QoSHistoryPolicy.KEEP_LAST,
            depth=1
        )
        
        self.pub = self.create_publisher(Bool, '/arm_message', qos_profile)

        self.get_logger().info('🛫 Sending takeoff (arming) trigger...')
        msg = Bool()
        msg.data = True
        self.pub.publish(msg)

        # Wait for PX4 FSM to detect and arm
        sleep(2.0)
        self.get_logger().info('✅ Arm message sent to PX4 FSM (velocity_control.py)')
        sleep(3.0)
        self.get_logger().info('🆙 Takeoff should now be in progress')

def main():
    rclpy.init()
    node = BtTakeoff()
    rclpy.spin_once(node, timeout_sec=1)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
