# ⚙️ 3D Model & Controllers: Manual Launch Guide

This document provides a complete guide to launching the `motor_testbench` simulation using individual terminal commands. It also explains the core concepts behind `ros2_control` and how the URDF and YAML configuration files work together to bring the robot to life.

---

## 1. The `ros2_control` Architecture

The `ros2_control` framework is the standard way in ROS 2 to connect abstract control logic with physical (or simulated) hardware. It creates a clean separation between the controller algorithm (e.g., a PID controller) and the hardware driver. In our project, this is handled by two key files.

### A. The Hardware Interface (`modules/ros2_control.xacro`)

The `<ros2_control>` tag inside your URDF is where you declare your robot's "hardware" capabilities to the ROS 2 ecosystem.

```xml
<ros2_control name="GazeboSystem" type="system">
  <hardware>
    <plugin>gz_ros2_control/GazeboSimSystem</plugin>
  </hardware>
  <joint name="rotor_joint">
    <command_interface name="position">
      <param name="min">-1.57</param>
      <param name="max">1.57</param>
    </command_interface>
    <state_interface name="position"/>
    <state_interface name="velocity"/>
  </joint>
</ros2_control>
```

*   **`<plugin>gz_ros2_control/GazeboSimSystem</plugin>`**: This is the magic link. It's a special Gazebo plugin that acts as the "hardware driver" for our simulation. It reads joint states directly from Gazebo's physics engine and writes command setpoints back to it.
*   **`<command_interface name="position">`**: This line "exports" a controllable interface. It tells `ros2_control` that the `rotor_joint` can accept and execute incoming position commands.
*   **`<state_interface name="position"/>` & `<state_interface name="velocity"/>`**: These lines "export" readable data. They tell `ros2_control` that it can read the current position and velocity of the `rotor_joint` from the hardware (Gazebo).

In essence, the URDF (specifically modularized in `ros2_control.xacro`) defines the **available hardware resources** that controllers can use. 

> [!NOTE]
> The bridge between ROS 2 Control and Gazebo Sim is established in `modules/gazebo_properties.xacro` using the `gz_ros2_control` plugin, which explicitly points to our controller configuration file.


### B. The Controller Configuration (`motor_testbench_controllers.yaml`)

This YAML file configures the `controller_manager`—the central orchestrator that loads, starts, and stops controllers. It defines which controllers to use and which hardware resources they should claim.

```yaml
controller_manager:
  ros__parameters:
    update_rate: 100

    joint_state_broadcaster:
      type: joint_state_broadcaster/JointStateBroadcaster

    position_controller:
      type: position_controllers/JointGroupPositionController

joint_state_broadcaster:
  ros__parameters:
    use_urdf_to_filter: true

position_controller:
  ros__parameters:
    joints:
      - rotor_joint
```

*   **`controller_manager`**: This top-level entry configures the manager itself. We set `update_rate: 100`, meaning the entire "read -> update -> write" control loop will run at 100 Hz.
*   **`joint_state_broadcaster`**: This is a standard utility controller. Its only job is to claim all the `<state_interface>` resources it can find (`position` and `velocity` for `rotor_joint`) and publish them as a `sensor_msgs/msg/JointState` message on the `/joint_states` topic. This is crucial for nodes like `robot_state_publisher` to function.
*   **`position_controller`**: This is the active controller that does the work. It is of type `position_controllers/JointGroupPositionController`.
    *   It subscribes to a command topic (by default, `/position_controller/commands`).
    *   It claims the `<command_interface name="position">` for the joints listed under its `joints` parameter (in this case, `rotor_joint`).
    *   On every update loop, it calculates the effort needed to move the joint toward the desired position and writes that value to the hardware via the command interface.

---

## 2. Troubleshooting & Prerequisites

### A. The "Install" Trap
In ROS 2, Xacro and Gazebo use the `$(find package_name)` macro to locate files. However, this macro looks in the **`install/`** directory, not your `src/` directory.

**If you get a "File Not Found" or "Failed to load plugin" error in Gazebo:**
1.  Ensure your `config/` and `meshes/` folders are included in the `install(DIRECTORY ...)` section of your `CMakeLists.txt`.
2.  Always rebuild and source after making changes to non-code files (URDF, YAML, meshes):
    ```bash
    colcon build --packages-select core_description
    source install/setup.bash
    ```

---

## 3. "Old School" Manual Launch Guide

This process requires **six** separate terminals. In each new terminal, you must first source your workspace: `source /root/ros2_ws/install/setup.bash`.

### Terminal 1: Start Gazebo

Start the modern Gazebo (Gazebo Sim) server and client. We must first export the resource and plugin paths so Gazebo can find our 3D meshes and the `gz_ros2_control` library.

```bash
# 1. Export paths (Repeat this if you open a new terminal for Gazebo)
export GZ_SIM_RESOURCE_PATH=$GZ_SIM_RESOURCE_PATH:/root/ros2_ws/install/core_description/share
export GZ_SIM_SYSTEM_PLUGIN_PATH=$GZ_SIM_SYSTEM_PLUGIN_PATH:/opt/ros/jazzy/lib

# 2. Start Gazebo Sim
gz sim -v 4 -r empty.sdf
```

### Terminal 2: Start the Clock Bridge

Gazebo and ROS 2 speak different languages. We need a bridge to bring the simulation time from Gazebo into ROS so that all our nodes stay synchronized.

```bash
ros2 run ros_gz_bridge parameter_bridge /clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock
```

### Terminal 3: Start Robot State Publisher & Load URDF

We start the `robot_state_publisher` and pass the URDF string directly. We use **single quotes** around the Xacro output to ensure the ROS 2 parameter parser treats the entire XML as a single string and doesn't get confused by XML characters.

```bash
ros2 run robot_state_publisher robot_state_publisher --ros-args -p use_sim_time:=true -p "robot_description:='$(xacro /root/ros2_ws/src/gazebo_sim/gazebo_core/core_description/urdf/motor_testbench/motor_testbench.urdf.xacro)'"
```

### Terminal 4: Spawn the Robot into Gazebo

With the `/robot_description` topic now active, we use the `ros_gz_sim` bridge to create an instance of our robot in the running Gazebo world.

```bash
ros2 run ros_gz_sim create -world empty -topic robot_description -name motor_testbench
```
> At this point, the `gz_ros2_control` plugin starts and a `/controller_manager` node is created. You should see your model in Gazebo.

### Terminal 5: Load and Start the Controllers

The `controller_manager` is running inside the Gazebo process, but its controllers are inactive. Use the `ros2 control` CLI to load and start the two controllers defined in the YAML file.

```bash
# Run these two commands one after the other
ros2 control load_controller --set-state active joint_state_broadcaster
ros2 control load_controller --set-state active position_controller
```
> Now the `/joint_states` and `/position_controller/commands` topics will become active.

### Terminal 6: Send a Command and Verify

With the system fully running, publish a command to the `position_controller` to move the rotor to 1.57 radians (90 degrees).

```bash
ros2 topic pub /position_controller/commands std_msgs/msg/Float64MultiArray "{data: [1.57]}"
```
> You should see the rotor on your model spin to the 90-degree position in the Gazebo window.