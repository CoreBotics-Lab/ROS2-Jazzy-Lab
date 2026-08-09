# Interface Generator — `generate.py`

Generates cross-platform **MsgPack-serialized** interface files from ROS 2-style `.msg` and `.srv`
definitions for three targets simultaneously:

| Target | Output location | Include / Import |
|--------|----------------|-----------------|
| **MCU C++** | `shared_libraries/mcu/zenoh_ros/zenoh_ros/msg_interface/custom_interface/` | `#include <zenoh_ros/custom_msgs/z_MyMsg.h>` |
| **PC C++** | `shared_libraries/cpp/zenoh_ros/zenoh_ros/msg_interface/custom_interface/` | `#include <zenoh_ros/custom_msgs/z_MyMsg.h>` |
| **Python** | `shared_libraries/python/zenoh_ros/msg_interface/custom_interface/` | `from zenoh_ros.custom_msgs import z_MyMsg` |

> Forwarding headers are automatically created at the old flat paths so existing user code never needs to change.

---

## Usage

Run from `shared_libraries/msg_interface/`:

```bash
# Generate
python3 generate_interface/generate.py -generate msg/custom_msgs/MyMsg.msg
python3 generate_interface/generate.py -generate srv/custom_srvs/MySrv.srv

# Remove generated files
python3 generate_interface/generate.py -rm msg/custom_msgs/MyMsg.msg
```

---

## What It Can Do

### 1. Custom `.msg` — Flat Primitive Fields

Simple message with only primitive types.

```
# msg/custom_msgs/MyRobot.msg
string name
int32  id
float32 speed
bool   active
```

**Supported primitive types:**

| `.msg` type | C++ type | Python type |
|-------------|----------|-------------|
| `int8` / `uint8` | `int8_t` / `uint8_t` | `int` |
| `int16` / `uint16` | `int16_t` / `uint16_t` | `int` |
| `int32` / `uint32` | `int32_t` / `uint32_t` | `int` |
| `int64` / `uint64` | `int64_t` / `uint64_t` | `int` |
| `float32` | `float` | `float` |
| `float64` | `double` | `float` |
| `string` / `String` | `std::string` | `str` |
| `bool` | `bool` | `bool` |

---

### 2. Custom `.msg` — Nested Pre-defined Types

Embed any supported ROS 2 standard type directly as a field. The generator **automatically** inserts
the correct `#include` / `import` — no extra includes needed in your firmware or node code.

```
# msg/custom_msgs/RobotOdom.msg
std_msgs/Header        header
nav_msgs/Odometry      odom
float32                battery_voltage
```

**Supported pre-defined nested types:**

| `.msg` field type | Resolves to (C++) | Resolves to (Python) |
|---|---|---|
| `geometry_msgs/Vector3` | `z_geometry_msgs::z_Vector3` | `z_geometry_msgs.z_Vector3` |
| `geometry_msgs/Point` | `z_geometry_msgs::z_Point` | `z_geometry_msgs.z_Point` |
| `geometry_msgs/Quaternion` | `z_geometry_msgs::z_Quaternion` | `z_geometry_msgs.z_Quaternion` |
| `geometry_msgs/Twist` | `z_geometry_msgs::z_Twist` | `z_geometry_msgs.z_Twist` |
| `geometry_msgs/Pose` | `z_geometry_msgs::z_Pose` | `z_geometry_msgs.z_Pose` |
| `builtin_interfaces/Time` | `builtin_interfaces::z_Time` | `z_builtin_interfaces.z_Time` |
| `builtin_interfaces/Duration` | `builtin_interfaces::z_Duration` | `z_builtin_interfaces.z_Duration` |
| `std_msgs/Header` | `z_std_msgs::z_Header` | `z_std_msgs.z_Header` |
| `std_msgs/ColorRGBA` | `z_std_msgs::z_ColorRGBA` | `z_std_msgs.z_ColorRGBA` |
| `sensor_msgs/Imu` | `z_sensor_msgs::z_Imu` | `z_sensor_msgs.z_Imu` |
| `sensor_msgs/Temperature` | `z_sensor_msgs::z_Temperature` | `z_sensor_msgs.z_Temperature` |
| `sensor_msgs/Range` | `z_sensor_msgs::z_Range` | `z_sensor_msgs.z_Range` |
| `nav_msgs/Odometry` | `z_nav_msgs::z_Odometry` | `z_nav_msgs.z_Odometry` |

> Both the short form (`geometry_msgs/Vector3`) and the full ROS 2 form
> (`geometry_msgs/msg/Vector3`) are accepted.

---

### 3. Custom `.msg` — Nested Custom Types (Same Package)

Reference another custom message you have already defined, by its bare name (no package prefix needed):

```
# msg/custom_msgs/LedRGB.msg
int32 r
int32 g
int32 b

# msg/custom_msgs/SetLED.msg
LedRGB rgb          # references another custom msg
int32  brightness
```

The generator resolves `LedRGB` → `custom_msgs::z_LedRGB` in C++ and imports it correctly in Python.

---

### 4. Custom `.srv` — Simple Request / Response

Use `---` as the divider between request and response fields:

```
# srv/custom_srvs/SetColor.srv
int32 r
int32 g
int32 b
---
bool   success
string message
```

Generates a service class with:
- `Request`  — serializable struct/class with `serialize()` / `deserialize()`
- `Response` — serializable struct/class with `serialize()` / `deserialize()`
- Correct Zenoh queryable/get wiring in the library

---

### 5. Custom `.srv` — Nested Custom Types in Request / Response

The generator correctly handles custom nested types inside service definitions:

```
# srv/custom_srvs/SetLEDColor.srv
SetLED led_data       # nested custom msg in request
---
string result
```

- In C++: nested object serialized as a JSON sub-object (MsgPack map)
- In Python: nested object serialized as a `dict` (not double-encoded bytes)

---

### 6. `rm` — Remove Generated Files

Deletes **both** the canonical file under `msg_interface/custom_interface/` **and** the
forwarding header at the flat path, for all three targets:

```bash
python3 generate_interface/generate.py -rm msg/custom_msgs/MyMsg.msg
```

---

## Directory Layout After Generation

```
shared_libraries/
├── msg_interface/
│   ├── msg/custom_msgs/MyMsg.msg          ← source definition (edit this)
│   ├── srv/custom_srvs/MySrv.srv          ← source definition (edit this)
│   └── generate_interface/generate.py     ← the generator
│
├── mcu/zenoh_ros/zenoh_ros/
│   ├── custom_msgs/z_MyMsg.h              ← forwarding header (user includes this)
│   └── msg_interface/custom_interface/msgs/custom_msgs/z_MyMsg.h   ← canonical
│
├── cpp/zenoh_ros/zenoh_ros/
│   ├── custom_msgs/z_MyMsg.h              ← forwarding header (user includes this)
│   └── msg_interface/custom_interface/msgs/custom_msgs/z_MyMsg.h   ← canonical
│
└── python/zenoh_ros/
    ├── custom_msgs/__init__.py            ← re-exports (user imports from here)
    └── msg_interface/custom_interface/msgs/custom_msgs/MyMsg.py     ← canonical
```

---

## Current Limitations

| Feature | Status |
|---------|--------|
| Array / list fields (`int32[] data`) | ❌ Not supported |
| Pre-defined nested types inside `.srv` fields | ❌ Not handled |
| `uint8[]` byte arrays | ❌ Not in type map |
| Auto-updating `__init__.py` exports on generation | ❌ Manual step required |
| Cross-package custom nested types (`my_pkg/MyType`) | ⚠️ Partial — falls back to `pkg::z_Name` derive |
