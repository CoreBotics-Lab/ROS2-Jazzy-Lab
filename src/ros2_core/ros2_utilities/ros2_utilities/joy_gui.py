#!/usr/bin/env python3
"""
ROS 2 Virtual Joystick GUI (PyQt6)

Features:
- Mouse Teleoperation: Click and drag the blue puck to control linear and angular velocity.
- Keyboard Teleoperation (WASD): Use W, A, S, D for directional control. Automatically normalizes diagonal inputs.
- Emergency Stop (Spacebar): Instantly halts the robot (0.0 velocity) and overrides all other inputs.
- Dynamic Topic Re-mapping: Change the target topic (e.g., 'turtle1/cmd_vel') at runtime via the UI or ROS 2 parameters.
- Dynamic Speed Limits: Adjust maximum linear and angular speeds on the fly via UI spinboxes or ROS 2 parameters.
- Event-Driven Parameters: Full two-way synchronization between the GUI and the ROS 2 Parameter Server.
- Adjustable Publish Rate: Change the 'publish_rate_hz' parameter at runtime to adjust the cmd_vel publish frequency.
- Settings Panel: Auto-hiding configuration menu for advanced driving modes.
- Axis Inversion: Instantly invert Linear or Angular outputs for testing mismatched motor wiring.
- Always on Top: Pin the joystick window above your simulator and terminals.

Controls: w↑ s↓ a← d→ wd↗ ds↘ sa↙ aw↖  (space: stop)
"""
import sys
import math
import signal
import rclpy
from rclpy.node import Node
from rclpy.logging import get_logger
from geometry_msgs.msg import Twist
from rclpy.parameter import Parameter
from rcl_interfaces.msg import SetParametersResult

from PyQt6.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QHBoxLayout, QWidget, QLabel, QLineEdit, QPushButton, QDoubleSpinBox, QCheckBox, QFrame, QSizePolicy
from PyQt6.QtCore import Qt, pyqtSignal, QPointF, QTimer, QEvent
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
        self.e_stop_active = False

    def paintEvent(self, event):  # type: ignore
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        
        # Draw the background boundary ring
        painter.setPen(QPen(QColor(100, 100, 100), 3))
        painter.setBrush(QBrush(QColor(220, 220, 220)))
        painter.drawEllipse(self.center, self.max_distance, self.max_distance)
        
        # Draw the draggable puck
        painter.setPen(QPen(QColor(20, 20, 20), 2))
        if self.e_stop_active:
            painter.setBrush(QBrush(QColor(255, 50, 50)))  # Bright Red for E-Stop!
        else:
            painter.setBrush(QBrush(QColor(50, 150, 255))) # Default Blue
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
        
    def set_e_stop(self, active):
        if self.e_stop_active != active:
            self.e_stop_active = active
            self.update()

class JoyNode(Node):
    def __init__(self):
        super().__init__('joy_gui_node')
        self.topic_name = 'cmd_vel'
        
        # Declare and get the initial topic name from parameters
        self.declare_parameter('topic_name', 'cmd_vel')
        self.topic_name = str(self.get_parameter('topic_name').value)
        
        self.publisher_ = self.create_publisher(Twist, self.topic_name, 10)
        self.twist_msg = Twist()
        
        # Advanced Driving Toggles
        self.invert_linear = False
        self.invert_angular = False
        
        # Adjust these to set the max speeds of your robot!
        self.declare_parameter('max_linear', 2.0)
        self.max_linear = float(self.get_parameter('max_linear').value)  # type: ignore
        
        self.declare_parameter('max_angular', 2.0)
        self.max_angular = float(self.get_parameter('max_angular').value)  # type: ignore
        
        # Speed Profiles
        self.declare_parameter('turtle_mode_speed', 0.5)
        self.turtle_mode_speed = float(self.get_parameter('turtle_mode_speed').value)  # type: ignore
        
        self.declare_parameter('rabbit_mode_speed', 3.0)
        self.rabbit_mode_speed = float(self.get_parameter('rabbit_mode_speed').value)  # type: ignore
        
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
            elif param.name == 'turtle_mode_speed' and param.type_ == Parameter.Type.DOUBLE:
                self.turtle_mode_speed = param.value
            elif param.name == 'rabbit_mode_speed' and param.type_ == Parameter.Type.DOUBLE:
                self.rabbit_mode_speed = param.value
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
        linear_mult = -1.0 if self.invert_linear else 1.0
        angular_mult = -1.0 if self.invert_angular else 1.0
        
        self.twist_msg.linear.x = norm_y * self.max_linear * linear_mult
        self.twist_msg.angular.z = -norm_x * self.max_angular * angular_mult

    def publish_twist(self):
        self.publisher_.publish(self.twist_msg)


class MainWindow(QMainWindow):
    def __init__(self, node):
        super().__init__()
        self.node = node
        self.setWindowTitle("ROS 2 PyQt6 Joystick")
        
        # Set window to be always on top by default!
        self.setWindowFlag(Qt.WindowType.WindowStaysOnTopHint, True)
        
        # Enable keyboard focus for WASD controls
        self.setFocusPolicy(Qt.FocusPolicy.StrongFocus)

        main_widget = QWidget()
        layout = QVBoxLayout(main_widget)
        layout.setContentsMargins(5, 5, 5, 5)

        # --- Topic Input Row ---
        topic_layout = QHBoxLayout()
        topic_label = QLabel("Topic:")
        self.topic_input = QLineEdit(self.node.topic_name)
        update_btn = QPushButton("Update")
        update_btn.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        update_btn.clicked.connect(self.on_topic_update)
        topic_layout.addWidget(topic_label)
        topic_layout.addWidget(self.topic_input)
        topic_layout.addWidget(update_btn)
        
        self.settings_btn = QPushButton("⚙️ Settings")
        self.settings_btn.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.settings_btn.clicked.connect(self.toggle_settings)
        topic_layout.addWidget(self.settings_btn)
        layout.addLayout(topic_layout)
        
        # --- Auto-Hiding Settings Panel (Detached Window) ---
        self.settings_panel = QFrame(self, Qt.WindowType.Tool | Qt.WindowType.FramelessWindowHint | Qt.WindowType.WindowStaysOnTopHint)
        self.settings_panel.setStyleSheet("QFrame { border: 1px solid #ccc; border-radius: 4px; background-color: #f9f9f9; }")
        settings_layout = QVBoxLayout()
        settings_layout.setContentsMargins(4, 4, 4, 4)
        
        self.inv_lin_cb = QCheckBox("Inv Lin")
        self.inv_ang_cb = QCheckBox("Inv Ang")
        self.ontop_cb = QCheckBox("Always on Top")
        self.ontop_cb.setChecked(True)
        
        for cb in [self.inv_lin_cb, self.inv_ang_cb, self.ontop_cb]:
            cb.clicked.connect(self.on_settings_interacted)
            settings_layout.addWidget(cb)
        settings_layout.addStretch() # Push checkboxes to the top
            
        self.inv_lin_cb.clicked.connect(self.on_inv_lin_changed)
        self.inv_ang_cb.clicked.connect(self.on_inv_ang_changed)
        self.ontop_cb.clicked.connect(self.on_ontop_changed)
        
        self.settings_panel.setLayout(settings_layout)
        self.settings_panel.setVisible(False)
        
        # --- Speed Limits Row ---
        speed_layout = QHBoxLayout()
        
        linear_label = QLabel("Max Linear:")
        self.linear_input = QDoubleSpinBox()
        self.linear_input.setRange(0.0, 20.0)
        self.linear_input.setSingleStep(0.1)
        self.linear_input.setValue(self.node.max_linear)
        self.linear_input.setKeyboardTracking(False)
        self.linear_input.installEventFilter(self)
        self.linear_input.valueChanged.connect(self.on_speed_update)
        
        angular_label = QLabel("Max Angular:")
        self.angular_input = QDoubleSpinBox()
        self.angular_input.setRange(0.0, 20.0)
        self.angular_input.setSingleStep(0.1)
        self.angular_input.setValue(self.node.max_angular)
        self.angular_input.setKeyboardTracking(False)
        self.angular_input.installEventFilter(self)
        self.angular_input.valueChanged.connect(self.on_speed_update)
        
        speed_layout.addWidget(linear_label)
        speed_layout.addWidget(self.linear_input)
        speed_layout.addWidget(angular_label)
        speed_layout.addWidget(self.angular_input)
        
        self.turtle_btn = QPushButton("🐢")
        self.turtle_btn.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.turtle_btn.setStyleSheet("font-size: 20px; padding: 4px;")
        self.turtle_btn.clicked.connect(self.set_turtle_mode)
        self.rabbit_btn = QPushButton("🐇")
        self.rabbit_btn.setFocusPolicy(Qt.FocusPolicy.NoFocus)
        self.rabbit_btn.setStyleSheet("font-size: 20px; padding: 4px;")
        self.rabbit_btn.clicked.connect(self.set_rabbit_mode)
        
        speed_layout.addWidget(self.turtle_btn)
        speed_layout.addWidget(self.rabbit_btn)
        layout.addLayout(speed_layout)

        self.label = QLabel("Linear X: 0.00 | Angular Z: 0.00")
        self.label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(self.label)

        self.joystick = JoystickWidget()
        self.joystick.joystickMoved.connect(self.on_joystick_moved)
        layout.addWidget(self.joystick)

        # --- Controls Hint ---
        controls_label = QLabel("Controls: w↑ s↓ a← d→ wd↗ ds↘ sa↙ aw↖  (space: stop)")
        controls_label.setAlignment(Qt.AlignmentFlag.AlignLeft)
        controls_label.setStyleSheet("color: gray; font-size: 11px;")
        layout.addWidget(controls_label)
        
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
        self.current_x = 0.0
        self.current_y = 0.0
        
        # Ensure the main window has focus initially, not the text input boxes
        self.setFocus()

        # Settings Auto-Hide Timer
        self.settings_timer = QTimer()
        self.settings_timer.setSingleShot(True)
        self.settings_timer.timeout.connect(self.hide_settings)

    def on_joystick_moved(self, x, y):
        if Qt.Key.Key_Space in self.keys_pressed:
            x, y = 0.0, 0.0
            self.joystick.blockSignals(True)
            self.joystick.set_normalized_position(0.0, 0.0)
            self.joystick.blockSignals(False)
        self.current_x = x
        self.current_y = y
        self.node.update_twist(x, y)
        self.label.setText(f"Linear X: {self.node.twist_msg.linear.x:.2f} | Angular Z: {self.node.twist_msg.angular.z:.2f}")
        
    def on_topic_update(self):
        new_topic = self.topic_input.text().strip()
        # Make the ROS 2 Parameter Server the Single Source of Truth!
        self.node.set_parameters([Parameter('topic_name', Parameter.Type.STRING, new_topic)])
        
        # Return keyboard focus to the main window so WASD and Spacebar E-Stop work immediately
        self.setFocus()
        
    def update_topic_input(self, new_topic):
        self.topic_input.setText(new_topic)
        
    def eventFilter(self, source, event):  # type: ignore
        # Intercept WASD and Spacebar from the spinboxes so you can drive while changing speeds!
        if event.type() == QEvent.Type.KeyPress:
            if event.key() in (Qt.Key.Key_W, Qt.Key.Key_A, Qt.Key.Key_S, Qt.Key.Key_D, Qt.Key.Key_Space):
                self.keyPressEvent(event)
                return True
        elif event.type() == QEvent.Type.KeyRelease:
            if event.key() in (Qt.Key.Key_W, Qt.Key.Key_A, Qt.Key.Key_S, Qt.Key.Key_D, Qt.Key.Key_Space):
                self.keyReleaseEvent(event)
                return True
        return super().eventFilter(source, event)

    def on_speed_update(self):
        # Update the ROS 2 Parameter Server instead of the variables directly
        self.node.set_parameters([
            Parameter('max_linear', Parameter.Type.DOUBLE, self.linear_input.value()),
            Parameter('max_angular', Parameter.Type.DOUBLE, self.angular_input.value())
        ])
        
    def update_linear_input(self, new_val):
        self.linear_input.setValue(new_val)
        self.on_joystick_moved(self.current_x, self.current_y)
        
    def update_angular_input(self, new_val):
        self.angular_input.setValue(new_val)
        self.on_joystick_moved(self.current_x, self.current_y)
        
    def toggle_settings(self):
        if self.settings_panel.isVisible():
            self.hide_settings()
        else:
            geo = self.geometry()
            self.settings_panel.move(geo.right() + 5, geo.top())
            self.settings_panel.show()
            self.settings_timer.start(3000)
            self.setFocus()
            
    def hide_settings(self):
        if not self.settings_panel.isVisible():
            return
            
        self.settings_panel.hide()
        self.settings_timer.stop()
        self.setFocus()
        
    def on_settings_interacted(self):
        self.settings_timer.start(3000)
        self.setFocus()
        
    def on_inv_lin_changed(self):
        self.node.invert_linear = self.inv_lin_cb.isChecked()
        self.on_joystick_moved(self.current_x, self.current_y)
        
    def on_inv_ang_changed(self):
        self.node.invert_angular = self.inv_ang_cb.isChecked()
        self.on_joystick_moved(self.current_x, self.current_y)
        
    def on_ontop_changed(self):
        self.setWindowFlag(Qt.WindowType.WindowStaysOnTopHint, self.ontop_cb.isChecked())
        self.show() # Window flags require a re-show to take effect
        
        self.settings_panel.setWindowFlag(Qt.WindowType.WindowStaysOnTopHint, self.ontop_cb.isChecked())
        if self.settings_panel.isVisible():
            self.settings_panel.show()
            
        self.setFocus()
        
    def set_turtle_mode(self):
        self.node.set_parameters([Parameter('max_linear', Parameter.Type.DOUBLE, self.node.turtle_mode_speed), Parameter('max_angular', Parameter.Type.DOUBLE, self.node.turtle_mode_speed)])
        self.setFocus()
        
    def set_rabbit_mode(self):
        self.node.set_parameters([Parameter('max_linear', Parameter.Type.DOUBLE, self.node.rabbit_mode_speed), Parameter('max_angular', Parameter.Type.DOUBLE, self.node.rabbit_mode_speed)])
        self.setFocus()

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
        if Qt.Key.Key_Space in self.keys_pressed:
            self.joystick.set_e_stop(True)
        else:
            self.joystick.set_e_stop(False)
            if Qt.Key.Key_W in self.keys_pressed: y += 1.0
            if Qt.Key.Key_S in self.keys_pressed: y -= 1.0
            if Qt.Key.Key_A in self.keys_pressed: x -= 1.0
            if Qt.Key.Key_D in self.keys_pressed: x += 1.0

            # Normalize the vector so diagonal movement doesn't exceed the max radius
            magnitude = math.sqrt(x**2 + y**2)
            if magnitude > 0.0:
                x, y = x / magnitude, y / magnitude

        self.joystick.set_normalized_position(x, y)
        
    def mousePressEvent(self, event):  # type: ignore
        if self.settings_panel.isVisible():
            self.hide_settings()
        super().mousePressEvent(event)

    def moveEvent(self, event):  # type: ignore
        super().moveEvent(event)
        if self.settings_panel.isVisible():
            geo = self.geometry()
            self.settings_panel.move(geo.right() + 5, geo.top())

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