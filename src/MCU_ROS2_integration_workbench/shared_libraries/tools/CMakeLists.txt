cmake_minimum_required(VERSION 3.10)
project(zenoh_ros_tools VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LOCAL_3RDPARTY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../cpp/3rdparty")

if(EXISTS "${LOCAL_3RDPARTY_DIR}/nlohmann/json.hpp")
    include_directories(SYSTEM "${LOCAL_3RDPARTY_DIR}")
else()
    find_package(nlohmann_json REQUIRED)
endif()

if(EXISTS "${LOCAL_3RDPARTY_DIR}/zenoh-c")
    include_directories(SYSTEM "${LOCAL_3RDPARTY_DIR}/zenoh-c/include")
    find_library(ZENOHC_LIB NAMES zenohc PATHS "${LOCAL_3RDPARTY_DIR}/zenoh-c/lib" NO_DEFAULT_PATH REQUIRED)
else()
    find_library(ZENOHC_LIB zenohc REQUIRED)
endif()

add_executable(echo src/echo.cpp)

target_include_directories(echo PRIVATE ../cpp/zenoh_ros)
target_link_libraries(echo PRIVATE ${ZENOHC_LIB})

if(TARGET nlohmann_json::nlohmann_json)
    target_link_libraries(echo PRIVATE nlohmann_json::nlohmann_json)
endif()

if(EXISTS "${LOCAL_3RDPARTY_DIR}/zenoh-c")
    if(APPLE)
        set_target_properties(echo PROPERTIES LINK_FLAGS "-Wl,-rpath,@loader_path/../cpp/3rdparty/zenoh-c/lib")
    else()
        set_target_properties(echo PROPERTIES LINK_FLAGS "-Wl,-rpath,\$ORIGIN/../cpp/3rdparty/zenoh-c/lib")
    endif()
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()
