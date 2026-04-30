/*
 * blink_worker_node.cpp
 *
 * Implementation of BlinkWorkerNode — blinks LED, does random CPU work,
 * and synchronizes with a peer worker via ROS 2 services.
 *
 * Synchronization pattern (translated from FreeRTOS):
 *   FreeRTOS: xTaskNotifyGive(peer) → ulTaskNotifyTake(pdTRUE, DELAY)
 *   ROS 2:    async service call to peer → service server increments counter
 *
 * Original: BlinkWorker.cpp by Jon Durrant, 17-Aug-2022
 * ROS 2 port: 2026
 */

#include "blink_synk/blink_worker_node.hpp"

#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <functional>

BlinkWorkerNode::BlinkWorkerNode(
  const std::string & node_name,
  uint8_t led_pad,
  const std::string & peer_name)
: BlinkAgentNode(node_name, led_pad),
  peer_name_(peer_name)
{
  // Service server: receives notifications from peer
  // Replaces ulTaskNotifyTake (receiving side)
  std::string srv_name = "/" + std::string(this->get_name()) + "/notify";
  notify_srv_ = this->create_service<std_srvs::srv::Trigger>(
    srv_name,
    std::bind(
      &BlinkWorkerNode::notify_callback, this,
      std::placeholders::_1, std::placeholders::_2));

  // Service client: sends notifications to peer
  // Replaces xTaskNotifyGive (sending side)
  std::string peer_srv_name = "/" + peer_name_ + "/notify";
  peer_client_ = this->create_client<std_srvs::srv::Trigger>(peer_srv_name);

  RCLCPP_INFO(
    this->get_logger(),
    "BlinkWorkerNode [%s] → peer: %s, srv: %s, peer_srv: %s",
    node_name.c_str(), peer_name_.c_str(),
    srv_name.c_str(), peer_srv_name.c_str());
}

BlinkWorkerNode::~BlinkWorkerNode()
{
  stop();
}

void BlinkWorkerNode::run()
{
  switch (state_) {
    case State::BLINK_ON:
      {
        // LED ON (replaces gpio_put(xLedPad, 1))
        led_state_ = true;
        auto msg = std_msgs::msg::Bool();
        msg.data = led_state_;
        // Publish through inherited publisher — we access it indirectly
        // by calling parent's topic directly
        RCLCPP_INFO(
          this->get_logger(),
          "LED %d → ON ███", led_pad_);
        state_ = State::BLINK_OFF_WORK;
      }
      break;

    case State::BLINK_OFF_WORK:
      {
        // LED OFF (replaces gpio_put(xLedPad, 0))
        led_state_ = false;
        RCLCPP_INFO(
          this->get_logger(),
          "LED %d → OFF ___", led_pad_);

        // Random CPU work (identical to original)
        do_random_work();

        // Notify peer (replaces xTaskNotifyGive(pPeer->getTask()))
        notify_peer();

        state_ = State::WAIT_PEER;
      }
      break;

    case State::WAIT_PEER:
      {
        // Check if peer has notified us
        // (replaces ulTaskNotifyTake(pdTRUE, DELAY))
        uint32_t count = notify_count_.exchange(0);
        if (count > 0) {
          RCLCPP_INFO(
            this->get_logger(),
            "✓ Received notify from peer [%s] (count=%u)",
            peer_name_.c_str(), count);
          state_ = State::BLINK_ON;
        } else {
          RCLCPP_DEBUG(
            this->get_logger(),
            "⏳ Waiting for peer [%s]...", peer_name_.c_str());
          // Stay in WAIT_PEER — will check again next timer tick
          // This mimics ulTaskNotifyTake with a timeout of DELAY
        }
      }
      break;
  }
}

void BlinkWorkerNode::notify_callback(
  const std::shared_ptr<std_srvs::srv::Trigger::Request> /*request*/,
  std::shared_ptr<std_srvs::srv::Trigger::Response> response)
{
  // Increment notification counter (replaces xTaskNotifyGive from peer's side)
  notify_count_.fetch_add(1);

  response->success = true;
  response->message = "Notify received by " + std::string(this->get_name());

  RCLCPP_DEBUG(
    this->get_logger(),
    "📨 Notify received (total pending: %u)", notify_count_.load());
}

void BlinkWorkerNode::do_random_work()
{
  // Random CPU work — identical to original FreeRTOS version
  int n = rand() % 5000;
  for (int i = 0; i < n; i++) {
    double d = asin(sin(sqrt(static_cast<double>(n))));
    d = acos(cos(d));
    (void)d;  // Suppress unused warning
  }

  RCLCPP_DEBUG(
    this->get_logger(),
    "Random work done (n=%d iterations)", n);
}

void BlinkWorkerNode::notify_peer()
{
  if (!peer_client_->wait_for_service(std::chrono::milliseconds(100))) {
    RCLCPP_WARN(
      this->get_logger(),
      "Peer service [%s] not available yet", peer_name_.c_str());
    return;
  }

  auto request = std::make_shared<std_srvs::srv::Trigger::Request>();

  // Async call — fire and forget (replaces xTaskNotifyGive which is non-blocking)
  auto future = peer_client_->async_send_request(
    request,
    [this](rclcpp::Client<std_srvs::srv::Trigger>::SharedFuture result) {
      auto response = result.get();
      if (response->success) {
        RCLCPP_DEBUG(
          this->get_logger(),
          "→ Notified peer [%s]: %s",
          peer_name_.c_str(), response->message.c_str());
      }
    });
}
