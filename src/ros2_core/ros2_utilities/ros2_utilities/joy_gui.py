#!/usr/bin/env python3
import sys
import math
import signal
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from geometry_msgs.msg import Twist
from rclpy.parameter import Parameter
from rcl_interfaces.msg import SetParametersResult

from PyQt6.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QHBoxLayout, QWidget, QLabel, QLineEdit, QPushButton, QDoubleSpinBox
from PyQt6.QtCore import Qt, pyqtSignal, QPointF, QTimer
from PyQt6.QtGui import QPainter, QColor, QPen, QBrush

class JoystickWidget(QWidget):
    """
    A custom Qt Widget that draws a joystick and emits normalized (x, y) 
    coordinates when dragged.
    """
    joystickMoved = pyqtSignal(float, float)  # Emits (norm_x, norm_y)

    def __init__(self, parent=None):
        super().__init__(parent)
        self.setMinimumSize(250, 250)
        self.max_distance = 100
        self.puck_pos = QPointF(125, 125)
        self.center = QPointF(125, 125)
        self.pressed = False

    def paintEvent(self, event):  # type: ignore
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        
        # Draw the background boundary ring
        painter.setPen(QPen(QColor(100, 100, 100), 3))
        painter.setBrush(QBrush(QColor(220, 220, 220)))
        painter.drawEllipse(self.center, self.max_distance, self.max_distance)
        
        # Draw the draggable puck
        painter.setPen(QPen(QColor(20, 20, 20), 2))
        painter.setBrush(QBrush(QColor(50, 150, 255)))
        painter.drawEllipse(self.puck_pos, 25, 25)

    def resizeEvent(self, event):  # type: ignore
        # Re-center the joystick if the window gets resized
        self.center = QPointF(self.width() / 2, self.height() / 2)
        self.puck_pos = self.center
        self.max_distance = min(self.width(), self.height()) / 2 - 30
        super().resizeEvent(event)

    def mousePressEvent(self, event):  # type: ignore
        self.pressed = True
        self.update_puck(event.position())

    def mouseMoveEvent(self, event):  # type: ignore
        if self.pressed:
            self.update_puck(event.position())

    def mouseReleaseEvent(self, event):  # type: ignore
        # Snap back to center when let go!
        self.pressed = False
        self.puck_pos = self.center
        self.update()
        self.joystickMoved.emit(0.0, 0.0)

    def update_puck(self, pos):
        # Calculate distance from center
        dx = pos.x() - self.center.x()
        dy = pos.y() - self.center.y()
        distance = math.sqrt(dx**2 + dy**2)

        # Clamp the puck to the outer ring if dragged too far
        if distance > self.max_distance:
            dx = dx * self.max_distance / distance
            dy = dy * self.max_distance / distance

        self.puck_pos = QPointF(self.center.x() + dx, self.center.y() + dy)
        self.update()

        # Normalize values to a range of [-1.0, 1.0]
        norm_x = dx / self.max_distance
        norm_y = -dy / self.max_distance  # Invert Y so "up" on screen is positive
        self.joystickMoved.emit(norm_x, norm_y)

    def set_normalized_position(self, norm_x, norm_y):
        """Programmatically move the puck (used by keyboard WASD)."""
        self.pressed = False  # Override mouse drag if keyboard takes over
        dx = norm_x * self.max_distance
        dy = -norm_y * self.max_distance
        self.puck_pos = QPointF(self.center.x() + dx, self.center.y() + dy)
        self.update()
        self.joystickMoved.emit(norm_x, norm_y)

class JoyNode(Node):
    def __init__(self):
        super().__init__('joy_gui_node')
        self.topic_name = 'cmd_vel'
        
        # Declare and get the initial topic name from parameters
        self.declare_parameter('topic_name', 'cmd_vel')
        self.topic_name = str(self.get_parameter('topic_name').value)
        
        self.publisher_ = self.create_publisher(Twist, self.topic_name, 10)
        self.twist_msg = Twist()
        
        # Adjust these to set the max speeds of your robot!
        self.declare_parameter('max_linear', 2.0)
        self.max_linear = float(self.get_parameter('max_linear').value)  # type: ignore
        
        self.declare_parameter('max_angular', 2.0)
        self.max_angular = float(self.get_parameter('max_angular').value)  # type: ignore
        
        # Frequency for publishing twist messages
        self.declare_parameter('publish_rate_hz', 10.0)
        self.publish_rate_hz = float(self.get_parameter('publish_rate_hz').value)  # type: ignore
        
        # Callbacks for GUI updates
        self.gui_update_topic_cb = None
        self.gui_update_linear_cb = None
        self.gui_update_angular_cb = None
        
        # Event-driven parameter handling
        self.add_on_set_parameters_callback(self.parameter_callback)
        
        # Real robots expect a continuous stream of velocity commands
        timer_period = 1.0 / self.publish_rate_hz if self.publish_rate_hz > 0 else 0.1
        self.timer = self.create_timer(timer_period, self.publish_twist)

    def parameter_callback(self, params: list[Parameter]):
        for param in params:
            if param.name == 'topic_name' and param.type_ == Parameter.Type.STRING:
                self.change_topic(param.value)
            elif param.name == 'max_linear' and param.type_ == Parameter.Type.DOUBLE:
                self.max_linear = param.value
                if self.gui_update_linear_cb is not None:
                    self.gui_update_linear_cb(self.max_linear)
            elif param.name == 'max_angular' and param.type_ == Parameter.Type.DOUBLE:
                self.max_angular = param.value
                if self.gui_update_angular_cb is not None:
                    self.gui_update_angular_cb(self.max_angular)
            elif param.name == 'publish_rate_hz' and param.type_ == Parameter.Type.DOUBLE:
                new_rate = float(param.value)  # type: ignore
                if new_rate > 0.0:
                    self.publish_rate_hz = new_rate
                    self.timer.cancel()
                    self.timer = self.create_timer(1.0 / self.publish_rate_hz, self.publish_twist)
                    self.get_logger().info(f"Publish rate changed dynamically to: {self.publish_rate_hz} Hz")
        return SetParametersResult(successful=True)

    def change_topic(self, new_topic):
        if new_topic and new_topic != self.topic_name:
            self.destroy_publisher(self.publisher_)
            self.topic_name = new_topic
            self.publisher_ = self.create_publisher(Twist, self.topic_name, 10)
            self.get_logger().info(f"Changed publisher topic to: {self.topic_name}")
            if self.gui_update_topic_cb is not None:
                self.gui_update_topic_cb(self.topic_name)

    def update_twist(self, norm_x, norm_y):
        # norm_y (up/down) controls Linear X (forward/backward)
        # norm_x (left/right) controls Angular Z. Left is positive rotation, so we invert X.
        self.twist_msg.linear.x = norm_y * self.max_linear
        self.twist_msg.angular.z = -norm_x * self.max_angular

    def publish_twist(self):
        self.publisher_.publish(self.twist_msg)


class MainWindow(QMainWindow):
    def __init__(self, node):
        super().__init__()
        self.node = node
        self.setWindowTitle("ROS 2 PyQt6 Joystick")
        self.resize(300, 400)
        
        # Enable keyboard focus for WASD controls
        self.setFocusPolicy(Qt.FocusPolicy.StrongFocus)

        main_widget = QWidget()
        layout = QVBoxLayout()

        # --- Topic Input Row ---
        topic_layout = QHBoxLayout()
        topic_label = QLabel("Topic:")
        self.topic_input = QLineEdit(self.node.topic_name)
        update_btn = QPushButton("Update")
        update_btn.clicked.connect(self.on_topic_update)
        topic_layout.addWidget(topic_label)
        topic_layout.addWidget(self.topic_input)
        topic_layout.addWidget(update_btn)
        layout.addLayout(topic_layout)
        
        # --- Speed Limits Row ---
        speed_layout = QHBoxLayout()
        
        linear_label = QLabel("Max Linear:")
        self.linear_input = QDoubleSpinBox()
        self.linear_input.setRange(0.0, 20.0)
        self.linear_input.setSingleStep(0.1)
        self.linear_input.setValue(self.node.max_linear)
        self.linear_input.valueChanged.connect(self.on_speed_update)
        
        angular_label = QLabel("Max Angular:")
        self.angular_input = QDoubleSpinBox()
        self.angular_input.setRange(0.0, 20.0)
        self.angular_input.setSingleStep(0.1)
        self.angular_input.setValue(self.node.max_angular)
        self.angular_input.valueChanged.connect(self.on_speed_update)
        
        speed_layout.addWidget(linear_label)
        speed_layout.addWidget(self.linear_input)
        speed_layout.addWidget(angular_label)
        speed_layout.addWidget(self.angular_input)
        layout.addLayout(speed_layout)

        self.label = QLabel("Linear X: 0.00 | Angular Z: 0.00")
        self.label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(self.label)

        self.joystick = JoystickWidget()
        self.joystick.joystickMoved.connect(self.on_joystick_moved)
        layout.addWidget(self.joystick)

        main_widget.setLayout(layout)
        self.setCentralWidget(main_widget)

        # Connect the node's GUI update callbacks
        self.node.gui_update_topic_cb = self.update_topic_input
        self.node.gui_update_linear_cb = self.update_linear_input
        self.node.gui_update_angular_cb = self.update_angular_input

        # Integrate ROS 2 spin loop into PyQt6's event loop
        self.ros_timer = QTimer()
        self.ros_timer.timeout.connect(lambda: rclpy.spin_once(self.node, timeout_sec=0))
        self.ros_timer.start(10)  # Spin at 100Hz
        
        # Track currently pressed keys for WASD movement
        self.keys_pressed = set()
        
        # Ensure the main window has focus initially, not the text input boxes
        self.setFocus()

    def on_joystick_moved(self, x, y):
        self.node.update_twist(x, y)
        self.label.setText(f"Linear X: {self.node.twist_msg.linear.x:.2f} | Angular Z: {self.node.twist_msg.angular.z:.2f}")
        
    def on_topic_update(self):
        new_topic = self.topic_input.text().strip()
        # Make the ROS 2 Parameter Server the Single Source of Truth!
        self.node.set_parameters([Parameter('topic_name', Parameter.Type.STRING, new_topic)])
        
    def update_topic_input(self, new_topic):
        self.topic_input.setText(new_topic)
        
    def on_speed_update(self):
        # Update the ROS 2 Parameter Server instead of the variables directly
        self.node.set_parameters([
            Parameter('max_linear', Parameter.Type.DOUBLE, self.linear_input.value()),
            Parameter('max_angular', Parameter.Type.DOUBLE, self.angular_input.value())
        ])
        
    def update_linear_input(self, new_val):
        self.linear_input.setValue(new_val)
        
    def update_angular_input(self, new_val):
        self.angular_input.setValue(new_val)

    def keyPressEvent(self, event):  # type: ignore
        # Ignore auto-repeat events (when you hold a key down)
        if event.isAutoRepeat():
            return
        key = event.key()
        if key in (Qt.Key.Key_W, Qt.Key.Key_A, Qt.Key.Key_S, Qt.Key.Key_D, Qt.Key.Key_Space):
            self.keys_pressed.add(key)
            self.update_joystick_from_keys()
        else:
            super().keyPressEvent(event)

    def keyReleaseEvent(self, event):  # type: ignore
        if event.isAutoRepeat():
            return
        key = event.key()
        if key in (Qt.Key.Key_W, Qt.Key.Key_A, Qt.Key.Key_S, Qt.Key.Key_D, Qt.Key.Key_Space):
            self.keys_pressed.discard(key)
            self.update_joystick_from_keys()
        else:
            super().keyReleaseEvent(event)

    def update_joystick_from_keys(self):
        x, y = 0.0, 0.0
        
        # Spacebar acts as an Emergency Stop (Priority)
        if Qt.Key.Key_Space not in self.keys_pressed:
            if Qt.Key.Key_W in self.keys_pressed: y += 1.0
            if Qt.Key.Key_S in self.keys_pressed: y -= 1.0
            if Qt.Key.Key_A in self.keys_pressed: x -= 1.0
            if Qt.Key.Key_D in self.keys_pressed: x += 1.0

            # Normalize the vector so diagonal movement doesn't exceed the max radius
            magnitude = math.sqrt(x**2 + y**2)
            if magnitude > 0.0:
                x, y = x / magnitude, y / magnitude

        self.joystick.set_normalized_position(x, y)

def main(args=None) -> None:
    log = get_logger("System")
    node_instance = None
    exit_code = 0

    try:
        log.info("Initializing the ROS2 Client...")
        rclpy.init(args=args)

        log.info("Starting a ROS2 Node...")
        node_instance = JoyNode()
        
        app = QApplication(sys.argv)
        window = MainWindow(node_instance)
        window.show()
        
        # Setup graceful shutdown for Ctrl+C inside the PyQt event loop
        def handle_sigint(signum, frame):
            log.warn("[CTRL+C]>>> Interrupted by the User.")
            app.quit()
        signal.signal(signal.SIGINT, handle_sigint)

        exit_code = app.exec()

    except KeyboardInterrupt:
        log.warn("[CTRL+C]>>> Interrupted by the User.")

    except Exception as e:
        log.error(f"Critical Error: {e}")
    
    finally:
        if node_instance is not None:
            log.info("Destroying the ROS2 Node...")
            node_instance.destroy_node()
            node_instance = None

        if rclpy.ok():
            log.info("Manually shutting down the ROS2 Client...")
            rclpy.shutdown()
            
        sys.exit(exit_code)

if __name__ == '__main__':
    main()