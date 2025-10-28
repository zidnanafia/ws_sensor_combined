// bt_uav_controller.cpp
#include <rclcpp/rclcpp.hpp>
#include <behaviortree_cpp_v3/bt_factory.h>
#include <std_msgs/msg/bool.hpp>
#include <px4_msgs/msg/vehicle_command.hpp>
#include <px4_msgs/msg/vehicle_local_position.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <px4_msgs/msg/vehicle_status.hpp>
#include "bt_uav_controller/px4_state_monitor.hpp"
#include <px4_msgs/msg/offboard_control_mode.hpp>
#include <px4_msgs/msg/trajectory_setpoint.hpp>
#include "rclcpp_action/rclcpp_action.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"
#include "bt_uav_controller/WaitForGoalReachedBT.hpp"

// Nav2 includes
#include <nav2_behavior_tree/plugins/action/navigate_to_pose_action.hpp>

// ===== Custom BT Nodes =====

class NavigateToPoseBT : public BT::StatefulActionNode {
public:
  NavigateToPoseBT(const std::string& name, const BT::NodeConfiguration& config)
      : BT::StatefulActionNode(name, config)
  {
    node_ = rclcpp::Node::make_shared("nav2_client_node");
    action_client_ =
        rclcpp_action::create_client<nav2_msgs::action::NavigateToPose>(node_, "navigate_to_pose");
  }

  static BT::PortsList providedPorts() {
    return {BT::InputPort<geometry_msgs::msg::PoseStamped>("goal_pose")};
  }

  BT::NodeStatus onStart() override {
    geometry_msgs::msg::PoseStamped goal;
    if (!getInput("goal_pose", goal)) {
      RCLCPP_ERROR(node_->get_logger(), "No goal pose provided!");
      return BT::NodeStatus::FAILURE;
    }

    if (!action_client_->wait_for_action_server(std::chrono::seconds(5))) {
      RCLCPP_ERROR(node_->get_logger(), "Nav2 action server not available");
      return BT::NodeStatus::FAILURE;
    }

    nav2_msgs::action::NavigateToPose::Goal goal_msg;
    goal_msg.pose = goal;

    auto send_goal_options =
        rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::SendGoalOptions();
    send_goal_options.goal_response_callback =
        [](auto) { RCLCPP_INFO(rclcpp::get_logger("NavigateToPoseBT"), "Goal accepted!"); };
    send_goal_options.feedback_callback =
        [](auto, auto feedback) { RCLCPP_INFO(rclcpp::get_logger("NavigateToPoseBT"),
                                              "Distance remaining: %.2f",
                                              feedback->distance_remaining); };
    send_goal_options.result_callback =
    [this](auto result) { (void)result; goal_done_ = true; };

    action_client_->async_send_goal(goal_msg, send_goal_options);

    return BT::NodeStatus::RUNNING;
  }

  BT::NodeStatus onRunning() override {
    rclcpp::spin_some(node_);
    if (goal_done_) {
      RCLCPP_INFO(node_->get_logger(), "Goal reached!");
      return BT::NodeStatus::SUCCESS;
    }
    return BT::NodeStatus::RUNNING;
  }

  void onHalted() override {
    RCLCPP_WARN(node_->get_logger(), "NavigateToPoseBT halted");
  }

private:
  rclcpp::Node::SharedPtr node_;
  rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::SharedPtr action_client_;
  bool goal_done_ = false;
};


// = ARM =
class Arm : public BT::StatefulActionNode {
public:
  Arm(const std::string& name, const BT::NodeConfiguration& config)
      : BT::StatefulActionNode(name, config)
  {
    node_ = rclcpp::Node::make_shared("arm_node");
    pub_vehicle_cmd_ = node_->create_publisher<px4_msgs::msg::VehicleCommand>(
        "/fmu/in/vehicle_command", 10);
    state_monitor_ = std::make_shared<PX4StateMonitor>();

    last_command_time_ = node_->now();
  }

  static BT::PortsList providedPorts() {
    return {};  // no input/output ports
  }

  BT::NodeStatus onStart() override {
    RCLCPP_INFO(node_->get_logger(), "Arming drone...");
    sendArmCommand(true);
    last_command_time_ = node_->now();
    return BT::NodeStatus::RUNNING;
  }

  BT::NodeStatus onRunning() override {
    // Process PX4 state updates
    rclcpp::spin_some(state_monitor_);

    // Check armed status
    if (state_monitor_->isArmed()) {
      RCLCPP_INFO(node_->get_logger(), "Drone armed!");
      return BT::NodeStatus::SUCCESS;
    }

    // Re-send arm command every 500 ms until armed
    auto now = node_->now();
    if ((now - last_command_time_).seconds() > 0.5) {
      RCLCPP_INFO(node_->get_logger(), "Re-sending arm command...");
      sendArmCommand(true);
      last_command_time_ = now;
    }

    return BT::NodeStatus::RUNNING;
  }

  void onHalted() override {
    RCLCPP_WARN(node_->get_logger(), "Arming node halted.");
  }

private:
  void sendArmCommand(bool arm) {
    px4_msgs::msg::VehicleCommand msg{};
    msg.command = px4_msgs::msg::VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM;
    msg.param1 = arm ? 1.0f : 0.0f;
    msg.target_system = 1;
    msg.target_component = 1;
    msg.source_system = 1;
    msg.source_component = 1;
    msg.from_external = true;
    msg.timestamp = node_->get_clock()->now().nanoseconds() / 1000;  // microseconds
    pub_vehicle_cmd_->publish(msg);
  }

  rclcpp::Node::SharedPtr node_;
  rclcpp::Publisher<px4_msgs::msg::VehicleCommand>::SharedPtr pub_vehicle_cmd_;
  std::shared_ptr<PX4StateMonitor> state_monitor_;
  rclcpp::Time last_command_time_;
};



// = CheckWaypointPublished =
class CheckWaypointPublished : public BT::StatefulActionNode
{
public:
  CheckWaypointPublished(const std::string& name, const BT::NodeConfiguration& config)
  : BT::StatefulActionNode(name, config)
  {
    node_ = rclcpp::Node::make_shared("check_waypoint_node");

    sub_ = node_->create_subscription<geometry_msgs::msg::PoseStamped>(
      "/goal_pose", 10,
      [this](const geometry_msgs::msg::PoseStamped::SharedPtr msg) {
        latest_waypoint_ = *msg;
        waypoint_received_ = true;
      });
  }

  static BT::PortsList providedPorts() {
    // output port to send waypoint to blackboard
    return { BT::OutputPort<geometry_msgs::msg::PoseStamped>("goal_pose") };
  }

  BT::NodeStatus onStart() override {
    RCLCPP_INFO(node_->get_logger(), "Waiting for waypoint to be published...");
    waypoint_received_ = false;
    return BT::NodeStatus::RUNNING;
  }

  BT::NodeStatus onRunning() override {
    rclcpp::spin_some(node_);

    if (waypoint_received_) {
      setOutput("goal_pose", latest_waypoint_);
      RCLCPP_INFO(node_->get_logger(),
                  "Waypoint received (x=%.2f, y=%.2f, z=%.2f)!",
                  latest_waypoint_.pose.position.x,
                  latest_waypoint_.pose.position.y,
                  latest_waypoint_.pose.position.z);
      return BT::NodeStatus::SUCCESS;
    }

    RCLCPP_INFO_THROTTLE(node_->get_logger(), *node_->get_clock(), 2000,
                         "Still waiting for waypoint...");
    return BT::NodeStatus::RUNNING;
  }

  void onHalted() override {
    RCLCPP_WARN(node_->get_logger(), "Waypoint waiting halted.");
  }

private:
  rclcpp::Node::SharedPtr node_;
  rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr sub_;
  geometry_msgs::msg::PoseStamped latest_waypoint_;
  bool waypoint_received_ = false;
};



// = TakeOff =
class TakeOff : public BT::StatefulActionNode {
public:
  TakeOff(const std::string& name, const BT::NodeConfiguration& config)
  : BT::StatefulActionNode(name, config)
  {
    node_ = rclcpp::Node::make_shared("takeoff_node");
    pub_vehicle_cmd_ = node_->create_publisher<px4_msgs::msg::VehicleCommand>(
      "/fmu/in/vehicle_command", 10);
    state_monitor_ = std::make_shared<PX4StateMonitor>();
  }

  static BT::PortsList providedPorts() {
    return {};
  }

  BT::NodeStatus onStart() override {
    RCLCPP_INFO(node_->get_logger(), "Initiating Takeoff...");
    sendVehicleCommand(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_NAV_TAKEOFF,
                       0.0, 0.0, 0.0, NAN, NAN, NAN, 2.0);
    return BT::NodeStatus::RUNNING;
  }

  BT::NodeStatus onRunning() override {
    rclcpp::spin_some(state_monitor_);
    
    float altitude = state_monitor_->getAltitude();

    if (state_monitor_->isArmed() && altitude > 1.5f) {
      RCLCPP_INFO(node_->get_logger(), "Takeoff complete! Altitude: %.2f m", altitude);
      return BT::NodeStatus::SUCCESS;
    }

    RCLCPP_INFO_THROTTLE(node_->get_logger(), *node_->get_clock(), 2000,
                        "Ascending... current altitude: %.2f m", altitude);
    return BT::NodeStatus::RUNNING;
  }

  void onHalted() override {
    RCLCPP_WARN(node_->get_logger(), "Takeoff halted!");
  }

private:
  void sendVehicleCommand(uint16_t command, float param1 = 0.0, float param2 = 0.0,
                          float param3 = 0.0, float param4 = NAN, float param5 = NAN,
                          float param6 = NAN, float param7 = NAN) {
    px4_msgs::msg::VehicleCommand msg{};
    msg.command = command;
    msg.param1 = param1;
    msg.param2 = param2;
    msg.param3 = param3;
    msg.param4 = param4;
    msg.param5 = param5;
    msg.param6 = param6;
    msg.param7 = param7;
    msg.target_system = 1;
    msg.target_component = 1;
    msg.source_system = 1;
    msg.source_component = 1;
    msg.from_external = true;
    msg.timestamp = node_->get_clock()->now().nanoseconds() / 1000; 

    pub_vehicle_cmd_->publish(msg);
  }

  rclcpp::Node::SharedPtr node_;
  rclcpp::Publisher<px4_msgs::msg::VehicleCommand>::SharedPtr pub_vehicle_cmd_;
  std::shared_ptr<PX4StateMonitor> state_monitor_;
};

// = IsOffboardMode =
class IsOffboardMode : public BT::StatefulActionNode
{
public:
  IsOffboardMode(const std::string& name, const BT::NodeConfiguration& config)
  : BT::StatefulActionNode(name, config)
  {
    auto best_effort_qos = rclcpp::QoS(rclcpp::KeepLast(10)).best_effort();
    auto transient_local_qos = rclcpp::QoS(rclcpp::KeepLast(1)).transient_local();

    // Node for this BT node (separate logger/clock)
    node_ = rclcpp::Node::make_shared("is_offboard_node");

    // publishers: trajectory_setpoint and offboard_control_mode are required for OFFBOARD
    pub_offboard_mode_ = node_->create_publisher<px4_msgs::msg::OffboardControlMode>(
      "/fmu/in/offboard_control_mode", transient_local_qos);
    pub_trajectory_ = node_->create_publisher<px4_msgs::msg::TrajectorySetpoint>(
      "/fmu/in/trajectory_setpoint", best_effort_qos);

    // command publisher (still useful)
    pub_vehicle_cmd_ = node_->create_publisher<px4_msgs::msg::VehicleCommand>(
      "/fmu/in/vehicle_command", 10);

    // state monitor (assumes PX4StateMonitor is a Node-derived class and implemented)
    state_monitor_ = std::make_shared<PX4StateMonitor>();
  }

  static BT::PortsList providedPorts() { return {}; }

  BT::NodeStatus onStart() override
  {
    RCLCPP_INFO(node_->get_logger(), "Preparing OFFBOARD: publishing setpoints first...");

    // publish setpoints for 2 seconds at 10Hz (20 messages)
    publish_pre_offboard_setpoints(20, std::chrono::milliseconds(100));

    // then send the set-mode command
    sendOffboardCommand();

    return BT::NodeStatus::RUNNING;
  }

  BT::NodeStatus onRunning() override
  {
    // spin the monitor node base interface so subscriptions run
    rclcpp::spin_some(state_monitor_->get_node_base_interface());

    if (state_monitor_->isOffboard()) {
      RCLCPP_INFO(node_->get_logger(), "PX4 is now in OFFBOARD mode!");
      return BT::NodeStatus::SUCCESS;
    }

    // not yet offboard: retry every ~1 second
    static int retry = 0;
    ++retry;
    if (retry % 10 == 0) {
      sendOffboardCommand();
      RCLCPP_INFO(node_->get_logger(), "Retrying OFFBOARD set-mode command...");
    }

    rclcpp::sleep_for(std::chrono::milliseconds(100));
    return BT::NodeStatus::RUNNING;
  }

  void onHalted() override {}

private:
  void publish_pre_offboard_setpoints(int count, std::chrono::milliseconds period)
  {
    // publish OffboardControlMode + a simple TrajectorySetpoint repeatedly
    for (int i = 0; i < count && rclcpp::ok(); ++i) {
      // OffboardControlMode: request velocity control (no position)
      px4_msgs::msg::OffboardControlMode off{};
      off.timestamp = static_cast<uint64_t>(node_->now().nanoseconds() / 1000);
      off.position = false;
      off.velocity = true;
      off.acceleration = false;
      pub_offboard_mode_->publish(off);

      // TrajectorySetpoint: send NaN position and zero velocity (or small)
      px4_msgs::msg::TrajectorySetpoint sp{};
      sp.timestamp = static_cast<uint64_t>(node_->now().nanoseconds() / 1000);
      sp.position[0] = std::numeric_limits<float>::quiet_NaN();
      sp.position[1] = std::numeric_limits<float>::quiet_NaN();
      sp.position[2] = std::numeric_limits<float>::quiet_NaN();
      sp.velocity[0] = 0.0f;
      sp.velocity[1] = 0.0f;
      sp.velocity[2] = 0.0f;
      sp.yaw = std::numeric_limits<float>::quiet_NaN();
      sp.yawspeed = 0.0f;
      pub_trajectory_->publish(sp);

      rclcpp::spin_some(node_);
      std::this_thread::sleep_for(period);
    }
  }

  void sendOffboardCommand()
  {
    px4_msgs::msg::VehicleCommand msg{};
    msg.command = px4_msgs::msg::VehicleCommand::VEHICLE_CMD_DO_SET_MODE;
    msg.param1 = 1.0f;  // custom mode enabled
    msg.param2 = 6.0f;  // main_mode = 6 (OFFBOARD)
    msg.target_system = 1;
    msg.target_component = 1;
    msg.source_system = 1;
    msg.source_component = 1;
    msg.from_external = true;
    msg.timestamp = static_cast<uint64_t>(node_->now().nanoseconds() / 1000);
    pub_vehicle_cmd_->publish(msg);
    RCLCPP_INFO(node_->get_logger(), "Sent VEHICLE_CMD_DO_SET_MODE (OFFBOARD)");
  }

  rclcpp::Node::SharedPtr node_;
  rclcpp::Publisher<px4_msgs::msg::OffboardControlMode>::SharedPtr pub_offboard_mode_;
  rclcpp::Publisher<px4_msgs::msg::TrajectorySetpoint>::SharedPtr pub_trajectory_;
  rclcpp::Publisher<px4_msgs::msg::VehicleCommand>::SharedPtr pub_vehicle_cmd_;
  std::shared_ptr<PX4StateMonitor> state_monitor_;
};

//== LAND ==
class Land : public BT::StatefulActionNode {
public:
  Land(const std::string& name, const BT::NodeConfiguration& config)
  : BT::StatefulActionNode(name, config)
  {
    node_ = rclcpp::Node::make_shared("land_node");
    pub_vehicle_cmd_ = node_->create_publisher<px4_msgs::msg::VehicleCommand>(
      "/fmu/in/vehicle_command", 10);
    state_monitor_ = std::make_shared<PX4StateMonitor>();
  }

  static BT::PortsList providedPorts() {
    return {};
  }

  BT::NodeStatus onStart() override {
    RCLCPP_INFO(node_->get_logger(), "Initiating Landing...");
    sendVehicleCommand(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_NAV_LAND);
    return BT::NodeStatus::RUNNING;
  }

  BT::NodeStatus onRunning() override {
    rclcpp::spin_some(state_monitor_);
    if (fabs(state_monitor_->getAltitude()) < 0.2) {
      RCLCPP_INFO(node_->get_logger(), "Landing complete!");
      return BT::NodeStatus::SUCCESS;
    }
    RCLCPP_INFO_THROTTLE(node_->get_logger(), *node_->get_clock(), 2000, "Descending...");
    return BT::NodeStatus::RUNNING;
  }

  void onHalted() override {
    RCLCPP_WARN(node_->get_logger(), "Landing halted!");
  }

private:
  void sendVehicleCommand(uint16_t command, float param1 = 0.0, float param2 = 0.0,
                          float param3 = 0.0, float param4 = NAN, float param5 = NAN,
                          float param6 = NAN, float param7 = NAN) {
    px4_msgs::msg::VehicleCommand msg{};
    msg.command = command;
    msg.param1 = param1;
    msg.param2 = param2;
    msg.param3 = param3;
    msg.param4 = param4;
    msg.param5 = param5;
    msg.param6 = param6;
    msg.param7 = param7;
    msg.target_system = 1;
    msg.target_component = 1;
    msg.source_system = 1;
    msg.source_component = 1;
    msg.from_external = true;
    msg.timestamp = node_->get_clock()->now().nanoseconds() / 1000; 

    pub_vehicle_cmd_->publish(msg);
  }

  rclcpp::Node::SharedPtr node_;
  rclcpp::Publisher<px4_msgs::msg::VehicleCommand>::SharedPtr pub_vehicle_cmd_;
  std::shared_ptr<PX4StateMonitor> state_monitor_;
};



// ===== XML Tree Definition =====
static const char* xml_tree = R"(
<root main_tree_to_execute="MainTree">
  <BehaviorTree ID="MainTree">
    <Sequence name="main_sequence">
      <CheckWaypointPublished/>
      <Arm/>
      <TakeOff/>
      <IsOffboardMode/>
      <WaitForGoalReached/>
      <Land/>
    </Sequence>
  </BehaviorTree>
</root>
)";

// ===== Main Function =====
int main(int argc, char **argv)
{
  rclcpp::init(argc, argv);
  rclcpp::Node::SharedPtr node = rclcpp::Node::make_shared("bt_uav_controller");

  BT::BehaviorTreeFactory factory;
  factory.registerNodeType<CheckWaypointPublished>("CheckWaypointPublished");
  factory.registerNodeType<TakeOff>("TakeOff");
  factory.registerNodeType<Arm>("Arm");
  factory.registerNodeType<IsOffboardMode>("IsOffboardMode");
  factory.registerNodeType<NavigateToPoseBT>("NavigateToPose");
  factory.registerNodeType<Land>("Land");
  factory.registerNodeType<WaitForGoalReachedBT>("WaitForGoalReached");

  // Register Nav2 action nodes
  // factory.registerNodeType<nav2_behavior_tree::NavigateToPoseAction>("NavigateToPose");


  auto blackboard = BT::Blackboard::create();
  blackboard->set("node", node); 

  auto tree = factory.createTreeFromText(xml_tree, blackboard);
  BT::NodeStatus status = BT::NodeStatus::RUNNING;

  rclcpp::Rate loop_rate(10);
  while (rclcpp::ok() && status == BT::NodeStatus::RUNNING) {
    status = tree.tickRoot();
    rclcpp::spin_some(node);
    loop_rate.sleep();
  }

  rclcpp::shutdown();
  return 0;
}
