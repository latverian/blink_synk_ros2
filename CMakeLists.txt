cmake_minimum_required(VERSION 3.8)
project(blink_synk)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)

# Include headers
include_directories(include)

# Executable
add_executable(blink_synk_node
  src/main.cpp
  src/agent_node.cpp
  src/blink_agent_node.cpp
  src/blink_worker_node.cpp
)

ament_target_dependencies(blink_synk_node
  rclcpp
  std_msgs
  std_srvs
)

# Install executable
install(TARGETS
  blink_synk_node
  DESTINATION lib/${PROJECT_NAME}
)

# Install launch files
install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}/
)

# Install headers
install(DIRECTORY
  include/
  DESTINATION include/
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
