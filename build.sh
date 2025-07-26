#!/bin/bash

# AutoDash OS Build Script
# This script demonstrates the complete build process for the embedded infotainment system

set -e  # Exit on any error

echo "🚗 AutoDash OS - Embedded Infotainment System Builder"
echo "=================================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "CMakeLists.txt" ]; then
    print_error "CMakeLists.txt not found. Please run this script from the project root."
    exit 1
fi

print_status "Starting AutoDash OS build process..."

# Check system requirements
print_status "Checking system requirements..."

# Check for required tools
command -v cmake >/dev/null 2>&1 || { print_error "cmake is required but not installed. Aborting."; exit 1; }
command -v make >/dev/null 2>&1 || { print_error "make is required but not installed. Aborting."; exit 1; }
command -v g++ >/dev/null 2>&1 || { print_error "g++ is required but not installed. Aborting."; exit 1; }

print_success "Basic build tools found"

# Check for Qt6
if ! pkg-config --exists Qt6Core; then
    print_warning "Qt6 not found via pkg-config. Checking alternative locations..."
    if [ -d "/usr/include/qt6" ] || [ -d "/opt/qt6" ]; then
        print_success "Qt6 headers found"
    else
        print_error "Qt6 not found. Please install Qt6 development packages."
        echo "Ubuntu/Debian: sudo apt-get install qt6-base-dev qt6-multimedia-dev"
        echo "macOS: brew install qt6"
        exit 1
    fi
else
    print_success "Qt6 found via pkg-config"
fi

# Check for OpenCV
if ! pkg-config --exists opencv4; then
    print_warning "OpenCV not found via pkg-config. Checking alternative locations..."
    if [ -d "/usr/include/opencv4" ] || [ -d "/usr/local/include/opencv4" ]; then
        print_success "OpenCV headers found"
    else
        print_error "OpenCV not found. Please install OpenCV development packages."
        echo "Ubuntu/Debian: sudo apt-get install libopencv-dev"
        echo "macOS: brew install opencv"
        exit 1
    fi
else
    print_success "OpenCV found via pkg-config"
fi

# Create build directory
print_status "Creating build directory..."
rm -rf build
mkdir -p build
cd build

# Configure with CMake
print_status "Configuring project with CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_CXX_FLAGS="-Wall -Wextra -O2" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

if [ $? -eq 0 ]; then
    print_success "CMake configuration completed"
else
    print_error "CMake configuration failed"
    exit 1
fi

# Build the project
print_status "Building AutoDash OS..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    print_success "Build completed successfully"
else
    print_error "Build failed"
    exit 1
fi

# Run tests if Catch2 is available
if pkg-config --exists Catch2; then
    print_status "Running unit tests..."
    if [ -f "AutoDash-Tests" ]; then
        ./AutoDash-Tests
        if [ $? -eq 0 ]; then
            print_success "All tests passed"
        else
            print_warning "Some tests failed"
        fi
    else
        print_warning "Test executable not found"
    fi
else
    print_warning "Catch2 not found, skipping tests"
fi

# Create necessary directories for runtime
print_status "Setting up runtime directories..."
mkdir -p config logs mnt/usb media/usb tmp/usb assets/icons assets/dummy_usb_files

# Copy assets
print_status "Copying assets..."
cp -r ../assets/* assets/ 2>/dev/null || true

# Create sample configuration
print_status "Creating sample configuration..."
cat > config/autodash_config.json << EOF
{
    "general": {
        "app_name": "AutoDash OS",
        "app_version": "1.0.0",
        "debug_enabled": true,
        "log_level": "INFO"
    },
    "user_settings": {
        "volumeLevel": 50,
        "preferredTemperature": 22.0,
        "preferredHumidity": 50.0,
        "brightnessLevel": 80,
        "theme": "auto",
        "language": "en_US",
        "timezone": "UTC"
    },
    "environments": {
        "development": {
            "debug_mode": true,
            "log_level": "DEBUG"
        },
        "production": {
            "debug_mode": false,
            "log_level": "INFO"
        }
    },
    "last_modified": "$(date -Iseconds)",
    "version": "1.0.0",
    "environment": "development"
}
EOF

print_success "Configuration file created"

# Check if executable was created
if [ -f "AutoDash-OS" ]; then
    print_success "AutoDash OS executable created successfully"
    
    # Display build information
    echo ""
    echo "🎉 Build Summary:"
    echo "================="
    echo "✅ Executable: AutoDash-OS"
    echo "✅ Build Type: Release"
    echo "✅ C++ Standard: C++17"
    echo "✅ Optimization: -O2"
    echo "✅ Qt Version: $(pkg-config --modversion Qt6Core 2>/dev/null || echo 'Unknown')"
    echo "✅ OpenCV Version: $(pkg-config --modversion opencv4 2>/dev/null || echo 'Unknown')"
    echo ""
    echo "🚀 To run AutoDash OS:"
    echo "   cd build"
    echo "   ./AutoDash-OS"
    echo ""
    echo "🔧 Available command line options:"
    echo "   ./AutoDash-OS --help"
    echo "   ./AutoDash-OS --debug"
    echo "   ./AutoDash-OS --log-level DEBUG"
    echo "   ./AutoDash-OS --theme dark"
    echo ""
    echo "📁 Project structure created:"
    echo "   config/     - Configuration files"
    echo "   logs/       - Log files"
    echo "   mnt/usb/    - USB mount points"
    echo "   assets/     - Application resources"
    echo ""
    
    # Optional: Run the application
    read -p "Would you like to run AutoDash OS now? (y/N): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        print_status "Starting AutoDash OS..."
        ./AutoDash-OS
    fi
    
else
    print_error "Executable not found after build"
    exit 1
fi

print_success "AutoDash OS build process completed successfully!"
echo ""
echo "🎓 This project demonstrates:"
echo "   • Modern C++17 development"
echo "   • Qt6 application architecture"
echo "   • Embedded systems simulation"
echo "   • Automotive infotainment design"
echo "   • Real-time sensor data processing"
echo "   • USB device management"
echo "   • Bluetooth device simulation"
echo "   • OpenCV computer vision integration"
echo "   • Comprehensive logging and debugging"
echo "   • Professional build system configuration"
echo ""
echo "💼 Perfect for embedded infotainment platform development!" 