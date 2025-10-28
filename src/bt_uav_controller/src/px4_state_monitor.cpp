#include "bt_uav_controller/px4_state_monitor.hpp"

PX4StateMonitor::PX4StateMonitor() : Node("px4_state_monitor") {
  using std::placeholders::_1;
  auto qos = rclcpp::QoS(10).best_effort();

  sub_status_ = create_subscription<px4_msgs::msg::VehicleStatus>(
    "/fmu/out/vehicle_status_v1", qos,
    std::bind(&PX4StateMonitor::statusCallback, this, _1));

  // Use v1 topic explicitly to match PX4 naming
  sub_local_pos_ = create_subscription<px4_msgs::msg::VehicleLocalPosition>(
    "/fmu/out/vehicle_local_position_v1", qos,
    std::bind(&PX4StateMonitor::posCallback, this, _1));
}

void PX4StateMonitor::statusCallback(const px4_msgs::msg::VehicleStatus::SharedPtr msg) {
  arming_state_ = msg->arming_state;
  nav_state_ = msg->nav_state;
}

void PX4StateMonitor::posCallback(const px4_msgs::msg::VehicleLocalPosition::SharedPtr msg) {
  z_pos_ = msg->z; // z is DOWN, so altitude = -z
}
