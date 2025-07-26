# AutoDash OS - Embedded Infotainment System Simulator

## 🚗 Project Overview

AutoDash OS is a comprehensive embedded infotainment system simulator designed to demonstrate advanced embedded systems development skills. This project showcases expertise in C++, Qt, Linux embedded systems, and automotive infotainment technologies - exactly the skills required for the **Rivian & Volkswagen Group Technologies Software Infotainment Platform Engineer** position.

## 🎯 Skills Demonstrated

This project demonstrates proficiency in all technologies mentioned in the job posting:

### ✅ **Core Technologies**
- **C++** - Modern C++17 with advanced features (smart pointers, RAII, templates)
- **Qt6** - Comprehensive Qt application with multimedia, networking, and UI components
- **Linux Embedded Systems** - Simulated Linux-style embedded architecture
- **Embedded System Software** - Real-time sensor simulation and hardware abstraction

### ✅ **Bonus Skills**
- **Python** - Build scripts and automation tools
- **Low-level Interfaces** - I2C, SPI, GPIO, UART, USB, PCIe simulation
- **Debugging** - Comprehensive logging and error handling systems
- **AOSP Knowledge** - Android-style system architecture
- **Technical Areas** - Displays, cameras, graphics, power management, audio, Bluetooth

## 🏗️ Architecture

```
AutoDash-OS/
├── src/
│   ├── main.cpp                 # Application entry point with proper initialization
│   ├── ui/                      # Qt-based user interface modules
│   │   ├── MainWindow.cpp/.h    # Main automotive-style interface
│   │   ├── MediaPlayer.cpp/.h   # USB media playback with metadata
│   │   ├── BluetoothPanel.cpp/.h # Bluetooth device management
│   │   ├── ClimateControl.cpp/.h # I2C sensor-based climate control
│   │   └── CameraModule.cpp/.h  # OpenCV-based rear camera system
│   └── system/                  # Embedded system simulation
│       ├── Logger.cpp/.h        # Comprehensive logging system
│       ├── MockI2C.cpp/.h      # I2C sensor simulation
│       ├── USBMonitor.cpp/.h    # USB device monitoring
│       ├── BluetoothSim.cpp/.h  # Bluetooth stack simulation
│       └── ConfigManager.cpp/.h # System configuration management
├── assets/                      # Application resources
├── tests/                       # Unit tests
├── CMakeLists.txt              # Modern CMake build system
└── README.md                   # This file
```

## 🔧 Key Features

### 1. **Media Player Module**
- **USB Media Detection**: Real-time USB device monitoring with file system watching
- **Metadata Extraction**: Automatic song title, artist, album, and duration parsing
- **Playback Controls**: Play, pause, skip, volume control with ALSA integration
- **Playlist Management**: Dynamic playlist updates when USB devices are connected/disconnected

```cpp
// Example: USB device monitoring
USBMonitor& usbMonitor = USBMonitor::getInstance();
usbMonitor.simulateUSBInsertion("USB_DRIVE_01");
QList<MediaFile> files = usbMonitor.getMediaFiles(deviceId);
```

### 2. **Bluetooth Module**
- **Device Discovery**: Simulated BlueZ-style Bluetooth device discovery
- **Pairing Process**: Realistic pairing simulation with timeout handling
- **Profile Management**: A2DP, AVRCP, HFP, HSP profile support
- **Signal Strength**: Dynamic signal strength simulation

```cpp
// Example: Bluetooth device pairing
BluetoothSim& btSim = BluetoothSim::getInstance();
btSim.simulateDeviceAppearance("iPhone 15 Pro", BluetoothDeviceType::PHONE);
btSim.pairDevice(deviceId);
```

### 3. **Climate Control Module**
- **I2C Sensor Simulation**: Realistic temperature, humidity, pressure sensor data
- **Calibration System**: Sensor calibration with offset management
- **Error Simulation**: Connection errors, sensor failures, data corruption
- **Real-time Updates**: 5-second sensor data updates with logging

```cpp
// Example: I2C sensor reading
MockI2C& i2c = MockI2C::getInstance();
i2c.begin(0x48);
SensorData data = i2c.getCurrentData();
double temperature = data.temperature;
```

### 4. **Rear Camera Module**
- **OpenCV Integration**: Real webcam feed with computer vision processing
- **Reverse Mode**: Full-screen camera view with parking guidelines
- **Image Processing**: Brightness, contrast, saturation adjustments
- **Recording Capability**: Video capture and image capture functionality

### 5. **System Integration**
- **Comprehensive Logging**: Multi-level logging system with file and console output
- **Configuration Management**: JSON-based configuration with environment support
- **Error Handling**: Robust error handling with user-friendly error messages
- **Startup Sequence**: Realistic automotive startup sequence with splash screen

## 🛠️ Build System

### Prerequisites

#### **Windows**
1. **Visual Studio 2022** (Community Edition is free)
   - Download from: https://visualstudio.microsoft.com/downloads/
   - Install with "Desktop development with C++" workload

2. **Qt6**
   - Download Qt6 from: https://www.qt.io/download
   - Install Qt 6.5+ with Qt Creator
   - Set QTDIR environment variable to Qt installation path

3. **CMake**
   - Download from: https://cmake.org/download/
   - Add to PATH during installation

4. **OpenCV**
   - Download pre-built binaries from: https://opencv.org/releases/
   - Or install via vcpkg: `vcpkg install opencv`

#### **Linux (Ubuntu/Debian)**
```bash
sudo apt-get install build-essential cmake qt6-base-dev qt6-multimedia-dev libopencv-dev
```

#### **macOS**
```bash
brew install cmake qt6 opencv
```

### Building the Project

#### **Windows (PowerShell/Command Prompt)**
```powershell
# Run the Windows build script
.\build.bat

# Or build manually
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
.\AutoDash-OS.exe
```

#### **Linux/macOS**
```bash
# Run the Linux build script
./build.sh

# Or build manually
mkdir build && cd build
cmake ..
make -j$(nproc)
./AutoDash-OS
```

### Build Features
- **Modern CMake**: C++17 standard with Qt6 and OpenCV integration
- **Cross-platform**: Windows, macOS, and Linux support
- **Optimized Compilation**: -O2 optimization with debug symbols
- **Dependency Management**: Automatic Qt MOC, RCC, and UIC processing

## 🔍 Debugging & Development

### GDB Integration
The project includes comprehensive debugging support:

```bash
# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
gdb ./AutoDash-OS

# Example debugging session
(gdb) break MockI2C::readRegister
(gdb) run
(gdb) print reg
(gdb) continue
```

### Logging System
```cpp
// Multi-level logging with file output
LOG_DEBUG("MockI2C", "Reading register 0x00");
LOG_INFO("USBMonitor", "USB device connected: USB_DRIVE_01");
LOG_ERROR("BluetoothSim", "Connection failed - device not responding");
```

### Error Simulation
The system includes realistic error simulation for testing:

```cpp
// Simulate I2C connection error
mockI2C.simulateConnectionError(true);

// Simulate USB mount failure
usbMonitor.simulateMountError(true);

// Simulate Bluetooth pairing error
bluetoothSim.simulatePairingError(deviceId, true);
```

## 📊 Performance & Optimization

### Memory Management
- **Smart Pointers**: RAII-compliant resource management
- **Singleton Pattern**: Efficient system component management
- **Qt Memory Model**: Proper Qt parent-child relationships

### Real-time Performance
- **Timer-based Updates**: Efficient sensor data updates (5-second intervals)
- **Event-driven Architecture**: Qt signal-slot mechanism for responsive UI
- **Thread-safe Operations**: Mutex-protected logging and configuration

## 🧪 Testing & Validation

### Unit Tests
```cpp
// Example test for MockI2C
TEST_CASE("MockI2C sensor reading") {
    MockI2C& i2c = MockI2C::getInstance();
    REQUIRE(i2c.begin(0x48) == true);
    
    SensorData data = i2c.getCurrentData();
    REQUIRE(data.temperature >= -40.0);
    REQUIRE(data.temperature <= 80.0);
    REQUIRE(data.humidity >= 0.0);
    REQUIRE(data.humidity <= 100.0);
}
```

### Integration Testing
- **USB Device Simulation**: Complete USB device lifecycle testing
- **Bluetooth Pairing Flow**: End-to-end pairing process validation
- **Sensor Data Flow**: I2C sensor data processing pipeline
- **Configuration Persistence**: Settings save/load functionality

## 🚀 Deployment & Packaging

### Linux Package
```bash
# Create AppImage
linuxdeployqt AutoDash-OS -appimage

# Create Debian package
cpack -G DEB
```

### Windows Installer
```bash
# Create NSIS installer
cpack -G NSIS
```

## 📈 Future Enhancements

### Planned Features
- **CAN Bus Simulation**: Automotive CAN bus communication
- **GPS Integration**: Navigation system with real GPS data
- **Voice Recognition**: Speech-to-text for hands-free operation
- **OTA Updates**: Over-the-air firmware updates
- **Diagnostic Interface**: OBD-II diagnostic simulation

### Performance Improvements
- **Multi-threading**: Parallel sensor data processing
- **GPU Acceleration**: OpenGL-based graphics rendering
- **Memory Pooling**: Optimized memory allocation for embedded systems
- **Real-time Scheduling**: Priority-based task scheduling

## 🎓 Educational Value

This project demonstrates:

1. **Embedded Systems Design**: Real-time sensor simulation and hardware abstraction
2. **Automotive Software**: Infotainment system architecture and user experience
3. **C++ Best Practices**: Modern C++ features, RAII, and memory management
4. **Qt Framework**: Comprehensive Qt application development
5. **Linux Development**: Embedded Linux system programming
6. **Debugging Skills**: GDB integration and comprehensive logging
7. **Build Systems**: Modern CMake-based build configuration
8. **Testing**: Unit testing and integration testing strategies

## 🤝 Contributing

This project serves as a demonstration of embedded systems development skills. The codebase follows:

- **Google C++ Style Guide**: Consistent code formatting and naming conventions
- **Qt Coding Standards**: Qt-specific best practices
- **Embedded C++ Guidelines**: MISRA C++ compliance considerations
- **Documentation**: Comprehensive inline documentation and comments

## 📄 License

This project is created for educational and demonstration purposes to showcase embedded systems development skills for the Rivian & Volkswagen Group Technologies position.

---

**Built with ❤️ for the future of automotive software**

*This project demonstrates the exact skills required for embedded infotainment platform development at leading automotive technology companies.*
