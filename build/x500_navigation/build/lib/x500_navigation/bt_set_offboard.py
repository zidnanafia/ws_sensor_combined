#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from px4_msgs.msg import VehicleCommand
from rclpy.qos import QoSProfile, ReliabilityPolicy, HistoryPolicy
import time

class BtSetOffboard(Node):
    def __init__(self):
        super().__init__('bt_set_offboard')
        qos = QoSProfile(
            reliability=ReliabilityPolicy.BEST_EFFORT,
            history=HistoryPolicy.KEEP_LAST,
            depth=10
        )
        self.cmd_pub = self.create_publisher(VehicleCommand, '/fmu/in/vehicle_command', qos)
        self.timer = self.create_timer(0.5, self.loop)
        self.start_time = time.time()
        self.sent = False
        self.get_logger().info('🧭 BT Offboard activation node initialized.')

    def loop(self):
        # Send the offboard mode command once
        if not self.sent:
            self.set_offboard_mode()
            self.get_logger().info('🛰️ Sending PX4 OFFBOARD mode command...')
            self.sent = True
        elif time.time() - self.start_time > 2.0:
            self.get_logger().info('✅ OFFBOARD mode activation sequence complete.')
            rclpy.shutdown()

    def set_offboard_mode(self):
        """Send MAV_CMD_DO_SET_MODE to enable OFFBOARD mode on PX4."""
        msg = VehicleCommand()
        msg.command = VehicleCommand.VEHICLE_CMD_DO_SET_MODE
        msg.param1 = 1.0   # custom mode
        msg.param2 = 6.0   # PX4_MODE_OFFBOARD (MAV_MODE_FLAG_CUSTOM_MODE_ENABLED)
        msg.target_system = 1
        msg.target_component = 1
        msg.source_system = 1
        msg.source_component = 1
        msg.from_external = True
        self.cmd_pub.publish(msg)

def main(args=None):
    rclpy.init(args=args)
    node = BtSetOffboard()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
