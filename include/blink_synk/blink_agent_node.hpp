/*
 * blink_agent_node.hpp
 *
 * Active agent that periodically toggles a simulated LED and publishes
 * its state to a ROS 2 topic.
 * Replaces BlinkAgent.h — GPIO replaced with topic publisher.
 *
 * Original: BlinkAgent.h by Jon Durrant, 15-Aug-2022
 * ROS 2 port: 2026
 */

#ifndef BLINK_SYNK__BLINK_AGENT_NODE_HPP_
#define BLINK_SYNK__BLINK_AGENT_NODE_HPP_

#include <cstdint>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/bool.hpp"

#include "blink_synk/agent_node.hpp"

class BlinkAgentNode : public AgentNode
{
public:
  /**
   * Constructor
   * @param node_name  Name for the ROS 2 node
   * @param led_pad    GPIO pad identifier (used for topic naming & logging)
   */
  explicit BlinkAgentNode(
    const std::string & node_name,
    uint8_t led_pad = 0);

  /**
   * Destructor
   */
  virtual ~BlinkAgentNode();

protected:
  /**
   * Run loop — toggles LED state and publishes to topic.
   * Called every timer tick (500 ms by default).
   */
  void run() override;

  /** GPIO pad identifier (simulated) */
  uint8_t led_pad_;

  /** Current LED state */
  bool led_state_{false};

private:
  /** Publisher for LED state: /led_<pad>/state */
  rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr led_pub_;
};

#endif  // BLINK_SYNK__BLINK_AGENT_NODE_HPP_
