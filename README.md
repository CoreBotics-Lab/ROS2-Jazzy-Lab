# ROS 2 Robotics Development and Learning Repository

This repository contains the practical code implementations and technical documentation of what I have learned across foundational and intermediate robotics engineering in ROS 2 Jazzy, covering mobile robot kinematics, physics simulation, and computational graph architecture.

---

## Basic Level: Core ROS 2 Computational Graph & Execution

### 1. Communication Primitives
- **Publish-Subscribe Architecture**: Asynchronous, decoupled data streaming for sensors and state telemetry implemented in C++ and Python.
- **Client-Service Paradigm**: Synchronous request-response communication for deterministic triggering, resetting, and service-based coordination.
- **Action Server and Client**: Non-blocking execution of long-running, preemptible tasks featuring continuous feedback loops, goal cancellation, and result reporting.
- **Custom Interface Definitions**: Implementation of application-specific message (`.msg`), service (`.srv`), and action (`.action`) specifications using ROS IDL.

### 2. Configuration & Parameter Management
- **Dynamic Parameters**: Declaring, reading, and dynamically reconfiguring node parameters at runtime.
- **Hierarchical Configuration Files**: Managing parameters externally via structured YAML files to decouple system configuration from node implementation.
- **Launch Architectures**: Automated multi-node orchestration, parameter loading, and namespace management using Python-based launch configurations.

### 3. Concurrency & Execution Control
- **Execution Engines**: Operational characteristics and thread safety trade-offs between `SingleThreadedExecutor` and `MultiThreadedExecutor`.
- **Callback Groups**: Concurrency control and race condition prevention using `MutuallyExclusiveCallbackGroup` (serialized callbacks) and `ReentrantCallbackGroup` (parallel dispatch) to prevent deadlock and callback starvation.

---

## Intermediate Level: Mobile Robotics, Kinematics & Simulation

### 4. Spatial Kinematics & Coordinate Frames
- **Transform Trees (TF2)**: Establishing hierarchical spatial relationships between reference frames (`map`, `odom`, `base_link`, and sensor frames).
- **Broadcasting & Listening**: Static and dynamic transform broadcasting, transform buffering, and coordinate frame re-projection across moving frames.

### 5. Robot Modeling
- **URDF / Xacro Development**: Constructing parametric robot kinematic chains comprising links, joints, visual meshes, and collision geometry.
- **Inertial Parameter Modeling**: Calculating and assigning physical inertial properties, including mass, center of mass, and moment of inertia tensors for dynamic simulation fidelity.

### 6. Differential Drive Kinematics
- **Forward Kinematics**: Deriving Cartesian linear and angular velocities ($\dot{x}, \dot{y}, \dot{\theta}$) from wheel angular velocities ($\omega_L, \omega_R$).
- **Inverse Kinematics**: Decomposing commanded velocity inputs ($v, \omega$) into individual wheel angular speeds using algebraic and matrix formulations:
  $$\begin{bmatrix} \omega_R \\ \omega_L \end{bmatrix} = \frac{1}{r} \begin{bmatrix} 1 & \frac{b}{2} \\ 1 & -\frac{b}{2} \end{bmatrix} \begin{bmatrix} v \\ \omega \end{bmatrix}$$
  where $r$ represents wheel radius and $b$ denotes the track baseline.
- **Wheel Odometry**: Dead-reckoning integration of wheel encoder telemetry to compute continuous robot pose estimates over time.

### 7. Physics Simulation
- **Gazebo Sim Integration**: Constructing simulation worlds, defining physical contact dynamics, surface friction, and gravity parameters.
- **Sensor & Actuator Simulation**: Simulating differential drive physics, joint state feedback, and sensor data pipelines in Gazebo environments.
