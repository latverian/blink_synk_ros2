/*
 * blink_agent_node.cpp
 *
 * Implementation of BlinkAgentNode — simulates LED blink by publishing
 * Bool messages to /led_<pad>/state topic.
 *
 * Original: BlinkAgent.cpp by Jon Durrant, 15-Aug-2022
 * ROS 2 port: 2026
 */

#include "blink_synk/blink_agent_node.hpp"

BlinkAgentNode::BlinkAgentNode(
  const std::string & node_name,
  uint8_t led_pad)
: AgentNode(node_name),
  led_pad_(led_pad)
{
  // Create publisher: /led_<pad>/state
  std::string topic_name = "/led_" + std::to_string(led_pad_) + "/state";
  led_pub_ = this->create_publisher<std_msgs::msg::Bool>(topic_name, 10);

  RCLCPP_INFO(
    this->get_logger(),
    "BlinkAgentNode [%s] → LED pad %d, topic: %s",
    node_name.c_str(), led_pad_, topic_name.c_str());
}

BlinkAgentNode::~BlinkAgentNode()
{
  stop();
}

void BlinkAgentNode::run()
{
  // Toggle LED state (replaces gpio_put)
  led_state_ = !led_state_;

  // Publish state
  auto msg = std_msgs::msg::Bool();
  msg.data = led_state_;
  led_pub_->publish(msg);

  // Log state change (replaces physical LED observation)
  RCLCPP_INFO(
    this->get_logger(),
    "LED %d → %s",
    led_pad_,
    led_state_ ? "ON ███" : "OFF ___");
}
