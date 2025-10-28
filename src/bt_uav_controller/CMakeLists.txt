cmake_minimum_required(VERSION 3.8)
project(bt_uav_controller)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ===== Find dependencies =====
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav2_behavior_tree REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(px4_msgs REQUIRED)
find_package(behaviortree_cpp_v3 REQUIRED)

# ===== Include dirs =====
include_directories(
  include
)

# ===== Build executable =====
add_executable(bt_uav_controller
  src/bt_uav_controller.cpp
  src/px4_state_monitor.cpp
)

ament_target_dependencies(bt_uav_controller
  rclcpp
  nav2_behavior_tree
  geometry_msgs
  nav_msgs
  std_msgs
  tf2_ros
  px4_msgs
  behaviortree_cpp_v3
)

# ===== Install target =====
install(TARGETS
  bt_uav_controller
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
