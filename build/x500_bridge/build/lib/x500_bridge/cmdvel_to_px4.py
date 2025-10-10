#!/usr/bin/env python3
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from px4_msgs.msg import TrajectorySetpoint, OffboardControlMode

class CmdVelToPX4(Node):
    def __init__(self):
        super().__init__('cmdvel_to_px4')
        self.sub = self.create_subscription(Twist, '/cmd_vel', self.cmd_callback, 10)
        self.traj_pub = self.create_publisher(TrajectorySetpoint, '/fmu/in/trajectory_setpoint', 10)
        self.mode_pub = self.create_publisher(OffboardControlMode, '/fmu/in/offboard_control_mode', 10)

        self.get_logger().info("Bridging /cmd_vel → PX4 /trajectory_setpoint ...")

    def cmd_callback(self, msg: Twist):
        # publish offboard mode (PX4 requires continuous publication)
        mode = OffboardControlMode()
        mode.position = False
        mode.velocity = True
        mode.acceleration = False
        mode.attitude = False
        mode.body_rate = False
        self.mode_pub.publish(mode)

        # convert Twist → TrajectorySetpoint
        ts = TrajectorySetpoint()
        ts.velocity[0] = float(msg.linear.x)
        ts.velocity[1] = float(msg.linear.y)
        ts.velocity[2] = float(msg.linear.z)
        ts.yaw = float(msg.angular.z)
        self.traj_pub.publish(ts)

def main(args=None):
    rclpy.init(args=args)
    node = CmdVelToPX4()
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
