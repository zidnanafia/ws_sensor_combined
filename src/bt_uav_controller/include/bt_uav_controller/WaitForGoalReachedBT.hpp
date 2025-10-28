// WaitForGoalReachedBT.hpp
#pragma once
#include <behaviortree_cpp_v3/bt_factory.h>
#include <rclcpp/rclcpp.hpp>
#include <std_msgs/msg/bool.hpp>

class WaitForGoalReachedBT : public BT::StatefulActionNode
{
public:
    WaitForGoalReachedBT(const std::string &name, const BT::NodeConfiguration &config)
        : BT::StatefulActionNode(name, config)
    {
        node_ = rclcpp::Node::make_shared("wait_goal_reached_node");

        sub_ = node_->create_subscription<std_msgs::msg::Bool>(
            "/goal_reached", 10,
            [this](const std_msgs::msg::Bool::SharedPtr msg) {
                goal_reached_ = msg->data;
            });
    }

    static BT::PortsList providedPorts()
    {
        return {}; // no input/output ports needed
    }

    BT::NodeStatus onStart() override
    {
        RCLCPP_INFO(node_->get_logger(), "Waiting for goal reached signal...");
        goal_reached_ = false;
        return BT::NodeStatus::RUNNING;
    }

    BT::NodeStatus onRunning() override
    {
        rclcpp::spin_some(node_);

        if (goal_reached_)
        {
            RCLCPP_INFO(node_->get_logger(), "Goal reached!");
            return BT::NodeStatus::SUCCESS;
        }

        return BT::NodeStatus::RUNNING;
    }

    void onHalted() override
    {
        RCLCPP_WARN(node_->get_logger(), "WaitForGoalReachedBT halted");
    }

private:
    rclcpp::Node::SharedPtr node_;
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr sub_;
    bool goal_reached_ = false;
};
