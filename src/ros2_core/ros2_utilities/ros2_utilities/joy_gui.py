#!/usr/bin/env python3
import sys
import math
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist

from PyQt6.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QHBoxLayout, QWidget, QLabel, QLineEdit, QPushButton
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


class JoyNode(Node):
    def __init__(self):
        super().__init__('joy_gui_node')
        self.topic_name = 'cmd_vel'
        self.publisher_ = self.create_publisher(Twist, self.topic_name, 10)
        self.twist_msg = Twist()
        
        # Adjust these to set the max speeds of your robot!
        self.max_linear = 2.0
        self.max_angular = 2.0
        
        # Real robots expect a continuous stream of velocity commands (usually 10Hz)
        self.timer = self.create_timer(0.1, self.publish_twist)

    def change_topic(self, new_topic):
        if new_topic and new_topic != self.topic_name:
            self.destroy_publisher(self.publisher_)
            self.topic_name = new_topic
            self.publisher_ = self.create_publisher(Twist, self.topic_name, 10)
            self.get_logger().info(f"Changed publisher topic to: {self.topic_name}")

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
        
        self.label = QLabel("Linear X: 0.00 | Angular Z: 0.00")
        self.label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(self.label)

        self.joystick = JoystickWidget()
        self.joystick.joystickMoved.connect(self.on_joystick_moved)
        layout.addWidget(self.joystick)

        main_widget.setLayout(layout)
        self.setCentralWidget(main_widget)

        # Integrate ROS 2 spin loop into PyQt6's event loop
        self.ros_timer = QTimer()
        self.ros_timer.timeout.connect(lambda: rclpy.spin_once(self.node, timeout_sec=0))
        self.ros_timer.start(10)  # Spin at 100Hz

    def on_joystick_moved(self, x, y):
        self.node.update_twist(x, y)
        self.label.setText(f"Linear X: {self.node.twist_msg.linear.x:.2f} | Angular Z: {self.node.twist_msg.angular.z:.2f}")
        
    def on_topic_update(self):
        new_topic = self.topic_input.text().strip()
        self.node.change_topic(new_topic)

def main(args=None):
    rclpy.init(args=args)
    app = QApplication(sys.argv)
    node = JoyNode()
    window = MainWindow(node)
    window.show()
    sys.exit(app.exec())

if __name__ == '__main__':
    main()