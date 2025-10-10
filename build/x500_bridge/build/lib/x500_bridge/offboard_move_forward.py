#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from px4_msgs.msg import VehicleCommand

class OffboardArm(Node):
    def __init__(self):
        super().__init__('offboard_arm_helper')
        self.pub = self.create_publisher(VehicleCommand, '/fmu/in/vehicle_command', 10)
        self.timer = self.create_timer(1.0, self.sequence)
        self.step = 0
        self.get_logger().info("🚀 Offboard arming helper started")

    def sequence(self):
        msg = VehicleCommand()
        msg.target_system = 1
        msg.target_component = 1
        msg.source_system = 1
        msg.source_component = 1
        msg.from_external = True

        if self.step == 0:
            msg.command = 176  # NAV_CMD_DO_SET_MODE → OFFBOARD
            msg.param1 = 1.0
            self.get_logger().info("➡️ Sending Offboard mode command (176)")
        elif self.step == 1:
            msg.command = 400  # VEHICLE_CMD_COMPONENT_ARM_DISARM
            msg.param1 = 1.0
            self.get_logger().info("🟢 Sending Arm command (400)")
        else:
            self.get_logger().info("✅ Sequence done")
            rclpy.shutdown()
            return

        self.pub.publish(msg)
        self.step += 1

def main(args=None):
    rclpy.init(args=args)
    node = OffboardArm()
    rclpy.spin(node)

if __name__ == '__main__':
    main()
