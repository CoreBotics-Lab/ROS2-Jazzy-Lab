from .z_std_msgs import (
    z_Int32, z_Int64, z_Float32, z_Float64, z_String, z_Bool,
    z_Int32MultiArray, z_Float32MultiArray, z_ByteMultiArray
)
from .node import (
    ZenohNode,
    ZenohConfig,
    QoS,
    SensorDataQoS,
    SystemDefaultsQoS,
    Reliability,
    Durability,
    ZenohPublisher,
    ZenohSubscription,
    ZenohTimer,
)
from .logging import (
    ZenohLogger,
    LogLevel,
    get_logger,
    set_logger_level,
    get_logger_effective_level,
)
from . import logging as z_logging
from . import std_msgs
from . import z_std_msgs
from . import geometry_msgs
from . import z_geometry_msgs
from . import sensor_msgs
from . import z_sensor_msgs
from . import nav_msgs
from . import z_nav_msgs
from . import custom_msgs
from . import custom_srvs
from . import std_srvs
from . import z_std_srvs
from . import builtin_interfaces
from . import z_builtin_interfaces

import time

def z_delay(ms: int) -> None:
    time.sleep(ms / 1000.0)

def copy_fields(ros_msg, zenoh_msg) -> None:
    """
    Utility function to automatically copy all matching fields from a Zenoh msg struct
    to a ROS 2 DDS msg struct (or vice-versa), bypassing manual field assignments.
    """
    if hasattr(ros_msg, 'get_fields_and_field_types'):
        fields = list(ros_msg.get_fields_and_field_types().keys())
    elif hasattr(ros_msg, '__slots__'):
        fields = [f.lstrip('_') for f in ros_msg.__slots__]
    else:
        fields = [attr for attr in dir(ros_msg) if not attr.startswith('_') and not callable(getattr(ros_msg, attr))]

    for field in fields:
        ros_field = f"_{field}" if hasattr(ros_msg, f"_{field}") else field
        zenoh_field = f"_{field}" if hasattr(zenoh_msg, f"_{field}") else field
        
        if hasattr(zenoh_msg, zenoh_field) and hasattr(ros_msg, ros_field):
            setattr(ros_msg, ros_field, getattr(zenoh_msg, zenoh_field))
