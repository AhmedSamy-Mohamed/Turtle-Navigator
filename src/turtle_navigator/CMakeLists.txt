cmake_minimum_required(VERSION 3.20)
project(turtle_navigator)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(turtlesim_msgs REQUIRED)
find_package(turtle_navigator_interfaces REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Goal Validator
add_executable(goal_validator
  src/goal_validator.cpp
)

target_link_libraries(goal_validator
  rclcpp::rclcpp
  turtle_navigator_interfaces::turtle_navigator_interfaces
)

# Navigation Action Server
add_executable(navigation_server
  src/navigation_server.cpp
)

target_link_libraries(navigation_server
  rclcpp::rclcpp
  rclcpp_action::rclcpp_action
  geometry_msgs::geometry_msgs
  turtlesim_msgs::turtlesim_msgs
  turtle_navigator_interfaces::turtle_navigator_interfaces
)

# Qt Goal Input GUI
add_executable(goal_input
  src/goal_input.cpp
)

target_link_libraries(goal_input
  rclcpp::rclcpp
  rclcpp_action::rclcpp_action
  turtle_navigator_interfaces::turtle_navigator_interfaces
  Qt6::Widgets
)

# Results & Monitoring Node
add_executable(results_monitor
  src/results_monitor.cpp
)

target_link_libraries(results_monitor
  rclcpp::rclcpp
  turtle_navigator_interfaces::turtle_navigator_interfaces
)

# Install all executables
install(TARGETS
  goal_validator
  navigation_server
  goal_input
  results_monitor
  DESTINATION lib/${PROJECT_NAME}
)

# Install Launch Files
install(
  DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
