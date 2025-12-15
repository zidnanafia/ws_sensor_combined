#pragma once
#include <behaviortree_cpp_v3/bt_factory.h>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

#include <px4_msgs/msg/offboard_control_mode.hpp>
#include <px4_msgs/msg/trajectory_setpoint.hpp>
#include <px4_msgs/msg/vehicle_local_position.hpp>

#include <chrono>
#include <limits>
#include <mutex>

class WaitForGoalReachedBT : public BT::StatefulActionNode
{
public:
    WaitForGoalReachedBT(const std::string &name, const BT::NodeConfiguration &config)
        : BT::StatefulActionNode(name, config)
    {
        // create node with unique name to avoid duplicate-name warnings
        node_ = rclcpp::Node::make_shared("wait_goal_reached_node_bt_" + name);

        // Subscribe to goal reached signal
        sub_goal_ = node_->create_subscription<std_msgs::msg::Bool>(
            "/goal_reached", 10,
            [this](const std_msgs::msg::Bool::SharedPtr msg) {
                std::lock_guard<std::mutex> lk(mutex_);
                goal_reached_ = msg->data;
            });

        // Subscribe to vehicle_local_position_v1 to get current x,y,z (NED, z down)
        sub_local_pos_ = node_->create_subscription<px4_msgs::msg::VehicleLocalPosition>(
            "/fmu/out/vehicle_local_position_v1", rclcpp::QoS(10).best_effort(),
            [this](const px4_msgs::msg::VehicleLocalPosition::SharedPtr msg) {
                std::lock_guard<std::mutex> lk(mutex_);
                curr_x_ = msg->x;
                curr_y_ = msg->y;
                curr_z_ = msg->z; // z is DOWN (NED)
                have_position_ = true;
            });

        // PX4 OFFBOARD keep-alive publishers
        offboard_pub_ = node_->create_publisher<px4_msgs::msg::OffboardControlMode>(
            "/fmu/in/offboard_control_mode", rclcpp::QoS(10).transient_local());

        traj_pub_ = node_->create_publisher<px4_msgs::msg::TrajectorySetpoint>(
            "/fmu/in/trajectory_setpoint", rclcpp::QoS(10).best_effort());
    }

    static BT::PortsList providedPorts()
    {
        return {};
    }

    BT::NodeStatus onStart() override
    {
        RCLCPP_INFO(node_->get_logger(), "Waiting for goal reached signal... (entering OFFBOARD keepalive)");

        {
            std::lock_guard<std::mutex> lk(mutex_);
            goal_reached_ = false;
        }

        // Wait until we have at least one position sample (short timeout)
        rclcpp::Time t0 = node_->now();
        while (rclcpp::ok() && !have_position_ && (node_->now() - t0).seconds() < 2.0) {
            rclcpp::spin_some(node_);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        // Publish a burst of setpoints BEFORE requesting OFFBOARD (if caller does mode change)
        // This gives PX4 recent setpoints.
        publish_position_burst(40, std::chrono::milliseconds(20));

        return BT::NodeStatus::RUNNING;
    }

    BT::NodeStatus onRunning() override
    {
        // keep processing subscriptions
        rclcpp::spin_some(node_);

        // publish keepalive setpoints at moderate-high rate (burst each tick)
        publish_position_burst(8, std::chrono::milliseconds(20)); // ~8*50Hz ≈ 400ms of setpoints

        {
            std::lock_guard<std::mutex> lk(mutex_);
            if (goal_reached_) {
                RCLCPP_INFO(node_->get_logger(), "Goal reached!");
                return BT::NodeStatus::SUCCESS;
            }
        }

        return BT::NodeStatus::RUNNING;
    }

    void onHalted() override
    {
        RCLCPP_WARN(node_->get_logger(), "WaitForGoalReachedBT halted");
    }

private:
    // publish N messages spaced by period_ms; use position control to hold current pos
    void publish_position_burst(int count, std::chrono::milliseconds period_ms)
    {
        // copy current pos under lock
        double x, y, z;
        bool have = false;
        {
            std::lock_guard<std::mutex> lk(mutex_);
            have = have_position_;
            x = curr_x_;
            y = curr_y_;
            z = curr_z_;
        }

        // If we don't have position, publish a safe zero-velocity setpoint (velocity control),
        // but prefer position control when we have position.
        for (int i = 0; i < count && rclcpp::ok(); ++i) {
            px4_msgs::msg::OffboardControlMode off{};
            off.timestamp = now_us();

            px4_msgs::msg::TrajectorySetpoint sp{};
            sp.timestamp = now_us();

            if (have) {
                // Use position hold in NED frame: sp.position is NED, z is DOWN (so negative altitude)
                off.position = true;
                off.velocity = false;
                off.acceleration = false;

                sp.position[0] = static_cast<float>(x);
                sp.position[1] = static_cast<float>(y);
                sp.position[2] = static_cast<float>(z);
                // no velocity/accel commands (let position controller handle)
                sp.velocity[0] = 0.0f;
                sp.velocity[1] = 0.0f;
                sp.velocity[2] = 0.0f;
                sp.yaw = std::numeric_limits<float>::quiet_NaN();
                sp.yawspeed = 0.0f;
            } else {
                // fallback: velocity mode with zeros
                off.position = false;
                off.velocity = true;
                off.acceleration = false;

                sp.position[0] = std::numeric_limits<float>::quiet_NaN();
                sp.position[1] = std::numeric_limits<float>::quiet_NaN();
                sp.position[2] = std::numeric_limits<float>::quiet_NaN();
                sp.velocity[0] = 0.0f;
                sp.velocity[1] = 0.0f;
                sp.velocity[2] = 0.0f;
                sp.yaw = std::numeric_limits<float>::quiet_NaN();
                sp.yawspeed = 0.0f;
            }

            offboard_pub_->publish(off);
            traj_pub_->publish(sp);

            // small sleep to produce a burst rate (avoid blocking too long)
            std::this_thread::sleep_for(period_ms);
        }
    }

    uint64_t now_us() const
    {
        return node_->get_clock()->now().nanoseconds() / 1000ULL;
    }

    rclcpp::Node::SharedPtr node_;
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr sub_goal_;
    rclcpp::Subscription<px4_msgs::msg::VehicleLocalPosition>::SharedPtr sub_local_pos_;
    rclcpp::Publisher<px4_msgs::msg::OffboardControlMode>::SharedPtr offboard_pub_;
    rclcpp::Publisher<px4_msgs::msg::TrajectorySetpoint>::SharedPtr traj_pub_;

    // protected state
    std::mutex mutex_;
    bool goal_reached_ = false;
    bool have_position_ = false;
    double curr_x_ = 0.0;
    double curr_y_ = 0.0;
    double curr_z_ = 0.0; // NED: down positive
};
