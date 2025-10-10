#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from px4_msgs.msg import OffboardControlMode, TrajectorySetpoint, VehicleCommand
import time

class OffboardMove(Node):
    def __init__(self):
        super().__init__('offboard_move_forward')
        self.offboard_pub = self.create_publisher(OffboardControlMode, '/fmu/in/offboard_control_mode', 10)
        self.traj_pub = self.create_publisher(TrajectorySetpoint, '/fmu/in/trajectory_setpoint', 10)
        self.cmd_pub = self.create_publisher(VehicleCommand, '/fmu/in/vehicle_command', 10)

        self.timer = self.create_timer(0.1, self.timer_callback)
        self.start_time = time.time()
        self.sent_offboard = False
        self.sent_arm = False
        self.sent_move = False

    def timer_callback(self):
        now = int(self.get_clock().now().nanoseconds / 1000)

        # Publish OffboardControlMode
        mode = OffboardControlMode()
        mode.timestamp = now
        mode.position = True
        mode.velocity = False
        self.offboard_pub.publish(mode)

        # Default: hover at [0, 0, -2]
        sp = TrajectorySetpoint()
        sp.timestamp = now
        sp.position = [0.0, 0.0, -2.0]
        sp.yaw = 0.0

        t = time.time() - self.start_time

        # After 4s, move 1m forward in X direction
        if t > 4.0:
            sp.position = [1.0, 0.0, -2.0]
        self.traj_pub.publish(sp)

        # After 2s, enable offboard mode
        if t > 3.0 and not self.sent_offboard:
            self.send_vehicle_command(176, 1.0)
            self.get_logger().info('✅ Sent OFFBOARD command')
            self.sent_offboard = True

        # After 3s, arm vehicle
        if t > 2.0 and not self.sent_arm:
            self.send_vehicle_command(400, 1.0)
            self.get_logger().info('✅ Sent ARM command')
            self.sent_arm = True

    def send_vehicle_command(self, command, param1=0.0, param2=0.0):
        msg = VehicleCommand()
        msg.timestamp = int(self.get_clock().now().nanoseconds / 1000)
        msg.param1 = float(param1)
        msg.param2 = float(param2)
        msg.command = int(command)
        msg.target_system = 1
        msg.target_component = 1
        msg.source_system = 1
        msg.source_component = 1
        msg.from_external = True
        self.cmd_pub.publish(msg)

def main(args=None):
    rclpy.init(args=args)
    node = OffboardMove()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
