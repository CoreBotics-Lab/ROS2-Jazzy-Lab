![ROS2](https://img.shields.io/badge/ROS2-Jazzy-blue)
![Platform](https://img.shields.io/badge/Platform-TurtleBot_4-green.svg)
![Ubuntu](https://img.shields.io/badge/Ubuntu-24.04-orange?logo=ubuntu)

# 🐢 TurtleBot 4 Project Roadmap (2026 Edition)

> **How to use:** In VS Code, press `Ctrl+Shift+V` to see the preview. To check an item, add an `x` between the brackets: `[x]`.

This roadmap provides a structured set of projects for the TurtleBot 4, designed to build your skills from basic mobility to advanced autonomy and system integration.

---

## 🟢 LEVEL 1: CORE MOBILITY (The Foundation)
**Focus:** Mastering basic robot movement, sensing, and state management using core ROS 2 concepts.

- [ ] **Obstacle Avoider (LIDAR):** Drive forward autonomously until an object is detected by the LIDAR, then stop and turn.
    - **Logic:** Write a Python or C++ node that subscribes to `/scan` (`sensor_msgs/msg/LaserScan`). Find the minimum distance in the front-facing arc. If it's below a threshold, publish a `geometry_msgs/msg/Twist` message to `/cmd_vel` to turn the robot.
    - **Concepts:** Topics (Pub/Sub), ROS 2 CLI.

- [ ] **Battery Watchdog & Auto-Dock:** Prevent the robot from losing power by automatically sending it to its charging dock when the battery is low.
    - **Logic:** Monitor the `/battery_state` topic (`sensor_msgs/msg/BatteryState`). When the `percentage` field drops below a set value, call the `/dock` action server provided by the Create 3 base (`irobot_create_msgs/action/Dock`).
    - **Concepts:** Topics, Actions.

- [ ] **Teleop Customizer:** Drive the robot using a non-standard input, like a custom GUI, keyboard letters (WASD), or a web socket.
    - **Logic:** Create a program that captures input and translates it into `geometry_msgs/msg/Twist` messages, which are then published to `/cmd_vel`.
    - **Concepts:** Topics, Launch Files.

---

## 🟡 LEVEL 2: APPLIED PERCEPTION (Computer Vision)
**Focus:** Using the OAK-D camera and OpenCV to enable vision-based behaviors.

- [ ] **Line Follower:** Follow a high-contrast tape line on the floor.
    - **Logic:** Process the `/oakd/rgb/preview/image_raw` topic using OpenCV. Apply color thresholding and contour detection to find the line's centroid. Use a simple PID controller to calculate the necessary angular velocity to keep the centroid in the center of the image, publishing `Twist` commands.
    - **Concepts:** Image Processing (OpenCV), PID Control.

- [ ] **Lane Follower (Self-Driving Lite):** Stay centered between two parallel lines, simulating a road lane.
    - **Logic:** Apply a perspective transform (Bird's Eye View) to the camera image using OpenCV. Use the Hough Line Transform to detect both left and right lane boundaries. Calculate the lane's center and use a PID controller to generate `Twist` commands that keep the robot centered.
    - **Concepts:** Advanced Image Processing (Perspective Transform, Hough Lines), Control Theory.

- [ ] **AprilTag / ArUco Tracker:** Lock onto a physical AprilTag and maintain a fixed distance and orientation from it.
    - **Logic:** Use the `apriltag_ros` package to get the pose of a tag relative to the camera frame from the `/detections` topic. Use TF2 to transform this pose into the `base_link` frame and implement a control loop to drive the robot towards the tag's position.
    - **Concepts:** TF2 Transforms, Custom Interfaces.

- [ ] **Person Follower:** Act like a "pet" that follows a specific person around the room.
    - **Logic:** Use the OAK-D’s onboard AI capabilities (e.g., MobileNet-SSD via the `depthai_ros` package) to detect people. Identify the largest bounding box in the camera view and steer the robot to keep it centered.
    - **Concepts:** AI/ML Inference, Action Utilization.

---

## 🟠 LEVEL 3: AUTONOMOUS NAVIGATION (The Navigator)
**Focus:** Using the Nav2 stack for mapping, localization, and path planning.

- [ ] **Room Mapper (SLAM):** Create a high-fidelity 2D map of your environment.
    - **Logic:** Run `slam_toolbox` in synchronous mode (`ros2 launch slam_toolbox online_sync_launch.py`) while carefully driving the robot around the entire area using a teleop node. Save the resulting map using `ros2 run nav2_map_server map_saver_cli`.
    - **Concepts:** SLAM, `ros2_control`.

- [ ] **Multi-Goal Patrol:** Program the robot to move between a sequence of saved locations in a continuous loop.
    - **Logic:** Use the Nav2 Simple Commander API (`nav2_simple_commander`) in a Python script. Define a list of `PoseStamped` coordinates (e.g., "Kitchen," "Office," "Bedroom") and send them as a sequence of `goToPose` goals to the Nav2 action server.
    - **Concepts:** Nav2, Action Client.

- [ ] **"Mail Delivery" Service:** Create a service that allows you to command the robot to go to a named location.
    - **Logic:** Build a ROS 2 service where the request contains a string (e.g., "desk"). The server node looks up the pre-defined coordinates for "desk" and uses the Nav2 Commander API to navigate there.
    - **Concepts:** Services, Nav2.

---

## 🔴 LEVEL 4: ADVANCED BEHAVIORS (AI & Logic)
**Focus:** Integrating modern AI, advanced perception, and complex logic into your robot's behavior.

- [ ] **Visual SLAM (V-SLAM):** (Challenge) Create a map and navigate using *only* the OAK-D camera, without the 2D LIDAR.
    - **Logic:** Use a package like `rtabmap_ros` configured for stereo vision. The goal is to generate a 3D point cloud and 2D occupancy grid for localization and navigation, relying entirely on visual features.
    - **Concepts:** SLAM, 3D Perception, `rtabmap_ros`.

- [ ] **AI Traffic Rule Follower:** Navigate a mini-road environment, stopping at red lights or stop signs.
    - **Logic:** Train or use a pre-trained YOLOv8 model to detect traffic signs from the camera feed. Integrate this detector into Nav2 by writing a custom Behavior Tree node that can pause and resume navigation based on sign detection.
    - **Concepts:** Behavior Trees, Custom Nav2 Plugins, YOLO.

- [ ] **Gesture-Based Teleop:** Control the robot's movement using hand gestures.
    - **Logic:** Use the OAK-D camera and a gesture recognition model to detect hand poses (e.g., "open palm" for stop, "thumbs up" for go). Translate these gestures into `Twist` commands for `/cmd_vel`.
    - **Concepts:** Computer Vision, Lifecycle Nodes.

- [ ] **Docking with Visual Servoing:** (Challenge) Re-implement the docking procedure using only the camera to align with the charging station's AprilTag.
    - **Logic:** Once the robot is near the dock, switch to a vision-based control loop. Use the pose of the dock's AprilTag (from `apriltag_ros`) to calculate precise linear and angular velocities, guiding the robot onto the charging contacts without relying on the built-in dock action.
    - **Concepts:** Visual Servoing, Control Theory, TF2.

- [ ] **Dynamic Obstacle Avoidance with Depth Camera:** Integrate the OAK-D depth data into Nav2 to avoid obstacles that the 2D LIDAR cannot see.
    - **Logic:** Write a C++ node that processes the depth camera's point cloud. Create a custom Nav2 Costmap Plugin that takes this point cloud data and inflates obstacles directly into the local costmap, allowing the planner to react to dynamic objects in real-time.
    - **Concepts:** 3D Perception, Point Clouds, Custom Nav2 Plugins.

---

## 🏗️ LEVEL 5: SYSTEM INTEGRATION (The Architect)
**Focus:** Connecting your robot to external systems and building a complete, user-facing application.

- [ ] **Semantic Map Labeler:** Create a map that knows where objects like "chairs" and "tables" are located.
    - **Logic:** Combine SLAM with a YOLO object detector. As the robot maps an area, project the 3D position of detected objects into the map frame and save them as labeled markers or part of a custom data layer.
    - **Concepts:** TF2, Data Association.

- [ ] **Web Control Dashboard:** Create a web-based interface to monitor and control your TurtleBot 4 from any browser.
    - **Logic:** Use the `rosbridge_suite` to create a WebSocket connection to your ROS 2 graph. Build a simple web page with HTML/JavaScript that subscribes to the camera feed, battery state, and map, and allows you to send navigation goals by clicking on the map.
    - **Concepts:** `rosbridge_suite`, Web Development.

- [ ] **Multi-Robot Coordination (Simulation):** Simulate two TurtleBot 4s and have them map an area together without colliding.
    - **Logic:** Launch two namespaced robots in a single Gazebo world. Implement a simple coordination scheme, such as having each robot claim a different section of the map to explore.
    - **Concepts:** Namespacing, Gazebo Simulation, Zenoh (optional).

- [ ] **Voice-Activated Butler:** Command your robot to navigate to pre-defined locations using voice commands.
    - **Logic:** Use a Python library like `vosk` for offline Speech-to-Text (STT). Create a node that listens for a wake word (e.g., "Hey TurtleBot"). Once triggered, it parses a command like "go to the kitchen," looks up the corresponding coordinates, and uses the Nav2 Simple Commander API to send the goal.
    - **Concepts:** STT/NLU, System Integration, Nav2 API.

- [ ] **Security Patrol Robot:** Have the robot patrol a route and send an alert if it detects a person.
    - **Logic:** Combine the "Multi-Goal Patrol" logic with the "Person Follower" detector. If a person is detected while on patrol, the robot stops, saves a camera image, and uses a Python library like `smtplib` or `requests` to send an email or a Discord/Slack notification.
    - **Concepts:** State Machines, Behavior Trees, System Integration.