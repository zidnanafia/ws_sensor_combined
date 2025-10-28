#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
import subprocess
import time
from rclpy.clock import Clock

from px4_msgs.msg import VehicleCommand

class StopOffboardNode(Node):
    def __init__(self):
        super().__init__('bt_stop_offboard')
        self.vehicle_command_pub = self.create_publisher(VehicleCommand, "/fmu/in/vehicle_command", 10)
        self.get_logger().info("✅ StopOffboard BT Node initialized.")

    def publish_vehicle_command(self, command, param1=0.0, param2=0.0):
        """Send PX4 vehicle command."""
        msg = VehicleCommand()
        msg.param1 = param1
        msg.param2 = param2
        msg.command = command
        msg.target_system = 1
        msg.target_component = 1
        msg.source_system = 1
        msg.source_component = 1
        msg.from_external = True
        msg.timestamp = int(Clock().now().nanoseconds / 1000)
        self.vehicle_command_pub.publish(msg)

    def tick(self):
        """Executed by Behavior Tree before Land step."""
        try:
            self.get_logger().info("🛑 Attempting to stop /px4_offboard/velocity node...")
            subprocess.run(["ros2", "node", "kill", "/px4_offboard/velocity"], check=False)

            time.sleep(0.5)
            self.get_logger().info("Switching PX4 mode to HOLD before landing...")
            self.publish_vehicle_command(VehicleCommand.VEHICLE_CMD_DO_SET_MODE, 1.0, 0.0)  # mode: hold/manual

            time.sleep(1.0)
            self.get_logger().info("✅ Offboard stopped successfully.")
            return "SUCCESS"
        except Exception as e:
            self.get_logger().error(f"❌ Failed to stop offboard: {e}")
            return "FAILURE"

def main(args=None):
    rclpy.init(args=args)
    node = StopOffboardNode()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
