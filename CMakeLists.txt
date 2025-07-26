cmake_minimum_required(VERSION 3.16)
project(AutoDash-OS VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Multimedia MultimediaWidgets Network)
find_package(OpenCV REQUIRED)

# Set compiler flags for embedded development
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2 -g")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/ui)
include_directories(${CMAKE_SOURCE_DIR}/src/system)

# Source files
set(SOURCES
    src/main.cpp
    src/ui/MainWindow.cpp
    src/ui/MediaPlayer.cpp
    src/ui/BluetoothPanel.cpp
    src/ui/ClimateControl.cpp
    src/ui/CameraModule.cpp
    src/system/Logger.cpp
    src/system/MockI2C.cpp
    src/system/USBMonitor.cpp
    src/system/BluetoothSim.cpp
    src/system/ConfigManager.cpp
)

set(HEADERS
    src/ui/MainWindow.h
    src/ui/MediaPlayer.h
    src/ui/BluetoothPanel.h
    src/ui/ClimateControl.h
    src/ui/CameraModule.h
    src/system/Logger.h
    src/system/MockI2C.h
    src/system/USBMonitor.h
    src/system/BluetoothSim.h
    src/system/ConfigManager.h
)

# Create executable
add_executable(AutoDash-OS ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(AutoDash-OS
    Qt6::Core
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::Network
    ${OpenCV_LIBS}
)

# Copy assets to build directory
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

# Create log directory
add_custom_command(TARGET AutoDash-OS POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/var/log
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/config
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/mnt/usb
)

# Enable testing
enable_testing()
add_subdirectory(tests) 