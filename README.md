# AutoDash OS - Embedded Infotainment System Simulator

## Project Overview

AutoDash OS is a modular infotainment system simulator designed to showcase embedded systems and automotive software engineering skills. The project is implemented in modern C++ (C++17) using Qt6 and simulates the core features found in real-world automotive infotainment platforms. It is intended as a demonstration of proficiency in C++, Qt, embedded Linux concepts, and system integration for roles such as the Rivian & Volkswagen Group Technologies Software Infotainment Platform Engineer.

## Technologies Used

- **C++17**: Modern language features, smart pointers, RAII
- **Qt6**: Core, Widgets, Multimedia, MultimediaWidgets, Network
- **OpenCV**: Camera integration
- **Linux Embedded Concepts**: Simulated environment, file system structure, process management
- **Low-level Interfaces**: I2C, USB, Bluetooth (BlueZ concepts), ALSA (audio)
- **Build System**: CMake for cross-platform compilation
- **Testing**: Catch2 for unit tests

## Project Structure

```
AutoDash-OS/
├── src/
│   ├── main.cpp
│   ├── ui/
│   │   ├── MainWindow.cpp/.h
│   │   ├── MediaPlayer.cpp/.h
│   │   ├── BluetoothPanel.cpp/.h
│   │   ├── ClimateControl.cpp/.h
│   │   └── CameraModule.cpp/.h
│   └── system/
│       ├── Logger.cpp/.h
│       ├── MockI2C.cpp/.h
│       ├── USBMonitor.cpp/.h
│       ├── BluetoothSim.cpp/.h
│       └── ConfigManager.cpp/.h
├── assets/
├── tests/
├── CMakeLists.txt
└── README.md
```

## Features

### Media Player
- Scans a simulated `/mnt/usb/` directory for `.mp3` and `.wav` files
- Displays song lists with metadata (title, artist, album, duration)
- Supports play, pause, skip, and volume control (QMediaPlayer or ALSA)
- Handles "No USB detected" state and updates UI dynamically
- Shows playback progress and elapsed time

### Bluetooth Module
- Simulates Bluetooth device discovery using BlueZ concepts
- Allows pairing, connecting, and unpairing of mock devices
- Stores paired devices in configuration for persistence
- Simulates connection states (searching, connecting, connected)

### Climate Control
- Simulates I2C sensor readings (temperature, humidity) via a `MockI2C` class
- Updates sensor data every 5 seconds
- Allows user to set target temperature; displays heating/cooling/idle status
- Saves and loads preferred climate settings

### Rear Camera
- Displays a live webcam feed using OpenCV and Qt
- Includes a "Reverse Mode" toggle for full-screen view with grid/guide overlays

### System Integration
- Watches the simulated USB folder using QFileSystemWatcher for real-time updates
- Implements a thread-safe logger (console and `/var/log/autodash.log`), with a UI log viewer
- Stores settings (paired device, climate, last song) in JSON config files
- Loads saved config and resumes last state on startup
- Simulates runtime bugs for GDB demonstration (see Debugging section)

## Building and Running

### Prerequisites

#### Windows
- Visual Studio 2022 (with "Desktop development with C++" workload)
- Qt6 (install via Qt Creator, set `QTDIR` environment variable)
- CMake (add to PATH)
- OpenCV (pre-built binaries or via vcpkg)

#### Linux (Ubuntu/Debian)
```
sudo apt-get install build-essential cmake qt6-base-dev qt6-multimedia-dev libopencv-dev
```

#### macOS
```
brew install cmake qt6 opencv
```

### Build Instructions

#### Windows (PowerShell/Command Prompt)
```
# Run the build script
dir build.bat

# Or build manually
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
./AutoDash-OS.exe
```

#### Linux/macOS
```
# Run the build script
./build.sh

# Or build manually
mkdir build && cd build
cmake ..
make -j$(nproc)
./AutoDash-OS
```

### Build System Features
- Modern CMake with Qt6 and OpenCV integration
- Cross-platform: Windows, macOS, Linux
- Optimized compilation with debug symbols
- Automatic Qt MOC, RCC, and UIC processing

## Debugging and Development

### GDB Integration
The project is designed for easy debugging with GDB:

```
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
gdb ./AutoDash-OS
```

Example session:
```
(gdb) break MockI2C::readRegister
(gdb) run
(gdb) print reg
(gdb) continue
```

### Logging
The logger supports multiple levels and outputs to both file and console:
```cpp
LOG_DEBUG("MockI2C", "Reading register 0x00");
LOG_INFO("USBMonitor", "USB device connected: USB_DRIVE_01");
LOG_ERROR("BluetoothSim", "Connection failed - device not responding");
```

### Error Simulation
You can simulate hardware and connection errors for testing:
```cpp
mockI2C.simulateConnectionError(true);
usbMonitor.simulateMountError(true);
bluetoothSim.simulatePairingError(deviceId, true);
```

## Performance and Optimization
- Uses smart pointers and RAII for memory management
- Singleton pattern for core system components
- Qt parent-child memory model
- Timer-based updates for real-time sensor data
- Thread-safe logging and configuration

## Testing

### Unit Tests
Unit tests are implemented with Catch2. Example:
```cpp
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
- USB device simulation
- Bluetooth pairing flow
- Sensor data processing
- Configuration persistence

## Packaging and Deployment

### Linux
- AppImage: `linuxdeployqt AutoDash-OS -appimage`
- Debian package: `cpack -G DEB`

### Windows
- NSIS installer: `cpack -G NSIS`

## Future Enhancements
- CAN bus simulation
- GPS integration
- Voice recognition
- OTA updates
- OBD-II diagnostic interface
- Multi-threading and GPU acceleration

## Educational Value
This project demonstrates:
- Embedded systems design and hardware abstraction
- Automotive infotainment architecture
- Modern C++ and Qt best practices
- Linux embedded development
- Debugging and logging
- Build systems and testing

## Contributing

This repository is intended as a demonstration of embedded systems and automotive software engineering skills. The codebase follows:
- Google C++ Style Guide
- Qt coding standards
- MISRA C++ compliance considerations
- Comprehensive inline documentation

## License

This project is for educational and demonstration purposes, created to showcase embedded systems development for automotive infotainment roles.

---

*For questions or feedback, please open an issue or contact the repository owner.*
