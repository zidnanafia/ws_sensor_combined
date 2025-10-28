#pragma once

#include "rclcpp/rclcpp.hpp"
#include "px4_msgs/msg/vehicle_status.hpp"
#include "px4_msgs/msg/vehicle_local_position.hpp"

class PX4StateMonitor : public rclcpp::Node {
public:
  PX4StateMonitor();

  bool isArmed() const { return arming_state_ == 2; }     // ARMING_STATE_ARMED
  bool isOffboard() const { return nav_state_ == 14; }    // NAVIGATION_STATE_OFFBOARD

  // Return altitude above takeoff (positive up)
  float getAltitude() const { return -z_pos_; }

private:
  void statusCallback(const px4_msgs::msg::VehicleStatus::SharedPtr msg);
  void posCallback(const px4_msgs::msg::VehicleLocalPosition::SharedPtr msg);

  rclcpp::Subscription<px4_msgs::msg::VehicleStatus>::SharedPtr sub_status_;
  rclcpp::Subscription<px4_msgs::msg::VehicleLocalPosition>::SharedPtr sub_local_pos_;

  uint8_t arming_state_{0};
  uint8_t nav_state_{0};
  float z_pos_{0.0f};
};
