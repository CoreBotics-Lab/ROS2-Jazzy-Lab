#!/usr/bin/env python3
import os
import sys
import argparse
import shutil

# Basic ROS 2 data type mappings
TYPE_MAP_CPP = {
    'int8': 'int8_t',
    'uint8': 'uint8_t',
    'int16': 'int16_t',
    'uint16': 'uint16_t',
    'int32': 'int32_t',
    'uint32': 'uint32_t',
    'int64': 'int64_t',
    'uint64': 'uint64_t',
    'float32': 'float',
    'float64': 'double',
    'string': 'std::string',
    'String': 'std::string',
    'bool': 'bool'
}

TYPE_MAP_PYTHON = {
    'int8': 'int',
    'uint8': 'int',
    'int16': 'int',
    'uint16': 'int',
    'int32': 'int',
    'uint32': 'int',
    'int64': 'int',
    'uint64': 'int',
    'float32': 'float',
    'float64': 'float',
    'string': 'str',
    'String': 'str',
    'bool': 'bool'
}

# Standard nested types mapping — mirrors ROS 2 package/Type → generated C++ qualified name
NESTED_TYPES = {
    # geometry_msgs
    'geometry_msgs/Vector3': 'z_geometry_msgs::z_Vector3',
    'geometry_msgs/msg/Vector3': 'z_geometry_msgs::z_Vector3',
    'geometry_msgs/Point': 'z_geometry_msgs::z_Point',
    'geometry_msgs/msg/Point': 'z_geometry_msgs::z_Point',
    'geometry_msgs/Quaternion': 'z_geometry_msgs::z_Quaternion',
    'geometry_msgs/msg/Quaternion': 'z_geometry_msgs::z_Quaternion',
    'geometry_msgs/Twist': 'z_geometry_msgs::z_Twist',
    'geometry_msgs/msg/Twist': 'z_geometry_msgs::z_Twist',
    'geometry_msgs/Pose': 'z_geometry_msgs::z_Pose',
    'geometry_msgs/msg/Pose': 'z_geometry_msgs::z_Pose',
    # builtin_interfaces
    'builtin_interfaces/Time': 'builtin_interfaces::z_Time',
    'builtin_interfaces/msg/Time': 'builtin_interfaces::z_Time',
    'builtin_interfaces/Duration': 'builtin_interfaces::z_Duration',
    'builtin_interfaces/msg/Duration': 'builtin_interfaces::z_Duration',
    # std_msgs complex nested types
    'std_msgs/Header': 'z_std_msgs::z_Header',
    'std_msgs/msg/Header': 'z_std_msgs::z_Header',
    'std_msgs/ColorRGBA': 'z_std_msgs::z_ColorRGBA',
    'std_msgs/msg/ColorRGBA': 'z_std_msgs::z_ColorRGBA',
    # sensor_msgs
    'sensor_msgs/Imu': 'z_sensor_msgs::z_Imu',
    'sensor_msgs/msg/Imu': 'z_sensor_msgs::z_Imu',
    'sensor_msgs/Temperature': 'z_sensor_msgs::z_Temperature',
    'sensor_msgs/msg/Temperature': 'z_sensor_msgs::z_Temperature',
    'sensor_msgs/Range': 'z_sensor_msgs::z_Range',
    'sensor_msgs/msg/Range': 'z_sensor_msgs::z_Range',
    # nav_msgs
    'nav_msgs/Odometry': 'z_nav_msgs::z_Odometry',
    'nav_msgs/msg/Odometry': 'z_nav_msgs::z_Odometry',
    # geometry_msgs — new types
    'geometry_msgs/Point32': 'z_geometry_msgs::z_Point32',
    'geometry_msgs/msg/Point32': 'z_geometry_msgs::z_Point32',
    'geometry_msgs/Accel': 'z_geometry_msgs::z_Accel',
    'geometry_msgs/msg/Accel': 'z_geometry_msgs::z_Accel',
    'geometry_msgs/PoseStamped': 'z_geometry_msgs::z_PoseStamped',
    'geometry_msgs/msg/PoseStamped': 'z_geometry_msgs::z_PoseStamped',
    'geometry_msgs/TwistStamped': 'z_geometry_msgs::z_TwistStamped',
    'geometry_msgs/msg/TwistStamped': 'z_geometry_msgs::z_TwistStamped',
    'geometry_msgs/TransformStamped': 'z_geometry_msgs::z_TransformStamped',
    'geometry_msgs/msg/TransformStamped': 'z_geometry_msgs::z_TransformStamped',
    'geometry_msgs/PointStamped': 'z_geometry_msgs::z_PointStamped',
    'geometry_msgs/msg/PointStamped': 'z_geometry_msgs::z_PointStamped',
    'geometry_msgs/WrenchStamped': 'z_geometry_msgs::z_WrenchStamped',
    'geometry_msgs/msg/WrenchStamped': 'z_geometry_msgs::z_WrenchStamped',
    'geometry_msgs/PoseWithCovariance': 'z_geometry_msgs::z_PoseWithCovariance',
    'geometry_msgs/msg/PoseWithCovariance': 'z_geometry_msgs::z_PoseWithCovariance',
    'geometry_msgs/TwistWithCovariance': 'z_geometry_msgs::z_TwistWithCovariance',
    'geometry_msgs/msg/TwistWithCovariance': 'z_geometry_msgs::z_TwistWithCovariance',
    # sensor_msgs — new types
    'sensor_msgs/NavSatStatus': 'z_sensor_msgs::z_NavSatStatus',
    'sensor_msgs/msg/NavSatStatus': 'z_sensor_msgs::z_NavSatStatus',
    'sensor_msgs/NavSatFix': 'z_sensor_msgs::z_NavSatFix',
    'sensor_msgs/msg/NavSatFix': 'z_sensor_msgs::z_NavSatFix',
    'sensor_msgs/BatteryState': 'z_sensor_msgs::z_BatteryState',
    'sensor_msgs/msg/BatteryState': 'z_sensor_msgs::z_BatteryState',
    'sensor_msgs/RelativeHumidity': 'z_sensor_msgs::z_RelativeHumidity',
    'sensor_msgs/msg/RelativeHumidity': 'z_sensor_msgs::z_RelativeHumidity',
    'sensor_msgs/JoyFeedback': 'z_sensor_msgs::z_JoyFeedback',
    'sensor_msgs/msg/JoyFeedback': 'z_sensor_msgs::z_JoyFeedback',
    'sensor_msgs/RegionOfInterest': 'z_sensor_msgs::z_RegionOfInterest',
    'sensor_msgs/msg/RegionOfInterest': 'z_sensor_msgs::z_RegionOfInterest',
    'sensor_msgs/TimeReference': 'z_sensor_msgs::z_TimeReference',
    'sensor_msgs/msg/TimeReference': 'z_sensor_msgs::z_TimeReference',
    'sensor_msgs/Joy': 'z_sensor_msgs::z_Joy',
    'sensor_msgs/msg/Joy': 'z_sensor_msgs::z_Joy',
    'sensor_msgs/LaserScan': 'z_sensor_msgs::z_LaserScan',
    'sensor_msgs/msg/LaserScan': 'z_sensor_msgs::z_LaserScan',
    'sensor_msgs/JointState': 'z_sensor_msgs::z_JointState',
    'sensor_msgs/msg/JointState': 'z_sensor_msgs::z_JointState',
    # nav_msgs — new types
    'nav_msgs/MapMetaData': 'z_nav_msgs::z_MapMetaData',
    'nav_msgs/msg/MapMetaData': 'z_nav_msgs::z_MapMetaData',
    'nav_msgs/GridCells': 'z_nav_msgs::z_GridCells',
    'nav_msgs/msg/GridCells': 'z_nav_msgs::z_GridCells',
    'nav_msgs/Path': 'z_nav_msgs::z_Path',
    'nav_msgs/msg/Path': 'z_nav_msgs::z_Path',
}

NESTED_PYTHON_TYPES = {
    # geometry_msgs
    'geometry_msgs/Vector3': 'z_geometry_msgs.z_Vector3',
    'geometry_msgs/msg/Vector3': 'z_geometry_msgs.z_Vector3',
    'geometry_msgs/Point': 'z_geometry_msgs.z_Point',
    'geometry_msgs/msg/Point': 'z_geometry_msgs.z_Point',
    'geometry_msgs/Quaternion': 'z_geometry_msgs.z_Quaternion',
    'geometry_msgs/msg/Quaternion': 'z_geometry_msgs.z_Quaternion',
    'geometry_msgs/Twist': 'z_geometry_msgs.z_Twist',
    'geometry_msgs/msg/Twist': 'z_geometry_msgs.z_Twist',
    'geometry_msgs/Pose': 'z_geometry_msgs.z_Pose',
    'geometry_msgs/msg/Pose': 'z_geometry_msgs.z_Pose',
    # builtin_interfaces
    'builtin_interfaces/Time': 'z_builtin_interfaces.z_Time',
    'builtin_interfaces/msg/Time': 'z_builtin_interfaces.z_Time',
    'builtin_interfaces/Duration': 'z_builtin_interfaces.z_Duration',
    'builtin_interfaces/msg/Duration': 'z_builtin_interfaces.z_Duration',
    # std_msgs complex nested types
    'std_msgs/Header': 'z_std_msgs.z_Header',
    'std_msgs/msg/Header': 'z_std_msgs.z_Header',
    'std_msgs/ColorRGBA': 'z_std_msgs.z_ColorRGBA',
    'std_msgs/msg/ColorRGBA': 'z_std_msgs.z_ColorRGBA',
    # sensor_msgs
    'sensor_msgs/Imu': 'z_sensor_msgs.z_Imu',
    'sensor_msgs/msg/Imu': 'z_sensor_msgs.z_Imu',
    'sensor_msgs/Temperature': 'z_sensor_msgs.z_Temperature',
    'sensor_msgs/msg/Temperature': 'z_sensor_msgs.z_Temperature',
    'sensor_msgs/Range': 'z_sensor_msgs.z_Range',
    'sensor_msgs/msg/Range': 'z_sensor_msgs.z_Range',
    # nav_msgs
    'nav_msgs/Odometry': 'z_nav_msgs.z_Odometry',
    'nav_msgs/msg/Odometry': 'z_nav_msgs.z_Odometry',
    # geometry_msgs — new types
    'geometry_msgs/Point32': 'z_geometry_msgs.z_Point32',
    'geometry_msgs/msg/Point32': 'z_geometry_msgs.z_Point32',
    'geometry_msgs/Accel': 'z_geometry_msgs.z_Accel',
    'geometry_msgs/msg/Accel': 'z_geometry_msgs.z_Accel',
    'geometry_msgs/PoseStamped': 'z_geometry_msgs.z_PoseStamped',
    'geometry_msgs/msg/PoseStamped': 'z_geometry_msgs.z_PoseStamped',
    'geometry_msgs/TwistStamped': 'z_geometry_msgs.z_TwistStamped',
    'geometry_msgs/msg/TwistStamped': 'z_geometry_msgs.z_TwistStamped',
    'geometry_msgs/TransformStamped': 'z_geometry_msgs.z_TransformStamped',
    'geometry_msgs/msg/TransformStamped': 'z_geometry_msgs.z_TransformStamped',
    'geometry_msgs/PointStamped': 'z_geometry_msgs.z_PointStamped',
    'geometry_msgs/msg/PointStamped': 'z_geometry_msgs.z_PointStamped',
    'geometry_msgs/WrenchStamped': 'z_geometry_msgs.z_WrenchStamped',
    'geometry_msgs/msg/WrenchStamped': 'z_geometry_msgs.z_WrenchStamped',
    'geometry_msgs/PoseWithCovariance': 'z_geometry_msgs.z_PoseWithCovariance',
    'geometry_msgs/msg/PoseWithCovariance': 'z_geometry_msgs.z_PoseWithCovariance',
    'geometry_msgs/TwistWithCovariance': 'z_geometry_msgs.z_TwistWithCovariance',
    'geometry_msgs/msg/TwistWithCovariance': 'z_geometry_msgs.z_TwistWithCovariance',
    # sensor_msgs — new types
    'sensor_msgs/NavSatStatus': 'z_sensor_msgs.z_NavSatStatus',
    'sensor_msgs/msg/NavSatStatus': 'z_sensor_msgs.z_NavSatStatus',
    'sensor_msgs/NavSatFix': 'z_sensor_msgs.z_NavSatFix',
    'sensor_msgs/msg/NavSatFix': 'z_sensor_msgs.z_NavSatFix',
    'sensor_msgs/BatteryState': 'z_sensor_msgs.z_BatteryState',
    'sensor_msgs/msg/BatteryState': 'z_sensor_msgs.z_BatteryState',
    'sensor_msgs/RelativeHumidity': 'z_sensor_msgs.z_RelativeHumidity',
    'sensor_msgs/msg/RelativeHumidity': 'z_sensor_msgs.z_RelativeHumidity',
    'sensor_msgs/JoyFeedback': 'z_sensor_msgs.z_JoyFeedback',
    'sensor_msgs/msg/JoyFeedback': 'z_sensor_msgs.z_JoyFeedback',
    'sensor_msgs/RegionOfInterest': 'z_sensor_msgs.z_RegionOfInterest',
    'sensor_msgs/msg/RegionOfInterest': 'z_sensor_msgs.z_RegionOfInterest',
    'sensor_msgs/TimeReference': 'z_sensor_msgs.z_TimeReference',
    'sensor_msgs/msg/TimeReference': 'z_sensor_msgs.z_TimeReference',
    'sensor_msgs/Joy': 'z_sensor_msgs.z_Joy',
    'sensor_msgs/msg/Joy': 'z_sensor_msgs.z_Joy',
    'sensor_msgs/LaserScan': 'z_sensor_msgs.z_LaserScan',
    'sensor_msgs/msg/LaserScan': 'z_sensor_msgs.z_LaserScan',
    'sensor_msgs/JointState': 'z_sensor_msgs.z_JointState',
    'sensor_msgs/msg/JointState': 'z_sensor_msgs.z_JointState',
    # nav_msgs — new types
    'nav_msgs/MapMetaData': 'z_nav_msgs.z_MapMetaData',
    'nav_msgs/msg/MapMetaData': 'z_nav_msgs.z_MapMetaData',
    'nav_msgs/GridCells': 'z_nav_msgs.z_GridCells',
    'nav_msgs/msg/GridCells': 'z_nav_msgs.z_GridCells',
    'nav_msgs/Path': 'z_nav_msgs.z_Path',
    'nav_msgs/msg/Path': 'z_nav_msgs.z_Path',
}

def parse_fields(lines):
    fields = []
    for line in lines:
        line = line.strip()
        if not line or line.startswith('#'):
            continue
        parts = line.split()
        if len(parts) >= 2:
            field_type = parts[0]
            field_name = parts[1]
            fields.append((field_type, field_name))
    return fields

def get_cpp_type(raw_type):
    if raw_type in TYPE_MAP_CPP:
        return TYPE_MAP_CPP[raw_type]
    if raw_type in NESTED_TYPES:
        return NESTED_TYPES[raw_type]
    # Unknown nested type: derive from package/Name convention → package::z_Name
    if '/' in raw_type:
        parts = raw_type.split('/')
        pkg, name = parts[0], parts[-1]
        return f'{pkg}::z_{name}'
    return raw_type

def get_python_type(raw_type):
    if raw_type in TYPE_MAP_PYTHON:
        return TYPE_MAP_PYTHON[raw_type]
    if raw_type in NESTED_PYTHON_TYPES:
        return NESTED_PYTHON_TYPES[raw_type]
    # Unknown nested type: package/Name → package.z_Name
    if '/' in raw_type:
        parts = raw_type.split('/')
        pkg, name = parts[0], parts[-1]
        return f"z_{name}"
    return 'Any'

def get_python_default(raw_type):
    if raw_type == 'bool':
        return 'False'
    if raw_type in ['float32', 'float64']:
        return '0.0'
    if raw_type in ['string', 'String']:
        return '""'
    if raw_type in NESTED_PYTHON_TYPES:
        return f"{NESTED_PYTHON_TYPES[raw_type]}()"
    if '/' in raw_type:
        return f"{get_python_type(raw_type)}()"
    if raw_type in TYPE_MAP_PYTHON:
        return '0'
    return 'None'

# ---------------------------------------------------------------------------
# Maps pre-defined package name → include path (relative to library root)
# ---------------------------------------------------------------------------
_PRE_DEFINED_INCLUDE_MCU = {
    'geometry_msgs':      'msg_interface/pre_defined_interface/z_geometry_msgs.h',
    'builtin_interfaces': 'msg_interface/pre_defined_interface/z_builtin_interfaces.h',
    'std_msgs':           'msg_interface/pre_defined_interface/z_std_msgs.h',
    'sensor_msgs':        'msg_interface/pre_defined_interface/z_sensor_msgs.h',
    'nav_msgs':           'msg_interface/pre_defined_interface/z_nav_msgs.h',
}
_PRE_DEFINED_INCLUDE_PC = {
    'geometry_msgs':      'msg_interface/pre_defined_interface/z_geometry_msgs.h',
    'builtin_interfaces': 'msg_interface/pre_defined_interface/z_builtin_interfaces.h',
    'std_msgs':           'msg_interface/pre_defined_interface/z_std_msgs.h',
    'sensor_msgs':        'msg_interface/pre_defined_interface/z_sensor_msgs.h',
    'nav_msgs':           'msg_interface/pre_defined_interface/z_nav_msgs.h',
}

def _field_pkg(f_type):
    """Return the top-level package name from a nested type, or None for primitives."""
    return f_type.split('/')[0] if '/' in f_type else None

def get_cpp_includes(fields, is_mcu=True, out_package=None):
    """
    Return all #include directives needed for the nested types in 'fields'.
    - Pre-defined packages (geometry_msgs, builtin_interfaces, std_msgs) map to
      their canonical pre-defined header or per-message include.
    - Custom packages resolve to sibling directories inside zenoh_ros/zenoh_ros/.
    """
    includes = []
    seen = set()
    lookup = _PRE_DEFINED_INCLUDE_PC if not is_mcu else _PRE_DEFINED_INCLUDE_MCU

    for f_type, _ in fields:
        pkg = _field_pkg(f_type)
        if pkg is None:
            continue  # primitive — no include needed

        nested_name = f_type.split('/')[-1]
        if pkg in lookup:
            inc = f'#include "{lookup[pkg]}"'
            if inc not in seen:
                seen.add(inc)
                includes.append(inc)
        else:
            # Custom nested type: derive the per-type include path
            if out_package and pkg == out_package:
                inc = f'#include "z_{nested_name}.h"'          # same directory
            else:
                inc = f'#include <zenoh_ros/{pkg}/z_{nested_name}.h>' # standard include path
            if inc not in seen:
                seen.add(inc)
                includes.append(inc)

    return includes

def get_cpp_using_declarations(fields):
    """
    Return 'using Alias = Qualified::Type;' lines for every nested type.
    This makes all nested types available in user code with a single #include.
    Also propagates transitive aliases (z_Header → z_Time, z_Duration).
    """
    decls = []
    seen = set()

    for f_type, _ in fields:
        if f_type in NESTED_TYPES:
            cpp_qual = NESTED_TYPES[f_type]          # e.g. 'z_std_msgs::z_Header'
            alias = cpp_qual.split('::')[-1]          # e.g. 'z_Header'
        elif '/' in f_type:
            pkg  = f_type.split('/')[0]
            name = f_type.split('/')[-1]
            alias    = f'z_{name}'
            cpp_qual = f'{pkg}::z_{name}'
        else:
            continue  # primitive

        if alias not in seen:
            seen.add(alias)
            decls.append(f'using {alias} = {cpp_qual};')

        # Transitive: z_Header brings z_Time and z_Duration
        if 'z_Header' in alias:
            for extra_alias, extra_qual in [
                ('z_Time',     'builtin_interfaces::z_Time'),
                ('z_Duration', 'builtin_interfaces::z_Duration'),
            ]:
                if extra_alias not in seen:
                    seen.add(extra_alias)
                    decls.append(f'using {extra_alias} = {extra_qual};')

    return decls

def get_python_package_imports(fields, out_package=None):
    """
    Return import statements for all nested types found in 'fields'.
    Pre-defined packages → pre_defined_interface module.
    Custom packages      → msg_interface.custom_interface path.
    """
    imports = set()
    for f_type, _ in fields:
        pkg = _field_pkg(f_type)
        if pkg is None:
            continue
        nested_name = f_type.split('/')[-1]
        if pkg == 'geometry_msgs':
            imports.add('from zenoh_ros.msg_interface.pre_defined_interface import z_geometry_msgs')
        elif pkg == 'builtin_interfaces':
            imports.add('from zenoh_ros.msg_interface.pre_defined_interface import z_builtin_interfaces')
        elif pkg == 'std_msgs':
            imports.add('from zenoh_ros.msg_interface.pre_defined_interface import z_std_msgs')
        elif pkg == 'sensor_msgs':
            imports.add('from zenoh_ros.msg_interface.pre_defined_interface import z_sensor_msgs')
        elif pkg == 'nav_msgs':
            imports.add('from zenoh_ros.msg_interface.pre_defined_interface import z_nav_msgs')
        else:
            # Custom nested type
            imports.add(f'from zenoh_ros.custom_msgs import z_{nested_name}')
    return sorted(imports)

# Generate Inline Nesting Serialization/Deserialization C++ helper code
def generate_cpp_field_serialization(f_type, f_name, doc_var, msg_var):
    if f_type in TYPE_MAP_CPP:
        return f'{doc_var}["{f_name}"] = {msg_var}.{f_name};'

    if 'Vector3' in f_type or 'Point' in f_type:
        return (f'  JsonArray {f_name}_arr = {doc_var}["{f_name}"].to<JsonArray>();\n'
                f'  {f_name}_arr.add({msg_var}.{f_name}.x);\n'
                f'  {f_name}_arr.add({msg_var}.{f_name}.y);\n'
                f'  {f_name}_arr.add({msg_var}.{f_name}.z);')

    if 'Quaternion' in f_type:
        return (f'  JsonArray {f_name}_arr = {doc_var}["{f_name}"].to<JsonArray>();\n'
                f'  {f_name}_arr.add({msg_var}.{f_name}.x);\n'
                f'  {f_name}_arr.add({msg_var}.{f_name}.y);\n'
                f'  {f_name}_arr.add({msg_var}.{f_name}.z);\n'
                f'  {f_name}_arr.add({msg_var}.{f_name}.w);')

    if 'Twist' in f_type:
        return (f'  JsonArray {f_name}_arr = {doc_var}["{f_name}"].to<JsonArray>();\n'
                f'  JsonArray {f_name}_lin = {f_name}_arr.add<JsonArray>();\n'
                f'  {f_name}_lin.add({msg_var}.{f_name}.linear.x);\n'
                f'  {f_name}_lin.add({msg_var}.{f_name}.linear.y);\n'
                f'  {f_name}_lin.add({msg_var}.{f_name}.linear.z);\n'
                f'  JsonArray {f_name}_ang = {f_name}_arr.add<JsonArray>();\n'
                f'  {f_name}_ang.add({msg_var}.{f_name}.angular.x);\n'
                f'  {f_name}_ang.add({msg_var}.{f_name}.angular.y);\n'
                f'  {f_name}_ang.add({msg_var}.{f_name}.angular.z);')

    # builtin_interfaces/Time or Duration → nested JSON object { sec, nanosec }
    if 'Time' in f_type or 'Duration' in f_type:
        return (f'  JsonObject {f_name}_obj = {doc_var}["{f_name}"].to<JsonObject>();\n'
                f'  {f_name}_obj["sec"]     = {msg_var}.{f_name}.sec;\n'
                f'  {f_name}_obj["nanosec"] = {msg_var}.{f_name}.nanosec;')

    # std_msgs/ColorRGBA → nested JSON object { r, g, b, a }
    if 'ColorRGBA' in f_type:
        return (f'  JsonObject {f_name}_obj = {doc_var}["{f_name}"].to<JsonObject>();\n'
                f'  {f_name}_obj["r"] = {msg_var}.{f_name}.r;\n'
                f'  {f_name}_obj["g"] = {msg_var}.{f_name}.g;\n'
                f'  {f_name}_obj["b"] = {msg_var}.{f_name}.b;\n'
                f'  {f_name}_obj["a"] = {msg_var}.{f_name}.a;')

    # std_msgs/Header → nested JSON object { stamp: { sec, nanosec }, frame_id }
    if 'Header' in f_type:
        return (f'  JsonObject {f_name}_obj   = {doc_var}["{f_name}"].to<JsonObject>();\n'
                f'  JsonObject {f_name}_stamp = {f_name}_obj["stamp"].to<JsonObject>();\n'
                f'  {f_name}_stamp["sec"]     = {msg_var}.{f_name}.stamp.sec;\n'
                f'  {f_name}_stamp["nanosec"] = {msg_var}.{f_name}.stamp.nanosec;\n'
                f'  {f_name}_obj["frame_id"]  = {msg_var}.{f_name}.frame_id;')

    # Generic custom nested message type (MCU): use serialize_msg / deserialize_msg via temporary buffer
    if '/' in f_type:
        return (f'  static uint8_t {f_name}_buf[512];\n'
                f'  size_t {f_name}_len = serialize_msg({msg_var}.{f_name}, {f_name}_buf, sizeof({f_name}_buf));\n'
                f'  JsonDocument {f_name}_sub_doc;\n'
                f'  deserializeMsgPack({f_name}_sub_doc, {f_name}_buf, {f_name}_len);\n'
                f'  {doc_var}["{f_name}"] = {f_name}_sub_doc;')

    return f'// Unsupported nested type: {f_type}'

def generate_cpp_field_deserialization(f_type, f_name, doc_var, msg_var):
    if f_type in TYPE_MAP_CPP:
        return f'{msg_var}.{f_name} = {doc_var}["{f_name}"].as<{get_cpp_type(f_type)}>();'

    if 'Vector3' in f_type or 'Point' in f_type:
        return (f'  {msg_var}.{f_name}.x = {doc_var}["{f_name}"][0].as<double>();\n'
                f'  {msg_var}.{f_name}.y = {doc_var}["{f_name}"][1].as<double>();\n'
                f'  {msg_var}.{f_name}.z = {doc_var}["{f_name}"][2].as<double>();')

    if 'Quaternion' in f_type:
        return (f'  {msg_var}.{f_name}.x = {doc_var}["{f_name}"][0].as<double>();\n'
                f'  {msg_var}.{f_name}.y = {doc_var}["{f_name}"][1].as<double>();\n'
                f'  {msg_var}.{f_name}.z = {doc_var}["{f_name}"][2].as<double>();\n'
                f'  {msg_var}.{f_name}.w = {doc_var}["{f_name}"][3].as<double>();')

    if 'Twist' in f_type:
        return (f'  {msg_var}.{f_name}.linear.x = {doc_var}["{f_name}"][0][0].as<double>();\n'
                f'  {msg_var}.{f_name}.linear.y = {doc_var}["{f_name}"][0][1].as<double>();\n'
                f'  {msg_var}.{f_name}.linear.z = {doc_var}["{f_name}"][0][2].as<double>();\n'
                f'  {msg_var}.{f_name}.angular.x = {doc_var}["{f_name}"][1][0].as<double>();\n'
                f'  {msg_var}.{f_name}.angular.y = {doc_var}["{f_name}"][1][1].as<double>();\n'
                f'  {msg_var}.{f_name}.angular.z = {doc_var}["{f_name}"][1][2].as<double>();')

    # builtin_interfaces/Time or Duration
    if 'Time' in f_type or 'Duration' in f_type:
        return (f'  {msg_var}.{f_name}.sec     = {doc_var}["{f_name}"]["sec"].as<int32_t>();\n'
                f'  {msg_var}.{f_name}.nanosec = {doc_var}["{f_name}"]["nanosec"].as<uint32_t>();')

    # std_msgs/ColorRGBA
    if 'ColorRGBA' in f_type:
        return (f'  {msg_var}.{f_name}.r = {doc_var}["{f_name}"]["r"].as<float>();\n'
                f'  {msg_var}.{f_name}.g = {doc_var}["{f_name}"]["g"].as<float>();\n'
                f'  {msg_var}.{f_name}.b = {doc_var}["{f_name}"]["b"].as<float>();\n'
                f'  {msg_var}.{f_name}.a = {doc_var}["{f_name}"]["a"].as<float>();')

    # std_msgs/Header
    if 'Header' in f_type:
        return (f'  {msg_var}.{f_name}.stamp.sec     = {doc_var}["{f_name}"]["stamp"]["sec"].as<int32_t>();\n'
                f'  {msg_var}.{f_name}.stamp.nanosec = {doc_var}["{f_name}"]["stamp"]["nanosec"].as<uint32_t>();\n'
                f'  {msg_var}.{f_name}.frame_id      = {doc_var}["{f_name}"]["frame_id"].as<std::string>();')

    # Generic custom nested message type (MCU)
    if '/' in f_type:
        return (f'  static uint8_t {f_name}_buf[512];\n'
                f'  size_t {f_name}_len = serializeMsgPack({doc_var}["{f_name}"], {f_name}_buf, sizeof({f_name}_buf));\n'
                f'  deserialize_msg({f_name}_buf, {f_name}_len, {msg_var}.{f_name});')

    return f'// Unsupported nested type: {f_type}'

# PC C++ Serialization/Deserialization using nlohmann/json
def generate_pc_cpp_field_serialization(f_type, f_name, json_var, msg_var):
    if f_type in TYPE_MAP_CPP:
        return f'{json_var}["{f_name}"] = {msg_var}.{f_name};'

    if 'Vector3' in f_type or 'Point' in f_type:
        return f'{json_var}["{f_name}"] = {{{msg_var}.{f_name}.x, {msg_var}.{f_name}.y, {msg_var}.{f_name}.z}};'

    if 'Quaternion' in f_type:
        return f'{json_var}["{f_name}"] = {{{msg_var}.{f_name}.x, {msg_var}.{f_name}.y, {msg_var}.{f_name}.z, {msg_var}.{f_name}.w}};'

    if 'Twist' in f_type:
        return (f'{json_var}["{f_name}"] = {{\n'
                f'    {{{msg_var}.{f_name}.linear.x, {msg_var}.{f_name}.linear.y, {msg_var}.{f_name}.linear.z}},\n'
                f'    {{{msg_var}.{f_name}.angular.x, {msg_var}.{f_name}.angular.y, {msg_var}.{f_name}.angular.z}}\n'
                f'  }};')

    # builtin_interfaces/Time or Duration
    if 'Time' in f_type or 'Duration' in f_type:
        return (f'{json_var}["{f_name}"] = {{{{"sec", {msg_var}.{f_name}.sec}}, {{"nanosec", {msg_var}.{f_name}.nanosec}}}};')

    # std_msgs/ColorRGBA
    if 'ColorRGBA' in f_type:
        return (f'{json_var}["{f_name}"] = {{{{"r", {msg_var}.{f_name}.r}}, {{"g", {msg_var}.{f_name}.g}}, '
                f'{{"b", {msg_var}.{f_name}.b}}, {{"a", {msg_var}.{f_name}.a}}}};')

    # std_msgs/Header
    if 'Header' in f_type:
        return (f'{json_var}["{f_name}"] = {{{{"stamp", {{{{"sec", {msg_var}.{f_name}.stamp.sec}}, '
                f'{{"nanosec", {msg_var}.{f_name}.stamp.nanosec}}}}}}, {{"frame_id", {msg_var}.{f_name}.frame_id}}}};')

    # Generic custom nested message type (PC C++)
    if '/' in f_type:
        return f'{json_var}["{f_name}"] = nlohmann::json::from_msgpack(serialize_msg({msg_var}.{f_name}));'

    return f'// Unsupported nested type: {f_type}'

def generate_pc_cpp_field_deserialization(f_type, f_name, json_var, msg_var):
    if f_type in TYPE_MAP_CPP:
        return f'{msg_var}.{f_name} = {json_var}["{f_name}"].get<{get_cpp_type(f_type)}>();'

    if 'Vector3' in f_type or 'Point' in f_type:
        return (f'  {msg_var}.{f_name}.x = {json_var}["{f_name}"][0].get<double>();\n'
                f'  {msg_var}.{f_name}.y = {json_var}["{f_name}"][1].get<double>();\n'
                f'  {msg_var}.{f_name}.z = {json_var}["{f_name}"][2].get<double>();')

    if 'Quaternion' in f_type:
        return (f'  {msg_var}.{f_name}.x = {json_var}["{f_name}"][0].get<double>();\n'
                f'  {msg_var}.{f_name}.y = {json_var}["{f_name}"][1].get<double>();\n'
                f'  {msg_var}.{f_name}.z = {json_var}["{f_name}"][2].get<double>();\n'
                f'  {msg_var}.{f_name}.w = {json_var}["{f_name}"][3].get<double>();')

    if 'Twist' in f_type:
        return (f'  {msg_var}.{f_name}.linear.x = {json_var}["{f_name}"][0][0].get<double>();\n'
                f'  {msg_var}.{f_name}.linear.y = {json_var}["{f_name}"][0][1].get<double>();\n'
                f'  {msg_var}.{f_name}.linear.z = {json_var}["{f_name}"][0][2].get<double>();\n'
                f'  {msg_var}.{f_name}.angular.x = {json_var}["{f_name}"][1][0].get<double>();\n'
                f'  {msg_var}.{f_name}.angular.y = {json_var}["{f_name}"][1][1].get<double>();\n'
                f'  {msg_var}.{f_name}.angular.z = {json_var}["{f_name}"][1][2].get<double>();')

    # builtin_interfaces/Time or Duration
    if 'Time' in f_type or 'Duration' in f_type:
        return (f'  {msg_var}.{f_name}.sec     = {json_var}["{f_name}"]["sec"].get<int32_t>();\n'
                f'  {msg_var}.{f_name}.nanosec = {json_var}["{f_name}"]["nanosec"].get<uint32_t>();')

    # std_msgs/ColorRGBA
    if 'ColorRGBA' in f_type:
        return (f'  {msg_var}.{f_name}.r = {json_var}["{f_name}"]["r"].get<float>();\n'
                f'  {msg_var}.{f_name}.g = {json_var}["{f_name}"]["g"].get<float>();\n'
                f'  {msg_var}.{f_name}.b = {json_var}["{f_name}"]["b"].get<float>();\n'
                f'  {msg_var}.{f_name}.a = {json_var}["{f_name}"]["a"].get<float>();')

    # std_msgs/Header
    if 'Header' in f_type:
        return (f'  {msg_var}.{f_name}.stamp.sec     = {json_var}["{f_name}"]["stamp"]["sec"].get<int32_t>();\n'
                f'  {msg_var}.{f_name}.stamp.nanosec = {json_var}["{f_name}"]["stamp"]["nanosec"].get<uint32_t>();\n'
                f'  {msg_var}.{f_name}.frame_id      = {json_var}["{f_name}"]["frame_id"].get<std::string>();')

    # Generic custom nested message type (PC C++)
    if '/' in f_type:
        return (f'  std::vector<uint8_t> {f_name}_bytes = nlohmann::json::to_msgpack({json_var}["{f_name}"]);\n'
                f'  deserialize_msg({f_name}_bytes, {msg_var}.{f_name});')

    return f'// Unsupported nested type: {f_type}'

# Python field serialization helpers
def generate_python_serialize_field(f_type, f_name):
    if f_type in TYPE_MAP_PYTHON:
        return f'"{f_name}": self.{f_name}'
    if 'Vector3' in f_type or 'Point' in f_type:
        return f'"{f_name}": [self.{f_name}.x, self.{f_name}.y, self.{f_name}.z]'
    if 'Quaternion' in f_type:
        return f'"{f_name}": [self.{f_name}.x, self.{f_name}.y, self.{f_name}.z, self.{f_name}.w]'
    if 'Twist' in f_type:
        return (f'"{f_name}": [\n'
                f'                [self.{f_name}.linear.x, self.{f_name}.linear.y, self.{f_name}.linear.z],\n'
                f'                [self.{f_name}.angular.x, self.{f_name}.angular.y, self.{f_name}.angular.z]\n'
                f'            ]')
    # builtin_interfaces/Time or Duration → dict { sec, nanosec }
    if 'Time' in f_type or 'Duration' in f_type:
        return f'"{f_name}": {{"sec": self.{f_name}.sec, "nanosec": self.{f_name}.nanosec}}'
    # std_msgs/ColorRGBA → dict { r, g, b, a }
    if 'ColorRGBA' in f_type:
        return f'"{f_name}": {{"r": self.{f_name}.r, "g": self.{f_name}.g, "b": self.{f_name}.b, "a": self.{f_name}.a}}'
    # std_msgs/Header → nested dict
    if 'Header' in f_type:
        return (f'"{f_name}": {{"stamp": {{"sec": self.{f_name}.stamp.sec, "nanosec": self.{f_name}.stamp.nanosec}}, '
                f'"frame_id": self.{f_name}.frame_id}}')
    # Generic custom nested type: delegate to the type's own serialize()
    if '/' in f_type:
        return f'"{f_name}": msgpack.unpackb(self.{f_name}.serialize())'
    return f'"{f_name}": None'

def _get_nested_sub(f_name):
    """Helper snippet: safely extract a nested sub-dict from msgpack data."""
    return f'(data.get(b"{f_name}", data.get("{f_name}", {{}})) or {{}})'

def generate_python_deserialize_field(f_type, f_name):
    if f_type in ['string', 'String']:
        return f'(lambda v: v.decode("utf-8") if isinstance(v, bytes) else str(v))(data.get(b"{f_name}", data.get("{f_name}", "")))'
    if f_type in TYPE_MAP_PYTHON:
        py_type = get_python_type(f_type)
        default_val = get_python_default(f_type)
        return f'{py_type}(data.get(b"{f_name}", data.get("{f_name}", {default_val})))'

    if 'Vector3' in f_type or 'Point' in f_type:
        pkg = 'z_geometry_msgs'
        cls_name = 'z_Vector3' if 'Vector3' in f_type else 'z_Point'
        return (f'{pkg}.{cls_name}(\n'
                f'                float(data.get(b"{f_name}", data.get("{f_name}", [0,0,0]))[0]),\n'
                f'                float(data.get(b"{f_name}", data.get("{f_name}", [0,0,0]))[1]),\n'
                f'                float(data.get(b"{f_name}", data.get("{f_name}", [0,0,0]))[2])\n'
                f'            )')

    if 'Quaternion' in f_type:
        pkg = 'z_geometry_msgs'
        return (f'{pkg}.z_Quaternion(\n'
                f'                float(data.get(b"{f_name}", data.get("{f_name}", [0,0,0,1]))[0]),\n'
                f'                float(data.get(b"{f_name}", data.get("{f_name}", [0,0,0,1]))[1]),\n'
                f'                float(data.get(b"{f_name}", data.get("{f_name}", [0,0,0,1]))[2]),\n'
                f'                float(data.get(b"{f_name}", data.get("{f_name}", [0,0,0,1]))[3])\n'
                f'            )')

    if 'Twist' in f_type:
        pkg = 'z_geometry_msgs'
        return (f'{pkg}.z_Twist(\n'
                f'                linear={pkg}.z_Vector3(\n'
                f'                    float(data.get(b"{f_name}", data.get("{f_name}", [[0,0,0],[0,0,0]]))[0][0]),\n'
                f'                    float(data.get(b"{f_name}", data.get("{f_name}", [[0,0,0],[0,0,0]]))[0][1]),\n'
                f'                    float(data.get(b"{f_name}", data.get("{f_name}", [[0,0,0],[0,0,0]]))[0][2])\n'
                f'                ),\n'
                f'                angular={pkg}.z_Vector3(\n'
                f'                    float(data.get(b"{f_name}", data.get("{f_name}", [[0,0,0],[0,0,0]]))[1][0]),\n'
                f'                    float(data.get(b"{f_name}", data.get("{f_name}", [[0,0,0],[0,0,0]]))[1][1]),\n'
                f'                    float(data.get(b"{f_name}", data.get("{f_name}", [[0,0,0],[0,0,0]]))[1][2])\n'
                f'                )\n'
                f'            )')

    # builtin_interfaces/Time or Duration: delegate to type's deserialize() with re-packed sub-dict
    if 'Time' in f_type or 'Duration' in f_type:
        py_type = get_python_type(f_type)
        return (f'{py_type}.deserialize(msgpack.packb({_get_nested_sub(f_name)}))')

    # std_msgs/ColorRGBA
    if 'ColorRGBA' in f_type:
        py_type = get_python_type(f_type)
        return f'{py_type}.deserialize(msgpack.packb({_get_nested_sub(f_name)}))'

    # std_msgs/Header
    if 'Header' in f_type:
        py_type = get_python_type(f_type)
        return f'{py_type}.deserialize(msgpack.packb({_get_nested_sub(f_name)}))'

    # Generic custom nested type: delegate to type's own deserialize()
    if '/' in f_type:
        py_type = get_python_type(f_type)
        return f'{py_type}.deserialize(msgpack.packb({_get_nested_sub(f_name)}))'

    return 'None'

def generate_mcu_header(package, name, fields, is_srv=False, req_fields=None, res_fields=None):
    z_name = f"z_{name}"
    guard = f"CUSTOM_INTERFACE_{package.upper()}_{z_name.upper()}_H"
    
    # Process includes
    all_fields = fields if not is_srv else (req_fields + res_fields)
    includes = get_cpp_includes(all_fields, is_mcu=True, out_package=package)
    include_str = '\n'.join(includes)
    if include_str:
        include_str += '\n'
        
    content = f"""#ifndef {guard}
#define {guard}

#include <ArduinoJson.h>
{include_str}
namespace {package} {{
"""
    if not is_srv:
        # Generate topic message struct
        struct_fields = '\n'.join([f"    {get_cpp_type(ftype)} {fname};" for ftype, fname in fields])
        content += f"""struct {z_name} {{
{struct_fields}
}};
}} // namespace {package}

// --- Topic Serialization Override ---
template <>
inline size_t serialize_msg<{package}::{z_name}>(
    const {package}::{z_name}& msg, uint8_t* buffer, size_t max_len) {{
    JsonDocument doc;
"""
        for ftype, fname in fields:
            content += f"    {generate_cpp_field_serialization(ftype, fname, 'doc', 'msg')}\n"
        content += f"""    return serializeMsgPack(doc, buffer, max_len);
}}

// --- Topic Deserialization Override ---
template <>
inline void deserialize_msg<{package}::{z_name}>(
    const uint8_t* buffer, size_t len, {package}::{z_name}& msg) {{
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
"""
        for ftype, fname in fields:
            content += f"    {generate_cpp_field_deserialization(ftype, fname, 'doc', 'msg')}\n"
        content += f"""}}
"""
    else:
        # Generate service message struct (nested classes)
        req_struct = '\n'.join([f"        {get_cpp_type(ftype)} {fname};" for ftype, fname in req_fields])
        res_struct = '\n'.join([f"        {get_cpp_type(ftype)} {fname};" for ftype, fname in res_fields])
        content += f"""struct {z_name} {{
    struct Request {{
{req_struct}
    }};

    struct Response {{
{res_struct}
    }};
}};
}} // namespace {package}

// --- Service Request Serializer ---
template <>
inline size_t serialize_msg<{package}::{z_name}::Request>(
    const {package}::{z_name}::Request& msg, uint8_t* buffer, size_t max_len) {{
    JsonDocument doc;
"""
        for ftype, fname in req_fields:
            content += f"    {generate_cpp_field_serialization(ftype, fname, 'doc', 'msg')}\n"
        content += f"""    return serializeMsgPack(doc, buffer, max_len);
}}

// --- Service Request Deserializer ---
template <>
inline void deserialize_msg<{package}::{z_name}::Request>(
    const uint8_t* buffer, size_t len, {package}::{z_name}::Request& msg) {{
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
"""
        for ftype, fname in req_fields:
            content += f"    {generate_cpp_field_deserialization(ftype, fname, 'doc', 'msg')}\n"
        content += f"""}}

// --- Service Response Serializer ---
template <>
inline size_t serialize_msg<{package}::{z_name}::Response>(
    const {package}::{z_name}::Response& msg, uint8_t* buffer, size_t max_len) {{
    JsonDocument doc;
"""
        for ftype, fname in res_fields:
            content += f"    {generate_cpp_field_serialization(ftype, fname, 'doc', 'msg')}\n"
        content += f"""    return serializeMsgPack(doc, buffer, max_len);
}}

// --- Service Response Deserializer ---
template <>
inline void deserialize_msg<{package}::{z_name}::Response>(
    const uint8_t* buffer, size_t len, {package}::{z_name}::Response& msg) {{
    JsonDocument doc;
    deserializeMsgPack(doc, buffer, len);
"""
        for ftype, fname in res_fields:
            content += f"    {generate_cpp_field_deserialization(ftype, fname, 'doc', 'msg')}\n"
        content += f"""}}
"""
    # Convenience using declarations so a single #include brings in all nested types
    using_decls = get_cpp_using_declarations(all_fields)
    if using_decls:
        content += '\n// Convenience aliases for nested types (no extra #include needed in user code)\n'
        content += '\n'.join(using_decls) + '\n'
    content += f"\nusing {z_name} = {package}::{z_name};\n"
    content += f"\n#endif // {guard}\n"
    return content

def generate_pc_header(package, name, fields, is_srv=False, req_fields=None, res_fields=None):
    z_name = f"z_{name}"
    guard = f"CUSTOM_INTERFACE_PC_{package.upper()}_{z_name.upper()}_H"
    
    # Process includes
    all_fields = fields if not is_srv else (req_fields + res_fields)
    includes = get_cpp_includes(all_fields, is_mcu=False, out_package=package)
    include_str = '\n'.join(includes)
    if include_str:
        include_str += '\n'
        
    content = f"""#ifndef {guard}
#define {guard}

#include <nlohmann/json.hpp>
#include <vector>
#include <string>
{include_str}
namespace {package} {{
"""
    if not is_srv:
        struct_fields = '\n'.join([f"    {get_cpp_type(ftype)} {fname};" for ftype, fname in fields])
        content += f"""struct {z_name} {{
{struct_fields}
}};
}} // namespace {package}

// --- Topic Serialization Override ---
template <>
inline std::vector<uint8_t> serialize_msg<{package}::{z_name}>(
    const {package}::{z_name}& msg) {{
    nlohmann::json j;
"""
        for ftype, fname in fields:
            content += f"    {generate_pc_cpp_field_serialization(ftype, fname, 'j', 'msg')}\n"
        content += f"""    return nlohmann::json::to_msgpack(j);
}}

// --- Topic Deserialization Override ---
template <>
inline void deserialize_msg<{package}::{z_name}>(
    const std::vector<uint8_t>& buffer, {package}::{z_name}& msg) {{
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
"""
        for ftype, fname in fields:
            content += f"    {generate_pc_cpp_field_deserialization(ftype, fname, 'j', 'msg')}\n"
        content += f"""}}
"""
    else:
        req_struct = '\n'.join([f"        {get_cpp_type(ftype)} {fname};" for ftype, fname in req_fields])
        res_struct = '\n'.join([f"        {get_cpp_type(ftype)} {fname};" for ftype, fname in res_fields])
        content += f"""struct {z_name} {{
    struct Request {{
{req_struct}
    }};

    struct Response {{
{res_struct}
    }};
}};
}} // namespace {package}

// --- Service Request Serializer ---
template <>
inline std::vector<uint8_t> serialize_msg<{package}::{z_name}::Request>(
    const {package}::{z_name}::Request& msg) {{
    nlohmann::json j;
"""
        for ftype, fname in req_fields:
            content += f"    {generate_pc_cpp_field_serialization(ftype, fname, 'j', 'msg')}\n"
        content += f"""    return nlohmann::json::to_msgpack(j);
}}

// --- Service Request Deserializer ---
template <>
inline void deserialize_msg<{package}::{z_name}::Request>(
    const std::vector<uint8_t>& buffer, {package}::{z_name}::Request& msg) {{
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
"""
        for ftype, fname in req_fields:
            content += f"    {generate_pc_cpp_field_deserialization(ftype, fname, 'j', 'msg')}\n"
        content += f"""}}

// --- Service Response Serializer ---
template <>
inline std::vector<uint8_t> serialize_msg<{package}::{z_name}::Response>(
    const {package}::{z_name}::Response& msg) {{
    nlohmann::json j;
"""
        for ftype, fname in res_fields:
            content += f"    {generate_pc_cpp_field_serialization(ftype, fname, 'j', 'msg')}\n"
        content += f"""    return nlohmann::json::to_msgpack(j);
}}

// --- Service Response Deserializer ---
template <>
inline void deserialize_msg<{package}::{z_name}::Response>(
    const std::vector<uint8_t>& buffer, {package}::{z_name}::Response& msg) {{
    nlohmann::json j = nlohmann::json::from_msgpack(buffer);
"""
        for ftype, fname in res_fields:
            content += f"    {generate_pc_cpp_field_deserialization(ftype, fname, 'j', 'msg')}\n"
        content += f"""}}
"""
    # Convenience using declarations so a single #include brings in all nested types
    using_decls = get_cpp_using_declarations(all_fields)
    if using_decls:
        content += '\n// Convenience aliases for nested types (no extra #include needed in user code)\n'
        content += '\n'.join(using_decls) + '\n'
    content += f"\nusing {z_name} = {package}::{z_name};\n"
    content += f"\n#endif // {guard}\n"
    return content

def generate_python_module(package, name, fields, is_srv=False, req_fields=None, res_fields=None):
    z_name = f"z_{name}"
    all_fields = fields if not is_srv else (req_fields + res_fields)
    # Detect all nested type imports needed
    nested_imports = get_python_package_imports(all_fields, out_package=package)
    import_str = '\n'.join(nested_imports)
    if import_str:
        import_str += '\n'

    content = f"""import msgpack
from typing import Any, List, Optional, cast
{import_str}"""
    if not is_srv:
        params = []
        inits = []
        reprs = []
        for ftype, fname in fields:
            py_type = get_python_type(ftype)
            default = get_python_default(ftype)
            params.append(f"{fname}: {py_type} = {default}")
            inits.append(f"        self.{fname} = {fname}")
            reprs.append(f"{fname}={{self.{fname}}}")
            
        param_str = ', '.join(params)
        init_str = '\n'.join(inits)
        repr_str = ', '.join(reprs)
        
        serialize_fields = ', '.join([generate_python_serialize_field(ftype, fname) for ftype, fname in fields])
        deserialize_fields = ', '.join([generate_python_deserialize_field(ftype, fname) for ftype, fname in fields])
        
        content += f"""class {z_name}:
    def __init__(self, {param_str}) -> None:
{init_str}

    def serialize(self) -> bytes:
        return cast(bytes, msgpack.packb({{{serialize_fields}}}))

    @classmethod
    def deserialize(cls, payload: bytes) -> '{z_name}':
        data = msgpack.unpackb(payload)
        # Handle empty/invalid payload
        if not isinstance(data, dict):
            data = {{}}
        return cls({deserialize_fields})

    def __repr__(self) -> str:
        return f"{package}.{z_name}({repr_str})"
"""
    else:
        req_params, req_inits, req_reprs = [], [], []
        for ftype, fname in req_fields:
            py_type = get_python_type(ftype)
            default = get_python_default(ftype)
            req_params.append(f"{fname}: {py_type} = {default}")
            req_inits.append(f"            self.{fname} = {fname}")
            req_reprs.append(f"{fname}={{self.{fname}}}")
            
        res_params, res_inits, res_reprs = [], [], []
        for ftype, fname in res_fields:
            py_type = get_python_type(ftype)
            default = get_python_default(ftype)
            res_params.append(f"{fname}: {py_type} = {default}")
            res_inits.append(f"            self.{fname} = {fname}")
            res_reprs.append(f"{fname}={{self.{fname}}}")
            
        req_serialize = ', '.join([generate_python_serialize_field(ftype, fname) for ftype, fname in req_fields])
        req_deserialize = ', '.join([generate_python_deserialize_field(ftype, fname) for ftype, fname in req_fields])
        
        res_serialize = ', '.join([generate_python_serialize_field(ftype, fname) for ftype, fname in res_fields])
        res_deserialize = ', '.join([generate_python_deserialize_field(ftype, fname) for ftype, fname in res_fields])

        content += f"""class {z_name}:
    class Request:
        def __init__(self, {', '.join(req_params)}) -> None:
{'\n'.join(req_inits)}

        def serialize(self) -> bytes:
            return cast(bytes, msgpack.packb({{{req_serialize}}}))

        @classmethod
        def deserialize(cls, payload: bytes) -> 'Request':
            data = msgpack.unpackb(payload)
            if not isinstance(data, dict):
                data = {{}}
            return cls({req_deserialize})

        def __repr__(self) -> str:
            return f"{package}.{z_name}.Request({', '.join(req_reprs)})"

    class Response:
        def __init__(self, {', '.join(res_params)}) -> None:
{'\n'.join(res_inits)}

        def serialize(self) -> bytes:
            return cast(bytes, msgpack.packb({{{res_serialize}}}))

        @classmethod
        def deserialize(cls, payload: bytes) -> 'Response':
            data = msgpack.unpackb(payload)
            if not isinstance(data, dict):
                data = {{}}
            return cls({res_deserialize})

        def __repr__(self) -> str:
            return f"{package}.{z_name}.Response({', '.join(res_reprs)})"
"""
    return content

def main():
    parser = argparse.ArgumentParser(description="zenoh_ros Interface Generator Script")
    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument('-generate', type=str, help="Generate target message/service files")
    group.add_argument('-rm', type=str, help="Remove generated message/service files")
    
    args = parser.parse_args()
    
    relative_path = args.generate if args.generate else args.rm
    action = 'generate' if args.generate else 'rm'
    
    # Locate paths
    script_dir = os.path.dirname(os.path.abspath(__file__))
    ws_root = os.path.abspath(os.path.join(script_dir, "../../../"))
    
    # Resolve the input file path relative to msg_interface folder
    input_file = os.path.abspath(os.path.join(script_dir, "../", relative_path))
    
    # Determine type, package, and name
    # e.g., srv/custom_srvs/SetColor.srv -> type=srv, package=custom_srvs, name=SetColor
    parts = relative_path.replace('\\', '/').split('/')
    if len(parts) < 2:
        print(f"Error: Invalid relative path layout {relative_path}. Expects '<msg|srv>/[package/]<filename>'")
        sys.exit(1)
        
    if parts[0] not in ['msg', 'srv']:
        print(f"Error: Path must start with 'msg/' or 'srv/'")
        sys.exit(1)
        
    interface_type = parts[0]
    filename = parts[-1]
    name, ext = os.path.splitext(filename)
    
    # Default package namespace if subdirectory is missing
    if len(parts) == 2:
        package = "custom_msgs" if interface_type == 'msg' else "custom_srvs"
    else:
        package = parts[1]
        
    # ---------------------------------------------------------------------------
    # Canonical output paths (inside msg_interface/)
    # ---------------------------------------------------------------------------
    out_dir_mcu = os.path.join(
        ws_root,
        f"shared_libraries/mcu/zenoh_ros/zenoh_ros/msg_interface/custom_interface/{interface_type}s/{package}"
    )
    out_dir_cpp = os.path.join(
        ws_root,
        f"shared_libraries/cpp/zenoh_ros/zenoh_ros/msg_interface/custom_interface/{interface_type}s/{package}"
    )
    out_dir_py = os.path.join(
        ws_root,
        f"shared_libraries/python/zenoh_ros/msg_interface/custom_interface/{interface_type}s/{package}"
    )

    mcu_file = os.path.join(out_dir_mcu, f"z_{name}.h")
    cpp_file = os.path.join(out_dir_cpp, f"z_{name}.h")
    py_file  = os.path.join(out_dir_py,  f"{name}.py")

    # ---------------------------------------------------------------------------
    # Forwarding header paths (old flat paths — kept for backward compatibility)
    # ---------------------------------------------------------------------------
    fwd_dir_mcu = os.path.join(ws_root, f"shared_libraries/mcu/zenoh_ros/zenoh_ros/{package}")
    fwd_dir_cpp = os.path.join(ws_root, f"shared_libraries/cpp/zenoh_ros/zenoh_ros/{package}")
    fwd_mcu_file = os.path.join(fwd_dir_mcu, f"z_{name}.h")
    fwd_cpp_file = os.path.join(fwd_dir_cpp, f"z_{name}.h")

    if action == 'rm':
        print(f"Removing generated files for {package}/{name}...")
        for f in [mcu_file, cpp_file, py_file, fwd_mcu_file, fwd_cpp_file]:
            if os.path.exists(f):
                os.remove(f)
                print(f"  Deleted: {f}")
            else:
                print(f"  Not found: {f}")
        # Clean up directories if they become empty
        for d in [out_dir_mcu, out_dir_cpp, out_dir_py, fwd_dir_mcu, fwd_dir_cpp]:
            if os.path.exists(d) and not os.listdir(d):
                os.rmdir(d)
                print(f"  Removed empty directory: {d}")
        sys.exit(0)

    # Generate action
    if not os.path.exists(input_file):
        print(f"Error: Input definition file not found: {input_file}")
        sys.exit(1)

    print(f"Reading definition from: {input_file}")
    with open(input_file, 'r') as f:
        content_lines = f.readlines()

    # Parse fields
    is_srv = (ext == '.srv')
    fields = []
    req_fields = []
    res_fields = []

    if not is_srv:
        fields = parse_fields(content_lines)
    else:
        # Split by divider '---'
        req_lines = []
        res_lines = []
        target = req_lines
        for line in content_lines:
            if line.strip() == '---':
                target = res_lines
                continue
            target.append(line)
        req_fields = parse_fields(req_lines)
        res_fields = parse_fields(res_lines)

    # Ensure canonical folders exist
    for d in [out_dir_mcu, out_dir_cpp, out_dir_py]:
        os.makedirs(d, exist_ok=True)
        if d == out_dir_py:
            init_file = os.path.join(d, "__init__.py")
            if not os.path.exists(init_file):
                with open(init_file, 'w') as init_f:
                    init_f.write("")

    # Ensure forwarding header directories exist
    for d in [fwd_dir_mcu, fwd_dir_cpp]:
        os.makedirs(d, exist_ok=True)

    # Generate content
    mcu_code = generate_mcu_header(package, name, fields, is_srv, req_fields, res_fields)
    cpp_code = generate_pc_header(package, name, fields, is_srv, req_fields, res_fields)
    py_code  = generate_python_module(package, name, fields, is_srv, req_fields, res_fields)

    # Build forwarding header content for MCU and PC C++
    mcu_canonical = f"zenoh_ros/msg_interface/custom_interface/{interface_type}s/{package}/z_{name}.h"
    cpp_canonical = f"zenoh_ros/msg_interface/custom_interface/{interface_type}s/{package}/z_{name}.h"
    fwd_template = (
        "// Forwarding header — do not edit directly.\n"
        "// Actual implementation: {canonical}\n"
        "#pragma once\n"
        "#include <{canonical}>\n"
    )
    fwd_mcu_code = fwd_template.format(canonical=mcu_canonical)
    fwd_cpp_code = fwd_template.format(canonical=cpp_canonical)

    # Save canonical files
    with open(mcu_file, 'w') as f:
        f.write(mcu_code)
    with open(cpp_file, 'w') as f:
        f.write(cpp_code)
    with open(py_file, 'w') as f:
        f.write(py_code)

    # Save forwarding headers at old flat paths
    with open(fwd_mcu_file, 'w') as f:
        f.write(fwd_mcu_code)
    with open(fwd_cpp_file, 'w') as f:
        f.write(fwd_cpp_code)

    print(f"Successfully generated interface files for {package}/{name}:")
    print(f"  MCU  (canonical): {mcu_file}")
    print(f"  MCU  (forwarding): {fwd_mcu_file}")
    print(f"  CPP  (canonical): {cpp_file}")
    print(f"  CPP  (forwarding): {fwd_cpp_file}")
    print(f"  Python: {py_file}")

if __name__ == "__main__":
    main()
