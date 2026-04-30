/*
 * agent_node.hpp
 *
 * Abstract agent interface translated from FreeRTOS task wrapper to ROS 2 node.
 * Replaces Agent.h — each "agent" is now a rclcpp::Node with a wall timer
 * that drives its run loop.
 *
 * Original: Agent.h by Jon Durrant, 15-Aug-2022
 * ROS 2 port: 2026
 */

#ifndef BLINK_SYNK__AGENT_NODE_HPP_
#define BLINK_SYNK__AGENT_NODE_HPP_

#include <string>
#include <memory>

#include "rclcpp/rclcpp.hpp"

class AgentNode : public rclcpp::Node
{
public:
  /**
   * Constructor
   * @param node_name  Name for the ROS 2 node
   */
  explicit AgentNode(const std::string & node_name);

  /**
   * Destructor
   */
  virtual ~AgentNode();

  /**
   * Start the agent's periodic timer (replaces FreeRTOS xTaskCreate).
   * @param period_ms  Timer period in milliseconds
   * @return true if started successfully
   */
  virtual bool start(int period_ms = 500);

  /**
   * Stop the agent's timer (replaces FreeRTOS vTaskDelete).
   */
  virtual void stop();

protected:
  /**
   * Called every timer tick. Subclasses implement their logic here.
   * Replaces the FreeRTOS run() pure-virtual.
   */
  virtual void run() = 0;

private:
  /** Timer callback that delegates to run() */
  void timer_callback();

  /** Wall timer driving the run loop */
  rclcpp::TimerBase::SharedPtr timer_;

  /** Timer period in milliseconds */
  int period_ms_{500};
};

#endif  // BLINK_SYNK__AGENT_NODE_HPP_
