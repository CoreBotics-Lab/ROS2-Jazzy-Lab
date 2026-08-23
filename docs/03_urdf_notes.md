# URDF & TF Design Principles

*A collection of core realizations about building professional robot models in ROS 2.*

## 1. The Golden Rule of URDF
**Joints are just TF placements (math). Visuals and Collisions are just decorations.**
When building a robot, stop thinking about where the plastic parts connect, and start asking: *"Where does the math actually happen?"* 
That mathematical center is exactly where your `<joint>` origin must go. Once the joint is placed correctly, you can shift the `<visual>` and `<collision>` origins to make the 3D model wrap around that joint properly.

## 2. Wheels vs. Arms
The placement of the joint changes depending on the kinematics of the part.

*   **Wheels / Rollers:** The math happens at the **axis of rotation**. The joint MUST be placed in the absolute center of the wheel. If the visual origin of the wheel is `0 0 0`, it will perfectly spin around its center.
*   **Arms / Shoulders:** The math happens at the **hinge** (the edge). The joint MUST be placed at the connecting point. To make a cylinder swing like an arm, you must offset the visual geometry inside the link by half its length (e.g., `<origin xyz="0 0 ${length/2}">`).

## 3. The "Fake Link" (Optical Frame) Pattern for Sensors
Sensors introduce a dilemma: The physical mounting point of the sensor (where the screws go) is rarely the exact center of the sensing component (the laser, the camera lens, etc.).

**The Solution:** Decouple the physical body from the mathematical sensor.
1.  **The Physical Link (e.g., `scan_link` or `camera_link`):** This holds the visual 3D model and collision mesh. Its joint is placed exactly where it mounts to the chassis.
2.  **The Fake Link (e.g., `scan_optical_frame` or `camera_optical_frame`):** This is an invisible, weightless dummy link. Its joint shifts the frame from the physical mount to the *exact mathematical center* where the lasers/pixels are measured.
3.  **The Plugin & The Rendering Quirk:** In modern Gazebo Sim, a sensor *always* renders looking down the **+X axis** of the link it is attached to. 
    *   **For Lidar:** Attach the `<gazebo reference="scan_optical_frame">` to the optical frame, because Lidar math is naturally X-forward.
    *   **For Cameras:** Attach the `<gazebo reference="camera_link">` to the *physical* frame (so it renders looking physically forward). Then, inside the sensor, use `<gz_frame_id>camera_link_optical</gz_frame_id>`. 

**The Magic Result for Cameras:** Gazebo renders the image looking physically forward (X-axis), but before sending the data to ROS, it stamps the message with the `camera_link_optical` frame ID. RViz receives it, applies the computer vision math (Z-axis forward, thanks to your `rpy="-1.5708 0 -1.5708"` joint offset), and perfectly projects the image right-side up!

## 4. Gazebo Sim Sensor Dictionary
When adding new sensors, don't guess the XML tags. Look up the official SDFormat Specification dictionary:
[SDFormat Sensor Specification](https://sdformat.org/spec/1.12/sensor/)

*   Use `gpu_lidar` instead of `lidar` for hardware acceleration.
*   Modern Gazebo Sim does NOT require ROS plugins inside the URDF `<sensor>` tag. It uses native sensors bridged via `ros_gz_bridge`.

## 5. The "ros_gz_bridge" Workflow
When you add a new sensor or plugin to Gazebo Sim, it publishes data natively in Gazebo, *not* ROS. To map it to ROS 2, do NOT guess the message types. Follow this exact 3-step professional workflow:

**Step 1: Find the Gazebo Topic**
Run the simulation and list the active Gazebo topics:
```bash
gz topic -l
```
*(Example: You see `/model/robot_2wd/tf` in the list.)*

**Step 2: Find the Gazebo Message Type**
Inspect that specific topic to see what data format Gazebo is using:
```bash
gz topic -i -t <topic_name>
```
*(Example: `gz topic -i -t /model/robot_2wd/tf` outputs `Type: gz.msgs.Pose_V`)*

**Step 3: Find the ROS 2 Translation**
Go to the official [ros_gz_bridge GitHub repository](https://github.com/gazebosim/ros_gz/tree/ros2/ros_gz_bridge) and check their mapping table. Look up the Gazebo type (`gz.msgs.Pose_V`) to find the exact ROS 2 equivalent (`tf2_msgs/msg/TFMessage`).

**Step 4: Determine the Direction**
The `direction` parameter tells the bridge which way the data should flow:
*   `GZ_TO_ROS`: Used for **Sensors, State, and Time** (e.g., `/clock`, Lidar, Camera, TF, Joint States). Gazebo simulates the environment (and generates the simulation time) and pushes that data out to ROS. (Note: `/clock` is a **must-have** for every project so ROS nodes sync with simulation time).
*   `ROS_TO_GZ`: Used for **Commands and Control** (e.g., `/cmd_vel` to drive the wheels, or a command to open a gripper). Your ROS nodes generate the command and send it to Gazebo so the physics engine can execute the physical movement.
*   `BIDIRECTIONAL`: Used when data needs to flow both ways (e.g., synchronized service interfaces or two-way topics).

**Step 5: Write the YAML Configuration**
Finally, combine all this information to write your bridge configuration YAML. *(Note: While you can set `ros_topic_name` to anything you want, always try to use global ROS 2 standard names—like `/clock`, `/tf`, `/scan`, or `/cmd_vel`—so that standard ROS tools like RViz and Nav2 work out-of-the-box! `/clock` in particular is mandatory for every project and is the only topic that requires absolutely zero plugin configuration in your URDF).*
```yaml
- ros_topic_name: "/tf"
  gz_topic_name: "/model/robot_2wd/tf"
  ros_type_name: "tf2_msgs/msg/TFMessage"
  gz_type_name: "gz.msgs.Pose_V"
  direction: GZ_TO_ROS
```