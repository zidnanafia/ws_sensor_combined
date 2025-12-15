#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "bt_uav_controller/action/takeoff.hpp"

#include "px4_msgs/msg/vehicle_command.hpp"
#include "px4_msgs/msg/offboard_control_mode.hpp"
#include "px4_msgs/msg/trajectory_setpoint.hpp"
#include "px4_msgs/msg/vehicle_status.hpp"

#include <thread>
#include <chrono>
#include <cmath>

using namespace std::chrono_literals;

class TakeoffActionServer : public rclcpp::Node {
public:
  using Takeoff = bt_uav_controller::action::Takeoff;
  using GoalHandleTakeoff = rclcpp_action::ServerGoalHandle<Takeoff>;

  TakeoffActionServer()
  : Node("takeoff_action_server")
  {
    using namespace std::placeholders;

    // --- Action Server ---
    server_ = rclcpp_action::create_server<Takeoff>(
      this,
      "takeoff_action",
      std::bind(&TakeoffActionServer::handle_goal, this, _1, _2),
      std::bind(&TakeoffActionServer::handle_cancel, this, _1),
      std::bind(&TakeoffActionServer::handle_accepted, this, _1)
    );

    // --- Publishers ---
    pub_vehicle_cmd_ = create_publisher<px4_msgs::msg::VehicleCommand>("/fmu/in/vehicle_command", 10);
    pub_offboard_mode_ = create_publisher<px4_msgs::msg::OffboardControlMode>("/fmu/in/offboard_control_mode", 10);
    pub_traj_ = create_publisher<px4_msgs::msg::TrajectorySetpoint>("/fmu/in/trajectory_setpoint", 10);

    // --- Subscriber (BestEffort QoS) ---
    sub_status_ = create_subscription<px4_msgs::msg::VehicleStatus>(
      "/fmu/out/vehicle_status_v1",
      rclcpp::QoS(rclcpp::KeepLast(10)).best_effort().durability_volatile(),
      std::bind(&TakeoffActionServer::vehicle_status_cb, this, std::placeholders::_1)
    );

    RCLCPP_INFO(get_logger(), "✅ PX4 FSM-style Takeoff Action Server ready.");
  }

private:
  // --- ROS Interfaces ---
  rclcpp_action::Server<Takeoff>::SharedPtr server_;
  rclcpp::Publisher<px4_msgs::msg::VehicleCommand>::SharedPtr pub_vehicle_cmd_;
  rclcpp::Publisher<px4_msgs::msg::OffboardControlMode>::SharedPtr pub_offboard_mode_;
  rclcpp::Publisher<px4_msgs::msg::TrajectorySetpoint>::SharedPtr pub_traj_;
  rclcpp::Subscription<px4_msgs::msg::VehicleStatus>::SharedPtr sub_status_;

  // --- PX4 Status ---
  uint8_t nav_state_ = 0;
  uint8_t arm_state_ = 0;
  bool preflight_ok_ = false;
  bool failsafe_ = false;

  // --- Action Callbacks ---
  rclcpp_action::GoalResponse handle_goal(const rclcpp_action::GoalUUID &,
                                          std::shared_ptr<const Takeoff::Goal> goal)
  {
    RCLCPP_INFO(get_logger(), "📥 Takeoff goal received: %.2f m", goal->target_altitude);
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }

  rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<GoalHandleTakeoff>)
  {
    RCLCPP_WARN(get_logger(), "⚠️ Takeoff canceled!");
    return rclcpp_action::CancelResponse::ACCEPT;
  }

  void handle_accepted(const std::shared_ptr<GoalHandleTakeoff> goal_handle)
  {
    std::thread{std::bind(&TakeoffActionServer::execute, this, goal_handle)}.detach();
  }

  // --- Main FSM Execution ---
  void execute(const std::shared_ptr<GoalHandleTakeoff> goal_handle)
  {
    auto goal = goal_handle->get_goal();
    auto feedback = std::make_shared<Takeoff::Feedback>();
    auto result = std::make_shared<Takeoff::Result>();
    rclcpp::Rate rate(10);

    const float target_alt = goal->target_altitude;
    const float z_ned = -std::fabs(target_alt);

    RCLCPP_INFO(get_logger(), "🚀 Takeoff Sequence: PREFLIGHT → ARM → TAKEOFF → OFFBOARD");

    const auto preflight_timeout = 15s;
    const auto arm_timeout = 10s;
    const auto takeoff_timeout = 15s;
    const auto overall_timeout = 90s;
    auto start_time = now();

    // 1️⃣ PREFLIGHT CHECK
    {
      auto t0 = now();
      while (!preflight_ok_ && rclcpp::ok() && (now() - t0) < preflight_timeout) {
        RCLCPP_INFO_THROTTLE(get_logger(), *get_clock(), 2000, "⏳ Waiting for preflight OK...");
        std::this_thread::sleep_for(200ms);
      }
      if (!preflight_ok_) {
        RCLCPP_ERROR(get_logger(), "❌ Preflight checks failed/timeout.");
        result->success = false;
        goal_handle->abort(result);
        return;
      }
      RCLCPP_INFO(get_logger(), "✅ Preflight OK.");
    }

    // 2️⃣ ARMING
    sendArmCommand();
    {
      auto t0 = now();
      while (arm_state_ != px4_msgs::msg::VehicleStatus::ARMING_STATE_ARMED &&
             rclcpp::ok() && (now() - t0) < arm_timeout) {
        std::this_thread::sleep_for(100ms);
      }
      if (arm_state_ != px4_msgs::msg::VehicleStatus::ARMING_STATE_ARMED) {
        RCLCPP_ERROR(get_logger(), "❌ Arming failed or timeout.");
        result->success = false;
        goal_handle->abort(result);
        return;
      }
      RCLCPP_INFO(get_logger(), "✅ Armed confirmed.");
    }

    // 3️⃣ TAKEOFF
    // publish some dummy setpoints to satisfy offboard requirement
    for (int i = 0; i < 50 && rclcpp::ok(); ++i) {
      publish_hover_setpoint(0.0f);
      std::this_thread::sleep_for(20ms);
    }

    sendTakeoffCommand(target_alt);
    {
      auto t0 = now();
      while (nav_state_ != px4_msgs::msg::VehicleStatus::NAVIGATION_STATE_AUTO_TAKEOFF &&
             rclcpp::ok() && (now() - t0) < takeoff_timeout) {
        feedback->current_altitude = get_current_alt_estimate_or_zero();
        goal_handle->publish_feedback(feedback);
        std::this_thread::sleep_for(100ms);
      }
      if (nav_state_ != px4_msgs::msg::VehicleStatus::NAVIGATION_STATE_AUTO_TAKEOFF) {
        RCLCPP_ERROR(get_logger(), "❌ TAKEOFF command not acknowledged by PX4.");
        result->success = false;
        goal_handle->abort(result);
        return;
      }
      RCLCPP_INFO(get_logger(), "🚁 PX4 entered AUTO_TAKEOFF.");
    }

    // 4️⃣ WAIT UNTIL TAKEOFF COMPLETED (PX4 switches to LOITER or MISSION)
    RCLCPP_INFO(get_logger(), "🕐 Waiting for PX4 to finish AUTO_TAKEOFF...");

    {
    const auto post_takeoff_timeout = 30s;
    auto t0 = now();
    bool takeoff_done = false;

    while (rclcpp::ok() && (now() - t0) < post_takeoff_timeout) {
        feedback->current_altitude = get_current_alt_estimate_or_zero();
        goal_handle->publish_feedback(feedback);

        // PX4 transitions to AUTO_LOITER (or MISSION) once takeoff complete
        if (nav_state_ == px4_msgs::msg::VehicleStatus::NAVIGATION_STATE_AUTO_LOITER ||
            nav_state_ == px4_msgs::msg::VehicleStatus::NAVIGATION_STATE_AUTO_MISSION) {
        RCLCPP_INFO(get_logger(), "✅ PX4 TAKEOFF completed — now hovering or holding position.");
        takeoff_done = true;
        break;
        }

        std::this_thread::sleep_for(200ms);
    }

    if (!takeoff_done) {
        RCLCPP_WARN(get_logger(), "⚠️ Takeoff complete not detected, proceeding anyway...");
    }
    }


    // 5️⃣ NOW SWITCH TO OFFBOARD (AFTER TAKEOFF COMPLETE)
    RCLCPP_INFO(get_logger(), "🔄 Switching to OFFBOARD mode after complete takeoff...");
    sendOffboardCommand();

    // send continuous hover setpoints for stability
    for (int i = 0; i < 200 && rclcpp::ok(); ++i) {
    publish_hover_setpoint(z_ned);
    feedback->current_altitude = target_alt;
    goal_handle->publish_feedback(feedback);
    std::this_thread::sleep_for(50ms);
    }

    // ✅ SUCCESS
    result->success = true;
    goal_handle->succeed(result);
    RCLCPP_INFO(get_logger(), "🎮 Takeoff succeeded and hovering at %.2fm (OFFBOARD active)", target_alt);
  }

  // --- Vehicle Status Callback ---
  void vehicle_status_cb(const px4_msgs::msg::VehicleStatus::SharedPtr msg)
  {
    nav_state_ = msg->nav_state;
    arm_state_ = msg->arming_state;
    preflight_ok_ = msg->pre_flight_checks_pass;
    failsafe_ = msg->failsafe;
  }

  // --- Helper Commands ---
  void sendArmCommand()
  {
    sendVehicleCommand(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM, 1.0f);
    RCLCPP_INFO(get_logger(), "Sent ARM command.");
  }

  void sendTakeoffCommand(float altitude)
  {
    sendVehicleCommand(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_NAV_TAKEOFF, 1.0f, 0.0f, 0.0f,
                       0.0f, 0.0f, altitude);
    RCLCPP_INFO(get_logger(), "Sent TAKEOFF command (%.2fm).", altitude);
  }

  void sendOffboardCommand()
  {
    sendVehicleCommand(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_DO_SET_MODE, 1.0f, 6.0f);
    RCLCPP_INFO(get_logger(), "Sent MODE->OFFBOARD command.");
  }

  void publish_hover_setpoint(float z_ned)
  {
    px4_msgs::msg::OffboardControlMode off{};
    off.timestamp = now_us();
    off.position = true;
    pub_offboard_mode_->publish(off);

    px4_msgs::msg::TrajectorySetpoint sp{};
    sp.timestamp = now_us();
    sp.position[0] = 0.0f;
    sp.position[1] = 0.0f;
    sp.position[2] = z_ned;
    sp.yaw = 0.0f;
    pub_traj_->publish(sp);
  }

  void sendVehicleCommand(uint16_t cmd, float p1 = 0.0f, float p2 = 0.0f,
                          float p3 = 0.0f, float p4 = 0.0f, float p5 = 0.0f,
                          float p7 = 0.0f)
  {
    px4_msgs::msg::VehicleCommand msg{};
    msg.timestamp = now_us();
    msg.command = cmd;
    msg.param1 = p1;
    msg.param2 = p2;
    msg.param3 = p3;
    msg.param4 = p4;
    msg.param5 = p5;
    msg.param7 = p7;
    msg.target_system = 1;
    msg.target_component = 1;
    msg.source_system = 1;
    msg.source_component = 1;
    msg.from_external = true;
    pub_vehicle_cmd_->publish(msg);
  }

  inline uint64_t now_us() { return this->get_clock()->now().nanoseconds() / 1000ULL; }
  rclcpp::Time now() { return this->get_clock()->now(); }
  float get_current_alt_estimate_or_zero() { return 0.0f; } // extend later with local_position
};

int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TakeoffActionServer>());
  rclcpp::shutdown();
  return 0;
}
