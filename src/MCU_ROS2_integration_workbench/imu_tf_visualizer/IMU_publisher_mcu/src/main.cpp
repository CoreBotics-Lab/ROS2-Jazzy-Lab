/*
 * ====================================================================
 * ESP32-S3 ↔ MPU6050 (GY-87 / HW-290 Breakout) Wiring Diagram
 * ====================================================================
 *  MPU / GY-87 Pin | ESP32-S3 Pin | Function / Description
 * -----------------+--------------+-----------------------------------
 *  VCC_IN / VCC    | 5V or 3.3V   | Main Power Supply (5V or 3.3V)
 *  3.3V            | (Unconnected)| Direct 3.3V Output (Leave NC)
 *  GND             | GND          | Common Ground
 *  SCL             | GPIO 10      | I2C Clock Line
 *  SDA             | GPIO 9       | I2C Data Line
 *  INTA            | GPIO 18      | MPU6050 Hardware Interrupt Pin
 * ====================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>

#include <zenoh_ros/ZenohRos.h>
#include <zenoh_ros/sensor_msgs/z_Imu.h>
#include <zenoh_ros/z_logger.h>

// --- Pin Definitions for ESP32-S3 ---
#define I2C_SDA_PIN  9
#define I2C_SCL_PIN  10
#define MPU_INT_PIN  18

// --- Configuration Struct (AP Mode) ---
ZenohConfig cfg = ZenohConfig()
    .set_wifi("ESP32S3_Zenoh_AP", "zenoh1234", WIFI_AP)
    .set_port(7447);

// Sensor Globals
MPU6050 mpu(0x68);
MPU6050 mpu_alt(0x69);
bool mpu_use_alt = false;
bool mpu_detected = false;

class MPU6050_Publisher_Node : public ZenohNode {
public:
    MPU6050_Publisher_Node() : ZenohNode("mpu6050_publisher") {
        ZLOG_INFO(this->get_logger(), "Node %s has been started", this->z_get_name());
        
        // 1. Create a typed publisher with standard ROS 2 z_Imu message interface
        publisher_ = this->z_create_publisher<z_Imu>("robot/mpu6050", 10);

        // 2. Create timer (triggers callback_timer every 50ms / 20Hz)
        timer_ = this->z_create_timer(50, [this]() -> void {
            this->callback_timer();
        });
    }

private:
    ZenohPublisher<z_Imu>* publisher_ = nullptr;
    ZenohTimer* timer_ = nullptr;
    z_Imu msg_;

    void callback_timer() {
        if (!this->publisher_ || !mpu_detected) return;

        int16_t ax = 0, ay = 0, az = 0;
        int16_t gx = 0, gy = 0, gz = 0;

        if (mpu_use_alt) {
            mpu_alt.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        } else {
            mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        }

        // Standard ROS 2 Header with timestamp
        ZenohTime now_time = this->now();
        msg_.header.stamp.sec = now_time.sec;
        msg_.header.stamp.nanosec = now_time.nanosec;
        msg_.header.frame_id = "imu_link";

        // Convert raw values to standard SI units:
        // Linear Acceleration (m/s²)
        msg_.linear_acceleration.x = (ax / 16384.0) * 9.80665;
        msg_.linear_acceleration.y = (ay / 16384.0) * 9.80665;
        msg_.linear_acceleration.z = (az / 16384.0) * 9.80665;

        // Angular Velocity (rad/s)
        msg_.angular_velocity.x = (gx / 131.0) * (3.141592653589793 / 180.0);
        msg_.angular_velocity.y = (gy / 131.0) * (3.141592653589793 / 180.0);
        msg_.angular_velocity.z = (gz / 131.0) * (3.141592653589793 / 180.0);

        // Identity Orientation Quaternion (0,0,0,1)
        msg_.orientation.x = 0.0;
        msg_.orientation.y = 0.0;
        msg_.orientation.z = 0.0;
        msg_.orientation.w = 1.0;

        // Publish over Zenoh
        this->publisher_->publish(msg_);

        ZLOG_INFO_THROTTLE(this->get_logger(), 1000, "Accel: (%6.2f, %6.2f, %6.2f) m/s² | Gyro: (%6.2f, %6.2f, %6.2f) rad/s",
                           msg_.linear_acceleration.x, msg_.linear_acceleration.y, msg_.linear_acceleration.z,
                           msg_.angular_velocity.x, msg_.angular_velocity.y, msg_.angular_velocity.z);
    }
};

MPU6050_Publisher_Node* node_instance = nullptr;

void init_i2c() {
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.setClock(400000);
    ZLOG_INFO(z_get_logger("hardware"), "Wire initialized on SDA: GPIO %d, SCL: GPIO %d", I2C_SDA_PIN, I2C_SCL_PIN);
}

void init_mpu() {
    mpu.initialize();

    if (!mpu.testConnection()) {
        ZLOG_WARN(z_get_logger("mpu6050"), "Address 0x68 failed. Trying alternate address 0x69...");
        mpu_alt.initialize();
        if (mpu_alt.testConnection()) {
            mpu_use_alt = true;
            mpu_detected = true;
            ZLOG_INFO(z_get_logger("mpu6050"), "Connected successfully at address 0x69!");
            return;
        }
    } else {
        mpu_detected = true;
        ZLOG_INFO(z_get_logger("mpu6050"), "Connected successfully at address 0x68!");
        return;
    }

    mpu_detected = false;
    ZLOG_ERROR(z_get_logger("mpu6050"), "ERROR: MPU6050 testConnection failed!");
}

void setup() {
    Serial.begin(115200);
    z_delay(2000);

    ZLOG_INFO(z_get_logger("system"), "==========================================");
    ZLOG_INFO(z_get_logger("system"), "  ESP32-S3 MPU6050 Zenoh SoftAP Publisher");
    ZLOG_INFO(z_get_logger("system"), "==========================================");

    init_i2c();
    init_mpu();

    // 1. Initialize global Zenoh client context
    ZLOG_INFO(z_get_logger("system"), "Initializing the Zenoh Client...");
    if (ZenohNode::init(cfg)) {
        // 2. Spin up Node instance
        ZLOG_INFO(z_get_logger("system"), "Starting the Zenoh Node...");
        node_instance = new MPU6050_Publisher_Node();
    } else {
        ZLOG_ERROR(z_get_logger("system"), "CRITICAL Error: Zenoh Client initialization failed!");
        while (1) { z_delay(1000); }
    }
}

void loop() {
    // Spin the node to keep it alive
    if (node_instance) {
        node_instance->z_spin();
    }
}
