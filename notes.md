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

---

# 🤝 ROS 2 Services: Request/Response Mechanism

The core concept of ROS 2 Services is a synchronous request/response communication pattern. This means:

*   **The Client Sends a Request:** The client node initiates the communication by sending a `Request` message to a specific service. It then typically waits for a response.
*   **The Server Receives the Request:** The server node, which has advertised the service, receives this `Request` message.
*   **The Server Processes and Sends a Response:** The server performs some computation or action based on the `Request`. Once it's done, it constructs a `Response` message and sends it back to the client.
*   **The Client Receives the Response:** The client, which has been waiting, receives the `Response` message and can then continue its execution.

This synchronous nature is key. Unlike topics, where data flows continuously and asynchronously, a service call is a one-time interaction where the client expects an immediate reply before proceeding with its next service call. This makes services ideal for tasks like triggering an action, querying a specific piece of information, or performing a calculation where the result is needed right away.

Think of it like making a phone call:
*   **Client:** You dial a number (send a request).
*   **Server:** The person on the other end answers (receives the request).
*   **Server:** They listen to your question and formulate an answer (process and send a response).
*   **Client:** You hear their answer (receive the response).

---

# ROS2 Action

Actions are the ROS 2 communication pattern for long-running, cancelable tasks. Unlike a service, which is a quick, blocking request/response, an action allows a client to track the progress of a task and optionally cancel it before completion.

Under the hood, an action is a clever combination of services and topics:
*   **Goal Service:** The client sends a request containing the goal details. The server sends an immediate response to either `ACCEPT` or `REJECT` the goal.
*   **Cancel Service:** The client sends a request to cancel a running goal. The server sends a response confirming whether the cancellation process has been initiated.
*   **Result Service:** The client sends a request to get the final result. The server holds this request until the task is complete, then sends a response containing the final result.
*   **Feedback Topic:** The server publishes progress updates, and the client subscribes to these updates.
*   **Status Topic:** The server publishes the status of all active goals, which clients can subscribe to for system monitoring.

## The Client-Server Interaction Flow (The "Two Futures" Model)

The process is a sequence of two main asynchronous calls, each returning its own "future" (a ticket for a result you'll get later).

1.  **Stage 1: The Goal Handshake (The First Future)**
    *   **Client Sends Goal:** The client calls `send_goal_async()`. This is a non-blocking service call that immediately returns a `_send_goal_future`.
    *   **Server Responds:** The server's `goal_callback` runs and returns `ACCEPT` or `REJECT`.
    *   **Client Receives Confirmation:** When the server responds, the `_send_goal_future` is completed. This triggers the client's `goal_response_callback`. Inside this callback, the client receives a `goal_handle` if the goal was accepted.

2.  **Stage 2: The Result Request (The Second Future)**
    *   **Server Executes:** As soon as the goal is accepted, the server's `execute_callback` begins running in the background.
    *   **Client Requests Result:** After receiving the `goal_handle`, the client code must **manually** call `get_result_async()` on that handle. This is a *second, separate* service call that asks the server, "Please notify me when you have the final result." This call immediately returns a `_get_result_future`. The client now holds a ticket for the final result.
    *   **Server Finishes and Responds:** The `execute_callback` eventually finishes and `return`s a `Counter.Result()` object. This return value is the **response** to the result service call.
    *   **Client Receives Result:** The `_get_result_future` is now completed, which triggers the client's `get_result_callback`. The client can now access the final result and status.

## The Cooperative Cancellation Model

Cancellation is an independent process that can interrupt Stage 2 of the interaction flow. The ROS 2 system does **not** forcefully terminate the server's execution. Instead, it provides a signal, and it is the server's responsibility to "cooperate" by checking for this signal and shutting itself down gracefully.

This process is a multi-step handshake:

1.  **The Request (Client):** The client sends a cancel request (e.g., by pressing `Ctrl+C` in the terminal or calling `cancel_goal_async()` in code).

2.  **The Gatekeeper (`cancel_callback`):**
    *   The ROS 2 system receives the request and calls your `cancel_callback` function.
    *   Its ONLY job is to be a gatekeeper. It must immediately return `CancelResponse.ACCEPT` or `CancelResponse.REJECT`.
    *   Its task is to approve the interruption request. By returning `ACCEPT`, it allows the system to set the `is_cancel_requested` flag.

3.  **The Signal (`is_cancel_requested`):**
    *   If (and only if) the `cancel_callback` returns `ACCEPT`, the ROS 2 action framework **automatically sets the `goal_handle.is_cancel_requested` flag to `True`**.
    *   This flag is the official signal that your execution code needs to stop. You do not set this flag manually.

4.  **The Check (Your `execute_callback`):**
    *   Inside your long-running process (e.g., a `for` or `while` loop), you must periodically check the state of this flag:
        ```python
        if goal_handle.is_cancel_requested:
            # Time to stop...
        ```

5.  **The Termination (Your `execute_callback`):**
    *   Once your code sees the flag is `True`, it must perform two actions:
    *   **Set Final Status:** Call `goal_handle.canceled()`. This function does **not** exit your code. Its only job is to set the goal's final state to `CANCELED` and inform the client.
    *   **Exit the Function:** Use the standard Python `return` keyword to immediately exit the `execute_callback` function. This is what actually stops the process.

## Critical Insight: Single-Threaded vs. Multi-Threaded Execution

The Cooperative Cancellation model has a major dependency on how your node processes callbacks. For cancellation to work reliably with blocking code, a multi-threaded executor is required.

### The Single-Threaded Problem
*   When you use `rclpy.spin(node)`, you are using a `SingleThreadedExecutor`. This provides **one single thread** to do all of the node's work.
*   Consider this code in your `execute_callback`: `time.sleep(1.0)`.
*   This `time.sleep()` call **blocks the entire thread**.
*   If a cancel request arrives while the thread is sleeping, there are no other threads available to process it. The `cancel_callback` is never called, the `is_cancel_requested` flag is never set, and the goal continues to run as if nothing happened.

### The Multi-Threaded Solution
*   By using a `MultiThreadedExecutor`, you provide your node with a pool of worker threads.
    ```python
    executor = MultiThreadedExecutor()
    executor.add_node(node)
    executor.spin()
    ```
*   Now, one thread can be blocked by `time.sleep()` in the `execute_callback`.
*   When a cancel request arrives, a **different, free thread** from the pool can wake up, run the `cancel_callback`, and set the `is_cancel_requested` flag.
*   On the next iteration of the loop, when the first thread wakes up from its sleep, it will see the flag is `True` and terminate correctly.

> **Note:** If your node hosts an Action or a Service that might have a long-running or blocking callback, always use a `MultiThreadedExecutor` to ensure the node remains responsive.
