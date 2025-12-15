// bt_uav_controller.cpp
// Behavior Tree for UAV mission, fully offloaded PX4 control to velocity_control.py
// Trigger arm via /arm_message, handle navigation and landing via BT

#include <rclcpp/rclcpp.hpp>
#include <behaviortree_cpp_v3/bt_factory.h>
#include <std_msgs/msg/bool.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <px4_msgs/msg/vehicle_status.hpp>
#include <px4_msgs/msg/vehicle_local_position.hpp>
#include <px4_msgs/msg/vehicle_command.hpp>
#include <px4_msgs/msg/trajectory_setpoint.hpp>
#include <memory>
#include <chrono>
#include <cmath>

using namespace std::chrono_literals;

/* --------------------
   PX4 State Monitor
   -------------------- */
class PX4StateMonitor : public rclcpp::Node {
public:
    PX4StateMonitor() : Node("px4_state_monitor") {
        sub_pos_ = this->create_subscription<px4_msgs::msg::VehicleLocalPosition>(
            "/fmu/out/vehicle_local_position", 10,
            [this](const px4_msgs::msg::VehicleLocalPosition::SharedPtr msg) {
                altitude_ = msg->z;
            });
    }

    float getAltitude() const { return altitude_; }

private:
    rclcpp::Subscription<px4_msgs::msg::VehicleLocalPosition>::SharedPtr sub_pos_;
    std::atomic<float> altitude_{0.0f};
};

/* --------------------
   Behavior Tree Nodes
   -------------------- */

// TriggerArmSequence
class TriggerArmSequence : public BT::StatefulActionNode {
public:
    TriggerArmSequence(const std::string &name, const BT::NodeConfiguration &config)
        : BT::StatefulActionNode(name, config)
    {
        node_ = rclcpp::Node::make_shared("trigger_arm_node");

        rclcpp::QoS arm_qos(10);
        arm_qos.transient_local();
        pub_arm_ = node_->create_publisher<std_msgs::msg::Bool>("/arm_message", arm_qos);

        auto status_qos = rclcpp::QoS(rclcpp::KeepLast(10)).best_effort();
        sub_status_ = node_->create_subscription<px4_msgs::msg::VehicleStatus>(
            "/fmu/out/vehicle_status_v1", status_qos,
            [this](const px4_msgs::msg::VehicleStatus::SharedPtr msg) {
                nav_state_ = msg->nav_state;
                armed_ = (msg->arming_state == px4_msgs::msg::VehicleStatus::ARMING_STATE_ARMED);
            });

        triggered_ = false;
        nav_state_ = 0;
        armed_ = false;
    }

    static BT::PortsList providedPorts() { return {}; }

    BT::NodeStatus onStart() override {
        triggered_ = false;
        RCLCPP_INFO(node_->get_logger(), "TriggerArmSequence started...");
        return BT::NodeStatus::RUNNING;
    }

    BT::NodeStatus onRunning() override {
        rclcpp::spin_some(node_);

        if (!triggered_) {
            std_msgs::msg::Bool msg;
            msg.data = true;
            pub_arm_->publish(msg);
            RCLCPP_INFO(node_->get_logger(), "Published /arm_message = true");
            triggered_ = true;
        }

        if (nav_state_ == px4_msgs::msg::VehicleStatus::NAVIGATION_STATE_OFFBOARD && armed_) {
            RCLCPP_INFO(node_->get_logger(), "PX4 is in OFFBOARD mode and armed!");
            return BT::NodeStatus::SUCCESS;
        }

        return BT::NodeStatus::RUNNING;
    }

    void onHalted() override {}

private:
    rclcpp::Node::SharedPtr node_;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr pub_arm_;
    rclcpp::Subscription<px4_msgs::msg::VehicleStatus>::SharedPtr sub_status_;

    bool triggered_;
    uint8_t nav_state_;
    bool armed_;
};


// CheckWaypointPublished
class CheckWaypointPublished : public BT::StatefulActionNode {
public:
    CheckWaypointPublished(const std::string &name, const BT::NodeConfiguration &config)
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
        return { BT::OutputPort<geometry_msgs::msg::PoseStamped>("goal_pose") };
    }

    BT::NodeStatus onStart() override {
        waypoint_received_ = false;
        RCLCPP_INFO(node_->get_logger(), "Waiting for waypoint...");
        return BT::NodeStatus::RUNNING;
    }

    BT::NodeStatus onRunning() override {
        rclcpp::spin_some(node_);
        if (waypoint_received_) {
            setOutput("goal_pose", latest_waypoint_);
            RCLCPP_INFO(node_->get_logger(), "Waypoint received!");
            return BT::NodeStatus::SUCCESS;
        }
        return BT::NodeStatus::RUNNING;
    }

    void onHalted() override {}

private:
    rclcpp::Node::SharedPtr node_;
    rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr sub_;
    geometry_msgs::msg::PoseStamped latest_waypoint_;
    bool waypoint_received_{false};
};

// WaitForGoalReachedBT
class WaitForGoalReachedBT : public BT::StatefulActionNode {
public:
    WaitForGoalReachedBT(const std::string &name, const BT::NodeConfiguration &config)
        : BT::StatefulActionNode(name, config)
    {
        node_ = rclcpp::Node::make_shared("wait_goal_node");
        sub_ = node_->create_subscription<std_msgs::msg::Bool>(
            "/goal_reached", 10,
            [this](const std_msgs::msg::Bool::SharedPtr msg) {
                goal_reached_ = msg->data;
            });
    }

    static BT::PortsList providedPorts() { return {}; }

    BT::NodeStatus onStart() override {
        goal_reached_ = false;
        RCLCPP_INFO(node_->get_logger(), "Waiting for goal reached...");
        return BT::NodeStatus::RUNNING;
    }

    BT::NodeStatus onRunning() override {
        rclcpp::spin_some(node_);
        if (goal_reached_) {
            RCLCPP_INFO(node_->get_logger(), "Goal reached!");
            return BT::NodeStatus::SUCCESS;
        }
        return BT::NodeStatus::RUNNING;
    }

    void onHalted() override {}

private:
    rclcpp::Node::SharedPtr node_;
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr sub_;
    bool goal_reached_{false};
};

// Land node
// Land node - publishes disable_offboard signal
class Land : public BT::StatefulActionNode {
public:
    Land(const std::string &name, const BT::NodeConfiguration &config)
        : BT::StatefulActionNode(name, config)
    {
        node_ = rclcpp::Node::make_shared("land_node");

        // Publisher for VEHICLE_CMD
        pub_vehicle_cmd_ = node_->create_publisher<px4_msgs::msg::VehicleCommand>(
            "/fmu/in/vehicle_command", 10);

        // Publisher to disable offboard control during landing
        rclcpp::QoS disable_qos(10);
        disable_qos.transient_local();
        pub_disable_offboard_ = node_->create_publisher<std_msgs::msg::Bool>(
            "/disable_offboard", disable_qos);

        // PX4StateMonitor subscription
        sub_pos_ = node_->create_subscription<px4_msgs::msg::VehicleLocalPosition>(
          "/fmu/out/vehicle_local_position_v1",
          rclcpp::QoS(rclcpp::KeepLast(10)).best_effort(),
          [this](const px4_msgs::msg::VehicleLocalPosition::SharedPtr msg) {
              altitude_ = -msg->z;  // Convert NED to altitude (negate Z)
          });

        sent_ = false;
        disable_sent_ = false;
    }

    static BT::PortsList providedPorts() { return {}; }

    BT::NodeStatus onStart() override {
        RCLCPP_INFO(node_->get_logger(), "Initiating Landing...");
        sent_ = false;
        disable_sent_ = false;
        return BT::NodeStatus::RUNNING;
    }

    BT::NodeStatus onRunning() override {
        rclcpp::spin_some(node_); // update subscriptions

        // Send disable_offboard signal once at the beginning
        if (!disable_sent_) {
            std_msgs::msg::Bool disable_msg;
            disable_msg.data = true;
            pub_disable_offboard_->publish(disable_msg);
            RCLCPP_INFO(node_->get_logger(), "Offboard control disabled for landing");
            disable_sent_ = true;
        }

        // Log altitude every 1 sec
        RCLCPP_INFO_THROTTLE(node_->get_logger(), *node_->get_clock(), 1000,
                             "Current Altitude: %.2f m", altitude_);

        // Send LAND command repeatedly until altitude < 0.2
        sendLandCommand();

        if (altitude_ < 0.2f) {
            RCLCPP_INFO(node_->get_logger(), "Landing complete!");
            return BT::NodeStatus::SUCCESS;
        }

        return BT::NodeStatus::RUNNING;
    }

    void onHalted() override {
        RCLCPP_WARN(node_->get_logger(), "Landing halted!");
    }

private:
    void sendLandCommand() {
        px4_msgs::msg::VehicleCommand land_cmd{};
        land_cmd.command = px4_msgs::msg::VehicleCommand::VEHICLE_CMD_NAV_LAND;

        land_cmd.param1 = 0.0f;
        land_cmd.param2 = 0.0f;
        land_cmd.param3 = 0.0f;
        land_cmd.param4 = NAN;
        land_cmd.param5 = NAN;
        land_cmd.param6 = NAN;
        land_cmd.param7 = NAN;

        land_cmd.target_system = 1;
        land_cmd.target_component = 1;
        land_cmd.source_system = 1;
        land_cmd.source_component = 1;
        land_cmd.from_external = true;
        land_cmd.timestamp = node_->get_clock()->now().nanoseconds() / 1000ULL;

        pub_vehicle_cmd_->publish(land_cmd);

        if (!sent_) {
            RCLCPP_INFO(node_->get_logger(), "Landing command sent!");
            sent_ = true;
        }
    }

    rclcpp::Node::SharedPtr node_;
    rclcpp::Publisher<px4_msgs::msg::VehicleCommand>::SharedPtr pub_vehicle_cmd_;
    rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr pub_disable_offboard_;  // NEW
    rclcpp::Subscription<px4_msgs::msg::VehicleLocalPosition>::SharedPtr sub_pos_;
    float altitude_{0.0f};
    bool sent_;
    bool disable_sent_;  // NEW
};


/* ======== Main & BT registration ======== */
int main(int argc, char **argv) {
    rclcpp::init(argc, argv);
    auto top_node = rclcpp::Node::make_shared("bt_uav_controller");

    BT::BehaviorTreeFactory factory;
    factory.registerNodeType<TriggerArmSequence>("TriggerArmSequence");
    factory.registerNodeType<CheckWaypointPublished>("CheckWaypointPublished");
    factory.registerNodeType<WaitForGoalReachedBT>("WaitForGoalReached");
    factory.registerNodeType<Land>("Land");

    // XML tree
    static const char* xml_tree = R"(
    <root main_tree_to_execute="MainTree">
        <BehaviorTree ID="MainTree">
            <Sequence name="main_sequence">
                <CheckWaypointPublished/>
                <TriggerArmSequence/>
                <WaitForGoalReached/>
                <Land/> 
            </Sequence>
        </BehaviorTree>
    </root>
    )";

    auto blackboard = BT::Blackboard::create();
    blackboard->set("node", top_node);

    auto tree = factory.createTreeFromText(xml_tree, blackboard);

    rclcpp::Rate loop_rate(10);
    BT::NodeStatus status = BT::NodeStatus::RUNNING;
    while (rclcpp::ok() && status == BT::NodeStatus::RUNNING) {
        status = tree.tickRoot();
        rclcpp::spin_some(top_node);
        loop_rate.sleep();
    }

    rclcpp::shutdown();
    return 0;
}