![ROS2](https://img.shields.io/badge/ROS2-Jazzy-blue)
![Docker](https://img.shields.io/badge/Docker-Enabled-blue?logo=docker)
![Ubuntu](https://img.shields.io/badge/Ubuntu-24.04-orange?logo=ubuntu)

# 🗺️ The Comprehensive ROS 2 Master Roadmap (2026 Edition)

> **How to use:** In VS Code, press `Ctrl+Shift+V` to see the preview. To check an item, add an `x` between the brackets: `[x]`.

## 🛠 My Current Tech Stack
* **Language:** C++ (Primary), Python (Scripting)
* **Distro:** ROS 2 Jazzy Jalisco
* **OS:** Ubuntu 24.04 (Dockerized)
* **IDE:** VS Code Insiders + RDE-Pack

---

## 🟢 LEVEL 1: CORE CONCEPTS (The Communication Graph)
- [ ] **Linux & Docker:** Terminal mastery, file permissions, and containerizing ROS 2.
- [x] **Workspace Anatomy:** Mastering `colcon build`, sourcing, and package structures.
- [x] **Topics (Pub/Sub):** Asynchronous data streams (e.g., Sensor data).
- [x] **Services (Req/Res):** Synchronous immediate tasks (e.g., Toggle a LED).
- [ ] **Actions (Goal/Feedback/Result):** Long-running, cancelable tasks (The "Big 4" logic).
- [ ] **Launch System:** Using Python/XML to automate starting multiple nodes at once.
- [ ] **Parameters:** Handling node configurations and YAML parameter files.
- [ ] **ROS 2 CLI:** Deep expertise in `ros2 node`, `topic`, `service`, `action`, and `param`.
- [ ] **Interfaces:** Creating custom `.msg`, `.srv`, and `.action` definitions.

---

## 🟡 LEVEL 2: MOBILE ROBOTICS (The Simulation)
- [ ] **URDF & Xacro:** Modeling a mobile robot with wheels, sensors, and physical properties.
- [ ] **TF2 Transforms:** Managing the tree: `map` -> `odom` -> `base_link` -> `laser`.
- [ ] **Launch Mastery:** Passing arguments and remapping topics through launch files.
- [ ] **Gazebo Sim:** Adding plugins for Differential Drive, Lidar, and IMU sensors.
- [ ] **Rviz2:** Configuring 3D visualizations for sensor data and transform trees.
- [ ] **Action Utilization:** Using actions to command specific movements in simulation.
- [ ] **Rosbag2:** Recording simulated data for offline debugging and playback.

---

## 🟠 LEVEL 3: PRO (Hardware & Autonomy)
- [ ] **micro-ROS:** Bridging ROS 2 to microcontrollers for real-time control.
- [ ] **Nav2 (Navigation):** Setting up Path Planning and Obstacle Avoidance.
- [ ] **SLAM:** Using `Slam Toolbox` to generate occupancy grid maps.
- [ ] **Localization:** Implementing `AMCL` or `Robot Localization` (EKF).
- [ ] **ros2_control:** Writing Hardware Interfaces for physical motors.
- [ ] **Behavior Trees:** Managing high-level robot logic with `BehaviorTree.CPP`.

---

## 🦾 LEVEL 4: MOVEIT 2 (Manipulation Mastery)
- [ ] **MoveIt Setup Assistant:** Generating configuration packages for robotic arms.
- [ ] **MoveGroup Interface:** Programming arm movements in C++ and Python.
- [ ] **Kinematics (IK/FK):** Solving Inverse Kinematics with KDL, TRAC-IK, or PickNik solvers.
- [ ] **Collision Checking:** Adding dynamic obstacles to the Planning Scene.
- [ ] **MoveIt Task Constructor:** Building multi-stage tasks (e.g., Pick -> Move -> Place).
- [ ] **MoveIt Servo:** Enabling real-time "jogging" or teleoperation of the arm.
- [ ] **Perception Integration:** Using Octomaps or Point Clouds for 3D environment awareness.

---

## 🏗️ LEVEL DEV: ARCHITECT (Production & Optimization)
- [ ] **Lifecycle Nodes:** State management (Unconfigured -> Inactive -> Active).
- [ ] **Zenoh Middleware:** Replacing/augmenting DDS with high-performance Zenoh.
- [ ] **DDS/QoS Tuning:** Optimizing "Quality of Service" for unreliable wireless links.
- [ ] **Real-Time Linux:** Working with `PREEMPT_RT` kernels for 1kHz+ control loops.
- [ ] **SROS2:** Implementing encrypted communication and node-level security.
- [ ] **Custom Plugins:** Writing C++ plugins for Nav2 planners or MoveIt solvers.
- [ ] **Tesseract:** Exploring industrial alternatives for high-complexity manufacturing.

---

## 📚 Quick Resources
* [ROS 2 Jazzy Official Documentation](https://docs.ros.org/en/jazzy/)
* [MoveIt 2 Documentation](https://moveit.picknik.ai/main/index.html)
* [Nav2 Documentation](https://docs.nav2.org/)