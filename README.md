Here is a **clean, copy-paste-ready README.md** you can directly use on GitHub.
It is written in a clear technical style and matches exactly what you described.

---

# UAV Localization and Autonomous Navigation Simulation (ROS 2 + PX4)

This repository contains a **UAV localization and autonomous navigation simulation** built using **ROS 2 Humble**, **PX4 Autopilot**, and **Gazebo Harmonic**, designed for a warehouse environment.
The system supports **SLAM-based mapping**, **autonomous navigation**, and **fully autonomous mission execution using Behavior Trees**.

---

## 📌 System Overview

The project integrates multiple software components into a single autonomous UAV architecture:

* PX4 Autopilot for flight control
* Gazebo Harmonic for simulation
* ROS 2 for middleware and system integration
* SLAM Toolbox for mapping and localization
* Nav2 for autonomous navigation
* Behavior Tree for high-level decision making (takeoff → navigate → land)

---

## ⚙️ Prerequisites

* Ubuntu 22.04 LTS
* ROS 2 Humble
* PX4 Autopilot (simulation setup)
* Gazebo Harmonic
* colcon build tools

Make sure your ROS 2 and PX4 environments are properly installed and sourced.

---

## 🔧 Environment Setup

### 1️⃣ Modify `.bashrc`

Add the following line to your `~/.bashrc` to set the warehouse world as default:

```bash
export PX4_GZ_WORLD=warehouse
```

After editing, reload the environment:

```bash
source ~/.bashrc
```

---

## 📦 PX4 Offboard Package

This project uses a **modified version** of `ROS2_PX4_Offboard_Example`.

* A **ZIP file** containing the modified `ROS2_PX4_Offboard_Example` package is attached to this repository.
* Please **extract it into your ROS 2 workspace `src/` directory** before building.

Example:

```bash
cd ~/ws_sensor_combined/src
unzip ROS2_PX4_Offboard_Example.zip
```

---

## 🛠 Build the Workspace

From your workspace root:

```bash
cd ~/ws_sensor_combined
rm -rf build install log
colcon build --symlink-install
source install/setup.bash
```

---

## 🚀 Running the System

Run the following commands **in separate terminals** (source the workspace in each terminal).

### 1️⃣ PX4 Offboard Control

```bash
ros2 launch px4_offboard offboard_velocity_control.launch.py
```

### 2️⃣ SLAM + Bridge (Gazebo ↔ ROS 2)

```bash
ros2 launch x500_bridge slam_with_bridge.launch.py
```

### 3️⃣ Navigation Stack (Nav2)

```bash
ros2 launch x500_navigation x500_nav_bringup.launch.py
```

### 4️⃣ Behavior Tree Controller (Full Autonomy)

```bash
ros2 run bt_uav_controller bt_uav_controller_node
```

---

## 🗺 Mapping Mode (SLAM)

To perform **mapping** instead of navigation:

1. Open the launch file:

   ```
   x500_bridge/slam_with_bridge.launch.py
   ```
2. **Uncomment** the following nodes:

   * `slam_toolbox`
   * `rviz`

This will enable real-time mapping and visualization using SLAM Toolbox.

---

## 📁 Project Structure (Simplified)

```
ws_sensor_combined/
├── src/
│   ├── ROS2_PX4_Offboard_Example/
│   ├── x500_bridge/
│   ├── x500_navigation/
│   ├── bt_uav_controller/
│   └── px4_msgs/
```

---

## 📌 Notes

* Ensure all ROS 2 nodes are launched in the correct order.
* Mapping and navigation modes should **not** be run simultaneously.
* Parameter tuning may be required for different warehouse layouts.
