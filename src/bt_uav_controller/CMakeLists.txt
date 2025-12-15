cmake_minimum_required(VERSION 3.8)
project(bt_uav_controller)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ===== Dependencies =====
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(px4_msgs REQUIRED)
find_package(behaviortree_cpp_v3 REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

# ===== Generate ROS 2 Interface (Action) =====
rosidl_generate_interfaces(${PROJECT_NAME}
  "action/Takeoff.action"
  DEPENDENCIES std_msgs
)

# ===== Include dirs =====
include_directories(
  include
)

# ===== Behavior Tree Executable =====
add_executable(bt_uav_controller_node
  src/bt_uav_controller.cpp
  src/px4_state_monitor.cpp
)

ament_target_dependencies(bt_uav_controller_node
  rclcpp
  rclcpp_action
  geometry_msgs
  nav_msgs
  std_msgs
  tf2_ros
  px4_msgs
  behaviortree_cpp_v3
  nav2_msgs
)

rosidl_target_interfaces(bt_uav_controller_node
  ${PROJECT_NAME} "rosidl_typesupport_cpp")

# ===== Takeoff Action Server Executable =====
add_executable(takeoff_action_server
  src/takeoff_action_server.cpp
  src/px4_state_monitor.cpp
)

ament_target_dependencies(takeoff_action_server
  rclcpp
  rclcpp_action
  px4_msgs
)

rosidl_target_interfaces(takeoff_action_server
  ${PROJECT_NAME} "rosidl_typesupport_cpp")

# ===== Install Targets =====
install(TARGETS
  bt_uav_controller_node
  takeoff_action_server
  DESTINATION lib/${PROJECT_NAME}
)

# ===== Lint/Test Section =====
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
