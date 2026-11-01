cmake_minimum_required(VERSION 3.20)
project(turtle_navigator_interfaces)

find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(builtin_interfaces REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/Goal.msg"
  "msg/NavigationStatus.msg"
  "srv/ValidateGoal.srv"
  "action/NavigateToGoal.action"
  DEPENDENCIES builtin_interfaces
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()
