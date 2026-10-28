# BlinkSynk on ROS 2

A ROS 2 port of the **BlinkSynk** FreeRTOS exercise from Jon Durrant's Raspberry Pi Pico FreeRTOS course. It keeps the same task-synchronization pattern, rebuilt from FreeRTOS tasks and notifications into ROS 2 nodes, timers and services.

> Companion project: [micro_ros_assignment](https://github.com/latverian/micro_ros_assignment) runs the original FreeRTOS version on a Raspberry Pi Pico and publishes its state to ROS 2 through micro-ROS.

![ROS 2 Humble](https://img.shields.io/badge/ROS_2-Humble-22314E?style=flat-square&logo=ros&logoColor=white)
![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![rclcpp](https://img.shields.io/badge/rclcpp-MultiThreadedExecutor-555555?style=flat-square)

## What it does

A single process starts three nodes on a `MultiThreadedExecutor`, each driven by a 500 ms timer:

| Node | LED pad | Behaviour |
| :--- | :---: | :--- |
| `blink_agent` | 2 | Toggles its LED every tick and publishes the state on `/led_2/state` (`std_msgs/Bool`) |
| `worker_1` | 3 | Blinks, does a random amount of CPU work, notifies `worker_2`, then waits for `worker_2` |
| `worker_2` | 4 | Same, mirrored with `worker_1` |

The two workers take turns. Each one can only blink again after its peer has notified it, so they stay in lock-step no matter how long the random work takes. The LEDs are simulated: the agent publishes its state to a topic, and the workers log theirs.

## FreeRTOS → ROS 2 mapping

| FreeRTOS (original) | ROS 2 (this port) |
| :--- | :--- |
| Task created by `Agent::start()` | Node with a wall timer, created by `AgentNode::start()` |
| `vTaskStartScheduler()` | `MultiThreadedExecutor::spin()` |
| `vTaskDelay(500)` | 500 ms timer period |
| `gpio_put(pad, on)` | `/led_<pad>/state` message or a log line |
| `xTaskNotifyGive(peer)` | Async call to `/<peer>/notify` (`std_srvs/Trigger`) |
| `ulTaskNotifyTake(pdTRUE, delay)` | Atomic notify counter, checked on every timer tick |

## Worker state machine

```text
BLINK_ON ──► BLINK_OFF_WORK ──► WAIT_PEER ──► BLINK_ON ...
 LED on       LED off,           stays here until
              random CPU work,   the peer's notify
              notify the peer    arrives
```

## Build and run

Requires ROS 2 Humble and `colcon`.

```bash
# in your colcon workspace
cd ~/ros2_ws/src
git clone https://github.com/latverian/blink_synk_ros2.git
cd ~/ros2_ws
colcon build --packages-select blink_synk
source install/setup.bash

ros2 launch blink_synk blink_synk_launch.py
# or: ros2 run blink_synk blink_synk_node
```

In a second terminal:

```bash
ros2 topic echo /led_2/state                              # the agent's LED
ros2 service list | grep notify                           # /worker_1/notify, /worker_2/notify
ros2 service call /worker_1/notify std_srvs/srv/Trigger   # nudge worker_1 by hand
```

## Layout

```text
include/blink_synk/   agent_node.hpp · blink_agent_node.hpp · blink_worker_node.hpp
src/                  main.cpp and the node implementations
launch/               blink_synk_launch.py
```

## Credits

Based on BlinkSynk from [jondurrant/RPIPicoFreeRTOSCourse](https://github.com/jondurrant/RPIPicoFreeRTOSCourse) (MIT). This ROS 2 port is by [Faril Pirwanhadi](https://github.com/latverian) and is also MIT-licensed, as declared in `package.xml`.
