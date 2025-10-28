#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from px4_msgs.msg import VehicleCommand
from rclpy.clock import Clock
import time

class ChangeAltitudeNode(Node):
    def __init__(self):
        super().__init__('bt_change_altitude')
        self.pub = self.create_publisher(VehicleCommand, '/fmu/in/vehicle_command', 10)
        self.timer = self.create_timer(1.0, self.change_alt)
        self.get_logger().info('✅ ChangeAltitude node ready')

    def publish_vehicle_command(self, command, param1=0.0, param2=0.0, param7=0.0):
        msg = VehicleCommand()
        msg.param1 = param1
        msg.param2 = param2
        msg.param7 = param7
        msg.command = command
        msg.target_system = 1
        msg.target_component = 1
        msg.source_system = 1
        msg.source_component = 1
        msg.from_external = True
        msg.timestamp = int(Clock().now().nanoseconds / 1000)
        self.pub.publish(msg)

    def change_alt(self):
        self.get_logger().info('⬆️ Changing altitude slightly (-0.5 m)')
        self.publish_vehicle_command(VehicleCommand.VEHICLE_CMD_NAV_TAKEOFF, 0.0, 0.0, 1.5)

def main():
    rclpy.init()
    node = ChangeAltitudeNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
