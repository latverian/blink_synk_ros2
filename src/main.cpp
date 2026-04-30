/*
 * main.cpp
 *
 * Entry point for the BlinkSynk ROS 2 node.
 * Creates 3 nodes (1 BlinkAgent + 2 BlinkWorkers) and runs them
 * in a MultiThreadedExecutor.
 *
 * Replaces the FreeRTOS main.cpp — vTaskStartScheduler() mapped to
 * executor.spin(), and mainTask() mapped to node construction + start().
 *
 * Original: main.cpp by Jon Durrant, 15-Aug-2022
 * ROS 2 port: 2026
 */

#include <cstdio>
#include <memory>
#include <chrono>

#include "rclcpp/rclcpp.hpp"

#include "blink_synk/blink_agent_node.hpp"
#include "blink_synk/blink_worker_node.hpp"

// LED PAD identifiers (same as original FreeRTOS project)
#define LED_PAD     2
#define LED1_PAD    3
#define LED2_PAD    4

// Blink delay in ms (same as original: 500ms)
#define BLINK_DELAY 500

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);

  printf("GO\n");
  printf("Starting ROS 2 BlinkSynk (ported from FreeRTOS):\n");

  // Create nodes (replaces mainTask in original FreeRTOS code)
  //   BlinkAgent  — simple LED blinker on pad 2
  //   Worker 1    — LED blinker on pad 3, peers with Worker 2
  //   Worker 2    — LED blinker on pad 4, peers with Worker 1
  auto blink_agent = std::make_shared<BlinkAgentNode>("blink_agent", LED_PAD);
  auto worker1 = std::make_shared<BlinkWorkerNode>("worker_1", LED1_PAD, "worker_2");
  auto worker2 = std::make_shared<BlinkWorkerNode>("worker_2", LED2_PAD, "worker_1");

  // Start all agents with 500ms timer period
  // (replaces agent.start("name", TASK_PRIORITY))
  blink_agent->start(BLINK_DELAY);
  worker1->start(BLINK_DELAY);
  worker2->start(BLINK_DELAY);

  // Use MultiThreadedExecutor to run all nodes concurrently
  // (replaces vTaskStartScheduler())
  rclcpp::executors::MultiThreadedExecutor executor;
  executor.add_node(blink_agent);
  executor.add_node(worker1);
  executor.add_node(worker2);

  RCLCPP_INFO(
    blink_agent->get_logger(),
    "All nodes started. Spinning executor...");

  // Spin until shutdown (Ctrl+C)
  // This replaces the infinite while(true) + vTaskDelay(3000) in mainTask
  executor.spin();

  // Cleanup
  rclcpp::shutdown();
  printf("BlinkSynk shutdown complete.\n");

  return 0;
}
