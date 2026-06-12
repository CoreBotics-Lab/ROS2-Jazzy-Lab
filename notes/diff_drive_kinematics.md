# Differential Drive Kinematics Node Documentation

This document provides a comprehensive overview of the `diff_drive_kinematics` ROS 2 node, including its configuration, interfaces, and the underlying mathematical theory.

## 1. Node Overview
The `diff_drive_kinematics` node acts as the mathematical bridge between high-level velocity commands and low-level hardware state. It performs two primary functions:
1. **Inverse Kinematics**: Translating standard Twist velocity commands into individual wheel speeds for the motor controllers.
2. **Forward Kinematics / Odometry**: Using wheel encoder positions to estimate the robot's movement and broadcasting its position via Odometry and TF messages.

### ROS 2 Interfaces

#### Parameters
- `wheel_radius` (double): The radius of the robot's wheels in meters.
- `wheel_separation` (double): The distance between the left and right wheels in meters.

#### Subscribed Topics
- `/cmd_vel` (`geometry_msgs/msg/Twist`): Target linear and angular velocity for the robot.
- `/joint_states` (`sensor_msgs/msg/JointState`): Current angular position of the left and right wheel joints.

#### Published Topics
- `/simple_velocity_controller/commands` (`std_msgs/msg/Float64MultiArray`): Target angular speeds for the left and right wheels.
- `/odom` (`nav_msgs/msg/Odometry`): The calculated odometry position and velocity of the robot.
- `/tf` (`tf2_msgs/msg/TFMessage`): The coordinate transform from the `odom` frame to the `base_footprint` frame.

---

## 2. Inverse Kinematics (`/cmd_vel` to Wheel Speeds)

The robot receives velocity commands on the `/cmd_vel` topic, representing the desired linear velocity ($v$) and angular velocity ($\omega$). However, the hardware controllers need to know how fast to spin the left and right wheels individually.

### Theory:
The linear velocity of the robot is the average of the two wheel velocities:
$$ v = \frac{v_{right} + v_{left}}{2} $$

The angular velocity is the difference between the wheel velocities divided by the wheel separation (the distance between the wheels, $L$):
$$ \omega = \frac{v_{right} - v_{left}}{L} $$

We can solve this system of equations for the individual wheel velocities ($v_{left}$ and $v_{right}$):
$$ v_{left} = v - \frac{\omega \cdot L}{2} $$
$$ v_{right} = v + \frac{\omega \cdot L}{2} $$

Since the motors are commanded in angular speed (radians per second), we divide the linear velocity of each wheel by the wheel radius ($r$):
$$ \text{Left Wheel Speed} = \frac{v - (\omega \cdot L / 2)}{r} $$
$$ \text{Right Wheel Speed} = \frac{v + (\omega \cdot L / 2)}{r} $$

**Implementation**: The `callback_cmd_vel` function calculates these values and publishes them to `/simple_velocity_controller/commands`.

---

## 3. Forward Kinematics & Odometry (Wheel Movement to Robot Position)

To track where the robot is in the world (its $x, y$ coordinates and heading $\theta$), the node reads the joint states (how far the wheels have rotated) and estimates the robot's movement.

### Theory:
First, we measure the change in the wheel positions from encoders and calculate the linear distance each wheel traveled:
$$ \Delta s_{left} = r \cdot \Delta \theta_{left} $$
$$ \Delta s_{right} = r \cdot \Delta \theta_{right} $$

From this, the robot's overall forward movement ($\Delta s$) and change in heading ($\Delta \theta$) over that small time step are:
$$ \Delta s = \frac{\Delta s_{right} + \Delta s_{left}}{2} $$
$$ \Delta \theta = \frac{\Delta s_{right} - \Delta s_{left}}{L} $$

### The Odometry Integration Problem
To update the robot's global coordinates ($x, y$), we use trigonometric functions to project the forward movement $\Delta s$ onto the $x$ and $y$ axes. 

#### Standard Euler Integration (The Trap)
If we use the robot's heading at the *start* of the movement ($\theta$), the math looks like this:
$$ x_{new} = x + \Delta s \cdot \cos(\theta) $$
$$ y_{new} = y + \Delta s \cdot \sin(\theta) $$
This assumes the robot drives in a straight line facing its old direction, then suddenly snaps to its new angle at the end. This ignores the fact that the robot was turning *while* it was moving, creating a jagged, zig-zag path that constantly overshoots the true curved path.

#### Mid-Angle Integration (The Fix)
Instead of assuming the robot faced the start angle ($\theta$) or the end angle ($\theta + \Delta \theta$) for the whole step, we use the heading exactly halfway through the movement:
$$ \theta_{mid} = \theta + \frac{\Delta \theta}{2} $$

By plugging this middle angle into the trigonometry functions, we calculate the straight chord that connects the start of the arc to the end of the arc:
$$ x_{new} = x + \Delta s \cdot \cos\left(\theta_{mid}\right) $$
$$ y_{new} = y + \Delta s \cdot \sin\left(\theta_{mid}\right) $$

This method ensures the robot's estimated position stays perfectly centered on its actual curved path.

#### Angle Normalization (The Maintenance Step)
Because $\theta$ continuously accumulates as the robot spins, it must be wrapped between $-\pi$ and $+\pi$ before broadcasting to ensure external ROS tools (like Nav2) don't experience coordinate flipping.
$$ \theta_{normalized} = \text{atan2}(\sin(\theta), \cos(\theta)) $$

**Implementation**: 
1. The `callback_joint_states` function performs these steps, utilizing the Mid-Angle integration method to update the robot's `x`, `y`, and `theta`. 
2. It calculates the robot's new linear and angular velocity from the space deltas.
3. It passes these values to `publish_odom_tf`, which uses the `tf_transformations.quaternion_from_euler` library to cleanly convert the 2D $\theta$ into a 3D quaternion.
4. It constructs and publishes an `Odometry` message to `/odom` and a `TransformStamped` message to the `/tf` tree linking the `odom` frame to `base_footprint`.

---

## 4. Matrix Method (Industry Standard Kinematics)

While the algebraic equations above are easy to read, production-level robotics heavily relies on linear algebra for speed and scalability (utilizing libraries like `Eigen` in C++ and `numpy` in Python). By formulating the kinematics as matrix multiplications, the compiler can use SIMD (Single Instruction, Multiple Data) processor instructions to calculate both wheels simultaneously.

### Inverse Kinematics (Matrix)
First, we establish the base mathematical relationship that defines the robot velocity ($V_b$ from `twist.linear.x`, $\Omega_b$ from `twist.angular.z`) as a function of the linear wheel velocities ($v_L, v_R$), utilizing the physical wheel separation ($L$) and wheel radius ($r$). This is our core Kinematics Matrix ($M$):

$$ \begin{bmatrix} V_b \\ \Omega_b \end{bmatrix} = \underbrace{\begin{bmatrix} 1/2 & 1/2 \\ -1/L & 1/L \end{bmatrix}}_{M} \begin{bmatrix} v_L \\ v_R \end{bmatrix} $$

Because our goal in Inverse Kinematics is to find the required wheel speeds for a given target robot velocity, we isolate the wheel velocities by multiplying both sides by the inverse of matrix $M$ ($M^{-1}$):

$$ \begin{bmatrix} v_L \\ v_R \end{bmatrix} = \begin{bmatrix} 1/2 & 1/2 \\ -1/L & 1/L \end{bmatrix}^{-1} \begin{bmatrix} V_b \\ \Omega_b \end{bmatrix} $$

Once the linear wheel velocities are found, we divide by the wheel radius ($r$) to get the angular wheel velocities ($\dot{\phi}_L, \dot{\phi}_R$) for the motor controllers:

$$ \begin{bmatrix} \dot{\phi}_L \\ \dot{\phi}_R \end{bmatrix} = \frac{1}{r} \begin{bmatrix} v_L \\ v_R \end{bmatrix} $$

**Implementation**: The `compute_wheel_velocities` function utilizes a pre-inverted matrix (`M_inv_`) at startup, converting runtime calculation into a single, highly optimized matrix multiplication step.

### Forward Kinematics (Jacobian Matrix)
To find the global movement of the robot ($\dot{x}, \dot{y}, \dot{\theta}$) from the angular speeds of the wheels ($\dot{\phi}_L, \dot{\phi}_R$), we first define a 3x2 Forward Jacobian matrix ($J$) that incorporates the robot's current heading ($\theta$):

$$ J = \begin{bmatrix} \frac{r}{2} \cos(\theta) & \frac{r}{2} \cos(\theta) \\ \frac{r}{2} \sin(\theta) & \frac{r}{2} \sin(\theta) \\ -\frac{r}{L} & \frac{r}{L} \end{bmatrix} $$

We then multiply this Jacobian matrix by the input wheel velocities to calculate the instantaneous velocity of the robot in the global frame:

$$ \begin{bmatrix} \dot{x} \\ \dot{y} \\ \dot{\theta} \end{bmatrix} = J \begin{bmatrix} \dot{\phi}_L \\ \dot{\phi}_R \end{bmatrix} $$

Expanded out, this represents the final mathematical operation our node performs:

$$ \begin{bmatrix} \dot{x} \\ \dot{y} \\ \dot{\theta} \end{bmatrix} = \begin{bmatrix} \frac{r}{2} \cos(\theta) & \frac{r}{2} \cos(\theta) \\ \frac{r}{2} \sin(\theta) & \frac{r}{2} \sin(\theta) \\ -\frac{r}{L} & \frac{r}{L} \end{bmatrix} \begin{bmatrix} \dot{\phi}_L \\ \dot{\phi}_R \end{bmatrix} $$

**Implementation**: The `forward_kinematics` function dynamically builds this Jacobian matrix using the robot's current heading and multiplies it by the wheel speeds to compute the precise rate of change for the robot position. These resulting velocities are then multiplied by the time step ($dt$) to securely update the robot's global Odometry tracking.
