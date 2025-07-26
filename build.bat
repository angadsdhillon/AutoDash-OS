@echo off
setlocal enabledelayedexpansion

echo.
echo ========================================
echo    AutoDash-OS Build Script (Windows)
echo ========================================
echo.

:: Check for required tools
echo [INFO] Checking for required tools...

:: Check for CMake
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo [ERROR] CMake not found! Please install CMake from https://cmake.org/download/
    echo [INFO] Make sure to add CMake to your PATH during installation.
    pause
    exit /b 1
)

:: Check for Visual Studio or MinGW
where cl >nul 2>&1
if %errorlevel% equ 0 (
    echo [INFO] Found Visual Studio compiler
    set "COMPILER=msvc"
) else (
    where g++ >nul 2>&1
    if %errorlevel% equ 0 (
        echo [INFO] Found MinGW compiler
        set "COMPILER=mingw"
    ) else (
        echo [ERROR] No C++ compiler found! Please install Visual Studio or MinGW.
        pause
        exit /b 1
    )
)

:: Check for Qt6
echo [INFO] Checking for Qt6...
if not exist "%QTDIR%" (
    echo [WARNING] QTDIR environment variable not set. Trying to find Qt6...
    for /f "tokens=*" %%i in ('where qmake 2^>nul') do (
        set "QMAKE_PATH=%%i"
        goto :found_qmake
    )
    echo [ERROR] Qt6 not found! Please install Qt6 and set QTDIR environment variable.
    echo [INFO] Download from: https://www.qt.io/download
    pause
    exit /b 1
) else (
    echo [INFO] Found Qt6 at %QTDIR%
)

:found_qmake
if defined QMAKE_PATH (
    echo [INFO] Found qmake at !QMAKE_PATH!
)

:: Check for OpenCV
echo [INFO] Checking for OpenCV...
if not exist "%OpenCV_DIR%" (
    echo [WARNING] OpenCV_DIR not set. Will try to find OpenCV automatically...
)

:: Create build directory
echo.
echo [INFO] Creating build directory...
if exist build (
    echo [INFO] Build directory already exists
) else (
    mkdir build
)

:: Navigate to build directory
cd build

:: Configure with CMake
echo.
echo [INFO] Configuring project with CMake...
if "%COMPILER%"=="msvc" (
    cmake .. -G "Visual Studio 17 2022" -A x64
) else (
    cmake .. -G "MinGW Makefiles"
)

if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed!
    pause
    exit /b 1
)

:: Build the project
echo.
echo [INFO] Building project...
if "%COMPILER%"=="msvc" (
    cmake --build . --config Release
) else (
    cmake --build . -j %NUMBER_OF_PROCESSORS%
)

if %errorlevel% neq 0 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

:: Create runtime directories
echo.
echo [INFO] Creating runtime directories...
if not exist "var\log" mkdir "var\log"
if not exist "config" mkdir "config"
if not exist "mnt\usb" mkdir "mnt\usb"

:: Copy assets
echo [INFO] Copying assets...
if exist "..\assets" (
    xcopy "..\assets" "assets" /E /I /Y >nul
)

:: Create sample config
echo [INFO] Creating sample configuration...
if not exist "config\autodash_config.json" (
    echo {> config\autodash_config.json
    echo   "media": {>> config\autodash_config.json
    echo     "lastPlayed": "",>> config\autodash_config.json
    echo     "volume": 50,>> config\autodash_config.json
    echo     "repeat": false,>> config\autodash_config.json
    echo     "shuffle": false>> config\autodash_config.json
    echo   },>> config\autodash_config.json
    echo   "climate": {>> config\autodash_config.json
    echo     "targetTemperature": 22.0,>> config\autodash_config.json
    echo     "fanSpeed": 3,>> config\autodash_config.json
    echo     "mode": "auto">> config\autodash_config.json
    echo   },>> config\autodash_config.json
    echo   "bluetooth": {>> config\autodash_config.json
    echo     "lastPairedDevice": "",>> config\autodash_config.json
    echo     "autoConnect": true>> config\autodash_config.json
    echo   },>> config\autodash_config.json
    echo   "system": {>> config\autodash_config.json
    echo     "theme": "dark",>> config\autodash_config.json
    echo     "logLevel": "info">> config\autodash_config.json
    echo   }>> config\autodash_config.json
    echo }>> config\autodash_config.json
)

:: Run tests if available
echo.
echo [INFO] Running tests...
if exist "AutoDash-Tests.exe" (
    echo [INFO] Running unit tests...
    AutoDash-Tests.exe
    if %errorlevel% neq 0 (
        echo [WARNING] Some tests failed, but continuing...
    )
) else (
    echo [INFO] No test executable found, skipping tests...
)

:: Build summary
echo.
echo ========================================
echo           BUILD SUMMARY
echo ========================================
echo.
echo [SUCCESS] AutoDash-OS has been built successfully!
echo.
echo [INFO] Executable location: build\AutoDash-OS.exe
echo [INFO] Configuration files: build\config\
echo [INFO] Log files: build\var\log\
echo [INFO] USB mount point: build\mnt\usb\
echo.
echo [INFO] To run the application:
echo [INFO]   cd build
echo [INFO]   AutoDash-OS.exe
echo.
echo [INFO] Or run directly from project root:
echo [INFO]   build\AutoDash-OS.exe
echo.

:: Ask if user wants to run the application
set /p "run_app=Do you want to run AutoDash-OS now? (y/n): "
if /i "!run_app!"=="y" (
    echo.
    echo [INFO] Starting AutoDash-OS...
    if exist "AutoDash-OS.exe" (
        start AutoDash-OS.exe
    ) else (
        echo [ERROR] AutoDash-OS.exe not found in build directory!
    )
)

echo.
echo [INFO] Build script completed successfully!
pause 