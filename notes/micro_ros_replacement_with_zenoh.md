# Zenoh to ROS 2 Architecture Suggestion

This note outlines an alternative architecture to micro-ROS for bridging an MCU (ESP32) to a PC running ROS 2.

## The Core Concept
Instead of running a heavy DDS middleware layer (micro-ROS) directly on the microcontroller, this architecture uses **Eclipse Zenoh**, a high-performance, edge-to-cloud data routing protocol built in Rust. It provides the publisher/subscriber modularity of ROS 2 but with significantly less overhead on the embedded side.

### The Stack
- **At the Edge (ESP32):** Runs `zenoh-pico`, an ultra-lean pure-C client library. The ESP32 simply reads sensor data (IMU, LiDAR) and publishes it via Zenoh URI paths (e.g., `robot/esp32/imu`) over Wi-Fi or Serial.
- **At the Core (PC):** Runs a Python gateway script using the official `eclipse-zenoh` library. This script acts as a translator.
- **The Translation:** The Python gateway subscribes to Zenoh topics, decodes the packets, maps them into standard ROS 2 `.msg` payloads (like `sensor_msgs/msg/Imu`), and natively publishes them to the ROS 2 graph via `rclpy`. It does the reverse for motor commands (`cmd_vel` to Zenoh paths).

## Why Zenoh over micro-ROS?
- **Lightweight MCU Footprint:** Avoids running an entire operating system network stack on the ESP32.
- **Zero Custom Mapping Code:** Unlike raw ZeroMQ + Protobuf, Zenoh paths map perfectly to ROS 2 topic concepts out of the box.
- **Automatic Discovery:** `zenoh-pico` broadcasts its presence on boot and automatically pairs with the PC. No hardcoded IPs required.
- **Wildcard Subscriptions:** Zenoh allows subscribing to `robot/esp32/*` in Python to easily capture multiple sensor streams in a single callback.
- **Code Isolation:** The ROS 2 stack on the PC can be refactored or restarted without bricking the ESP32 connection. The embedded firmware engineers don't even need to know ROS 2.

## Security Blueprint for Production
If moving from a lab environment to a public or production environment, the architecture can be secured using a defense-in-depth approach:

### 1. Wrapper Level: Schema Enforcement via MessagePack
Raw Python dictionaries (JSON) lack strict structure and are vulnerable to injection attacks. 
Swap JSON for **MessagePack (`msgpack`)** payload serialization inside the Zenoh pipe. Msgpack is binary and fast, and the Python gateway can strictly validate the schema before passing the data to the ROS 2 DDS.

### 2. Zenoh Level: Mutual TLS (mTLS)
Enable Zenoh's built-in TLS and authentication. Both the PC and ESP32 use cryptographic certificates to authenticate the connection. This encrypts the data payloads (protecting against packet sniffing) and rejects unauthorized nodes.

### 3. Network Level: WireGuard Tunnel (Optional but Recommended)
To prevent network scanners from seeing open device ports and attempting DDoS attacks, use a **WireGuard VPN**. 
*Implementation:* Instead of taxing the ESP32, attach a small hardware travel router to the robot's chassis. The router acts as the WireGuard client, creating an invisible, encrypted virtual tunnel between the robot's subnet and the processing PC.

---

## Zenoh mTLS vs. WireGuard: Who Does What?

Strictly speaking, WireGuard is not mathematically or computationally necessary if you have already configured Zenoh with full mTLS (Mutual Transport Layer Security). Both layers encrypt your data, so running both can feel like putting a padlock on a safe. However, in professional robotics deployments, engineers often use them together because they protect against entirely different kinds of threats.

### 🛡️ What Zenoh mTLS Solves (The Data Guard / The Key to the House)
When you enable TLS inside zenoh-pico and your Python wrapper, your data payloads are fully encrypted using modern cryptography (AES or ChaCha20).
- **The Protection:** If a hacker intercepts your Wi-Fi traffic, they cannot read your LiDAR scans or spoof your `cmd_vel` motor speeds. The application layer is secure. It forces mutual authentication; only devices with the cryptographic keys can connect.
- **The Loophole:** Your underlying device ports are still visible to the local network. Anyone running a network scanner (like `nmap`) can see that your PC and your robot have open network ports waiting for connections. This exposes you to DDoS (Denial of Service) attacks.

### 🧱 What WireGuard Solves (The Cloaking Device)
WireGuard doesn't care about your robotics data; it secures the entire operating system network interface.
- **The Protection:** It creates a private, invisible virtual tunnel between your PC and your robot's subnet. To an external attacker, your robot and PC don't even exist on the Wi-Fi. Their IP addresses and ports are completely hidden inside the tunnel.
- **The Security Benefit:** It completely eliminates the risk of network port scanning, brute-force connection attempts, and local network intrusion.

### 📊 The Trade-Offs

| Configuration | Security Level | Compute Overhead on MCU | Setup Complexity |
| :--- | :--- | :--- | :--- |
| **Zenoh mTLS Only** | Good (Data is safe, but ports are exposed) | Medium (MCU must process crypto handshakes) | Low |
| **WireGuard Only** | Good (Network is hidden, but data is cleartext inside it) | Zero (If handled by an external hardware router) | Medium |
| **Both Together** | Excellent (Defense-in-depth industry standard) | Medium | High |

### 🎨 The Ultimate Analogy

Think of your robot system like a top-secret government compound:

| Layer | The Real-World Analogy | What it Actually Does |
| :--- | :--- | :--- |
| **WireGuard** | **Camouflage & Fog:** The entire compound is painted in perfect camouflage and hidden from all maps. Attackers don't even know where to shoot or look. | Hides your network ports and device IP addresses from scanners. |
| **Zenoh mTLS** | **Armed Guard at the Door:** Even if someone stumbles into the compound by accident, they cannot enter any building without an un-forgeable security badge. | Ensures only authorized devices can connect and encrypts the stream. |
| **MessagePack** | **Secure Briefcase:** The data inside the building isn't written on loose, messy papers (like JSON text). It's packed in a dense, standardized briefcase. | Structured binary data format that prevents injection or crashing bugs. |

## 🏗️ The Production-Ready Stack (Visualized)

Here is exactly how a secure, production-grade payload moves through your system:

```text
[ Your ESP32 C++ Code ]
   │
   ▼ 1. PACK (Application Layer)
   │   Converts sensor readings into an ultra-compact MessagePack binary payload.
   │   Safeguards against corrupted data formats.
   │
[ Zenoh-Pico Stack ]
   │
   ▼ 2. LOCK (Transport Layer)
   │   Applies mTLS encryption using unique digital certificates.
   │   Ensures only your specific devices can read or write to topics.
   │
[ Hardware Travel Router (Onboard Robot) ]
   │
   ▼ 3. CLOAK (Network Layer)
   │   Wraps the entire Wi-Fi payload inside a WireGuard VPN tunnel.
   │   Hides the robot's IP addresses completely from external network scanners.
   │
   ====================== OVER THE AIR (WI-FI) ======================
   │
[ Host Processing PC ]
   │
   ▼ 4. DECRYPT, VERIFY & TRANSLATE
   │   WireGuard decrypts network -> Zenoh checks certificates -> Python wrapper
   │   validates the MessagePack schema and maps it directly into ROS 2 topics.
```

## 🎯 Your Strategy Going Forward (The Engineering Advantage)

**1. During Development & Lab Prototyping: Skip WireGuard**
Stick to Zenoh + MessagePack while developing at your desk. If you are testing inside a closed, private lab Wi-Fi network that you completely control, WireGuard is overkill. This keeps your workspace clean, ultra-fast, and easy to debug without dealing with heavy security certificates or network routing rules while focusing on your core robotics math or logic.

**2. Moving to Production or Public Environments: Add WireGuard and mTLS**
When you are ready to take your multi-robot systems out into the real world or deploy them in shared public spaces, WireGuard becomes highly recommended. Simply activate the mTLS config flags in Zenoh and power on the WireGuard router mounted on the chassis. You gain a fortress of security—a defense-in-depth model that rivals top-tier autonomous vehicle architectures—without having to change, rewrite, or compile a single line of your core robotics math or logic!

## 🔄 How it Compares to Native ROS 2

At the end of the day, combining Zenoh + MessagePack + mTLS essentially reconstructs the core capabilities of ROS 2, but it is custom-built to be lightweight enough for a microcontroller.

If we look at what ROS 2 actually is under the hood, it provides three main things, and our custom stack perfectly mimics them:

### 1. The Transport Layer (Pub/Sub & Discovery)
- **ROS 2 uses:** Heavy DDS (Data Distribution Service).
- **Our Stack uses:** Zenoh (Much faster, uses significantly less RAM, and works over Wi-Fi without crashing the ESP32).

### 2. The Data Structure Layer
- **ROS 2 uses:** `.msg` files and CDR serialization.
- **Our Stack uses:** MessagePack (Achieves the exact same thing—turning variables into structured, compressed binary arrays—but is easier to implement in raw C++). 
  *Note: The MCU packs the data using a C++ MessagePack library, and the Python PC gateway unpacks the data using the Python MessagePack library (and vice versa for sending commands back to the MCU).*

### 3. The Security Layer
- **ROS 2 uses:** SROS2 (Secure ROS 2, which wraps DDS in encryption).
- **Our Stack uses:** Zenoh mTLS + WireGuard (Achieves the exact same military-grade encryption, but pushes the heavy lifting to the hardware router so the MCU doesn't lag).

### Why go through the trouble of building this?
You might wonder: *"If it equals ROS 2, why not just use micro-ROS?"*

Because standard DDS (which micro-ROS relies on) was designed for massive enterprise servers, not tiny microcontrollers. Forcing an ESP32 to act like a full ROS 2 node often leads to out-of-memory errors, dropped Wi-Fi packets, and massive headaches when trying to tune QoS (Quality of Service) settings.

By building this custom stack, you get all the benefits of ROS 2 on your PC, while keeping your microcontroller fast, clean, and isolated. Your ESP32 just blasts raw MessagePack bytes over Zenoh, and your Python PC gateway acts as the "translation layer" that officially turns it into ROS 2 for the rest of your system!
