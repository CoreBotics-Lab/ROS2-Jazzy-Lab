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
