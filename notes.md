# 📝 ROS 2 C++ Memory Management Notes

## 1. The Publisher (The Creator)
* **Concept:** The node "owns" the data creation.
* **Technique:** Use `std::make_shared<MessageType>()` in the **Constructor**.
* **Why:** We want to allocate a "memory bucket" on the heap exactly once.
* **Usage:** In the timer callback, we simply update the data inside that bucket and publish it. This avoids the overhead of creating/deleting memory every 500ms.
* **Keyword:** `this->msg` (The pointer is a member of the class).

## 2. The Subscriber (The Receiver)
* **Concept:** The node "borrows" the data from the middleware.
* **Technique:** Use `const MessageType::SharedPtr msg` as a **Function Argument**.
* **Why:** ROS 2 handles the allocation when the message arrives. It hands us a "Smart Pointer" that automatically cleans itself up when the callback finishes.
* **Usage:** Access the data directly using the argument name (e.g., `msg->data`). 
* **Mistake to Avoid:** Do **not** use `this->` in the subscriber callback for the incoming message.

---

## 🔍 Deep Dive: Why `this->` is forbidden in the Subscriber Callback

Understanding the `this->` keyword is fundamental to mastering C++ Scope.

### What `this->` actually means
In C++, `this` is a pointer to the **current instance** of the class. When you write `this->variable`, you are telling the compiler:
> "Look inside the private/public members of this specific object."

### The Publisher Scenario (Inside the "Pocket")
In your Publisher, you declared `String::SharedPtr msg` in the `private` section of the class. 
* The variable **belongs** to the class.
* It lives as long as the Node lives.
* You use `this->msg` because the variable is literally "inside the pocket" of the class instance.

### The Subscriber Scenario (The "Hand-off")
In the Subscriber, the `msg` variable is **not** declared in your class. Instead, it is passed into the `callback_subscriber(const String::SharedPtr msg)` function as an **argument**.
* **Scope:** This `msg` only exists inside the brackets of that function.
* **Origin:** It was created by the ROS 2 middleware (DDS) and handed to the function.
* **The Error:** If you try to use `this->msg`, the compiler looks inside the class members for a variable named `msg`. Since you didn't declare one there (and you shouldn't), the code fails.

**Summary:** * Use `this->` for **Class Members** (Variables that stay with the node).
* Do **NOT** use `this->` for **Function Arguments** (Data that is just passing through the function).

---

# 📡 ROS 2 Quality of Service (QoS) Deep Dive

QoS is the "contract" between a Publisher and a Subscriber. It defines how data is handled by the middleware (DDS). If the contracts are incompatible, the nodes will not communicate.

## 1. The Three "Big Knobs" (Policies)

| Policy | Option A: Reliable (Default) | Option B: Best Effort (Sensor) |
| :--- | :--- | :--- |
| **Reliability** | **Reliable:** Guarantees delivery. It will re-send lost packets. | **Best Effort:** Fire and forget. It won't wait for confirmation. |
| **Durability** | **Transient Local:** "Late-joiners" get the last sent message. | **Volatile:** "Late-joiners" only see messages sent *after* they joined. |
| **History** | **Keep Last (X):** Keeps only the $X$ most recent messages. | **Keep All:** Keeps everything until the buffer is full (Risky). |

---

## 2. QoS Profile Comparison Table (Pre-configured Presets)

ROS 2 provides "Presets"—pre-set configurations for the most common tasks:

| Preset | Reliability | Durability | Best Use Case |
| :--- | :--- | :--- | :--- |
| `SensorDataQoS()` | **Best Effort** | Volatile | High-frequency data where losing a packet is okay. (LIDAR, IMU, Cameras). |
| `ServicesQoS()` | **Reliable** | Volatile | One-time requests where losing a packet is NOT okay. (Open gripper, reset motor). |
| `ParametersQoS()` | **Reliable** | **Transient Local** | Configuration values that nodes must remember. |
| `SystemDefaultsQoS()` | Reliable* | Volatile | Standard topics (Default if only an integer is used). |

---

## ⚙️ QoS Configuration: Presets vs. Custom
You have two ways to define a "Contract" in both C++ and Python:

1. **Pre-configured Presets:** Best for 90% of use cases. 
   * *Example:* `rclcpp::SensorDataQoS()` (C++) or `qos_profile_sensor_data` (Python).
   * *Pro:* Guaranteed to use industry-standard settings for that data type.

2. **Custom QoS Profiles:** Use this when you need to "mix and match" policies.
   * *Example:* If you need **Best Effort** reliability but want to **Keep Last (100)** messages instead of the default 5.
   * *Logic:* You create a QoS object, set the history depth, and then manually toggle the reliability or durability settings.

> **Note:** Whether using a preset or a custom profile, the **Compatibility Rule** still applies: the Subscriber's requirements must be met by the Publisher's offerings.


## 💡 Architectural Strategy

### When to use Best Effort (`SensorDataQoS`)
Use this for high-frequency streams. In robotics, **"old data is bad data."** If a packet is lost, it is better to skip it and move to the newest data rather than lagging the system to re-send a stale packet.

### When to use Reliable (`ServicesQoS` / `Default`)
Use this for commands. If you send a "Stop" command to a motor, you cannot afford to lose that packet. The system must ensure it arrives, even if it takes extra time to confirm delivery.

### 🤝 Compatibility Rule
QoS is a negotiation. The Subscriber "requests" a level of service, and the Publisher "offers" one. If the offer is lower than the request, the connection fails.

| Publisher Offer | Subscriber Request | Connection Status |
| :--- | :--- | :--- |
| **Best Effort** | **Best Effort** | ✅ **Works** |
| **Reliable** | **Best Effort** | ✅ **Works** (Sub gets better than it asked for) |
| **Best Effort** | **Reliable** | ❌ **Fails** (Sub demands more than Pub gives) |

> **Tip:** If your nodes are running but `ros2 topic echo` shows nothing, always check for a **Reliability** mismatch first!

## 🛠️ Topic Debugging & Utilities
* **Check Frequency:** `ros2 topic hz /topic_name`
* **Check Bandwidth:** `ros2 topic bw /topic_name`
* **Remapping (Terminal):** `ros2 run pkg node --ros-args -r /old:=/new`
* **Remapping (Launch):** Done via the `remappings` argument in a Launch file.

## 📊 Performance Monitoring
* **Frequency (Hz):** How many times per second data is sent. 
    * *Rule:* High frequency = Better reactivity, higher CPU usage.
    * *Command:* `ros2 topic hz <topic>`
* **Bandwidth (BW):** How much total data is being pushed through the network.
    * *Rule:* High bandwidth = More network stress (crucial for Cameras/LIDAR).
    * *Command:* `ros2 topic bw <topic>`

