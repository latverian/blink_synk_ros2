/*
 * blink_worker_node.hpp
 *
 * Worker node that blinks an LED, performs random CPU work, and synchronizes
 * with a peer worker via ROS 2 services.
 * Replaces BlinkWorker.h — xTaskNotifyGive/ulTaskNotifyTake mapped to
 * ROS 2 service calls (std_srvs/Trigger).
 *
 * Original: BlinkWorker.h by Jon Durrant, 17-Aug-2022
 * ROS 2 port: 2026
 */

#ifndef BLINK_SYNK__BLINK_WORKER_NODE_HPP_
#define BLINK_SYNK__BLINK_WORKER_NODE_HPP_

#include <string>
#include <mutex>
#include <atomic>

#include "rclcpp/rclcpp.hpp"
#include "std_srvs/srv/trigger.hpp"

#include "blink_synk/blink_agent_node.hpp"

class BlinkWorkerNode : public BlinkAgentNode
{
public:
  /**
   * Constructor
   * @param node_name  Name for the ROS 2 node
   * @param led_pad    GPIO pad identifier
   * @param peer_name  Name of the peer worker node (for service calls)
   */
  explicit BlinkWorkerNode(
    const std::string & node_name,
    uint8_t led_pad,
    const std::string & peer_name);

  /**
   * Destructor
   */
  virtual ~BlinkWorkerNode();

protected:
  /**
   * Run loop — blink LED, do random work, notify peer, wait for peer.
   * Replaces BlinkWorker::run().
   */
  void run() override;

private:
  /**
   * Service callback — called when peer notifies us.
   * Replaces ulTaskNotifyTake (receiving side).
   */
  void notify_callback(
    const std::shared_ptr<std_srvs::srv::Trigger::Request> request,
    std::shared_ptr<std_srvs::srv::Trigger::Response> response);

  /**
   * Perform random CPU work (identical to original FreeRTOS version).
   * sin/cos/sqrt loop with random iteration count.
   */
  void do_random_work();

  /**
   * Notify the peer worker via service call.
   * Replaces xTaskNotifyGive (sending side).
   */
  void notify_peer();

  /** Service server: /<node_name>/notify — receives notifications from peer */
  rclcpp::Service<std_srvs::srv::Trigger>::SharedPtr notify_srv_;

  /** Service client: /<peer_name>/notify — sends notifications to peer */
  rclcpp::Client<std_srvs::srv::Trigger>::SharedPtr peer_client_;

  /** Name of the peer worker */
  std::string peer_name_;

  /** Counter for received notifications from peer */
  std::atomic<uint32_t> notify_count_{0};

  /** State machine for the worker's blink cycle */
  enum class State {
    BLINK_ON,       // LED just turned ON
    BLINK_OFF_WORK, // LED OFF + random work + notify peer
    WAIT_PEER       // Waiting for peer notification
  };
  State state_{State::BLINK_ON};
};

#endif  // BLINK_SYNK__BLINK_WORKER_NODE_HPP_
