/*
 * agent_node.cpp
 *
 * Implementation of the abstract AgentNode base class.
 * Replaces Agent.cpp — FreeRTOS task lifecycle mapped to ROS 2 timer.
 *
 * Original: Agent.cpp by Jon Durrant, 15-Aug-2022
 * ROS 2 port: 2026
 */

#include "blink_synk/agent_node.hpp"

AgentNode::AgentNode(const std::string & node_name)
: rclcpp::Node(node_name)
{
  RCLCPP_INFO(this->get_logger(), "AgentNode [%s] created", node_name.c_str());
}

AgentNode::~AgentNode()
{
  stop();
}

bool AgentNode::start(int period_ms)
{
  period_ms_ = period_ms;

  timer_ = this->create_wall_timer(
    std::chrono::milliseconds(period_ms_),
    std::bind(&AgentNode::timer_callback, this));

  RCLCPP_INFO(this->get_logger(), "Agent started with period %d ms", period_ms_);
  return true;
}

void AgentNode::stop()
{
  if (timer_) {
    timer_->cancel();
    timer_.reset();
    RCLCPP_INFO(this->get_logger(), "Agent stopped");
  }
}

void AgentNode::timer_callback()
{
  run();
}
