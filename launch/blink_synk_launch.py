"""
Launch file for blink_synk package.

Launches all 3 nodes:
  - blink_agent  (simple LED blinker, pad 2)
  - worker_1     (synced worker, pad 3, peers with worker_2)
  - worker_2     (synced worker, pad 4, peers with worker_1)

Usage:
  ros2 launch blink_synk blink_synk_launch.py
"""

from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription([
        # BlinkAgent — simple LED blinker (replaces BlinkAgent on pad 2)
        Node(
            package='blink_synk',
            executable='blink_synk_node',
            name='blink_synk',
            output='screen',
            emulate_tty=True,
            parameters=[],
        ),
    ])
