#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QStyleFactory>
#include <QFontDatabase>
#include <QSplashScreen>
#include <QPixmap>
#include <QTimer>
#include <QDebug>

#include "ui/MainWindow.h"
#include "system/Logger.h"
#include "system/MockI2C.h"
#include "system/USBMonitor.h"
#include "system/BluetoothSim.h"
#include "system/ConfigManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("AutoDash OS");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Rivian & Volkswagen Group Technologies");
    app.setOrganizationDomain("autodash.com");
    
    // Parse command line arguments
    QCommandLineParser parser;
    parser.setApplicationDescription("AutoDash OS - Embedded Infotainment System Simulator");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption debugOption(QStringList() << "d" << "debug", "Enable debug mode");
    parser.addOption(debugOption);
    
    QCommandLineOption logLevelOption(QStringList() << "l" << "log-level", 
                                     "Set log level (DEBUG, INFO, WARNING, ERROR, CRITICAL)", 
                                     "level", "INFO");
    parser.addOption(logLevelOption);
    
    QCommandLineOption configOption(QStringList() << "c" << "config", 
                                   "Configuration file path", "file");
    parser.addOption(configOption);
    
    QCommandLineOption themeOption(QStringList() << "t" << "theme", 
                                  "Application theme (light, dark, auto)", "theme", "auto");
    parser.addOption(themeOption);
    
    parser.process(app);
    
    // Create necessary directories
    QStringList directories = {
        "config",
        "logs", 
        "mnt/usb",
        "media/usb",
        "tmp/usb",
        "assets/icons",
        "assets/dummy_usb_files"
    };
    
    for (const QString& dir : directories) {
        QDir().mkpath(dir);
    }
    
    // Initialize logger
    Logger& logger = Logger::getInstance();
    
    // Set log level from command line
    QString logLevel = parser.value(logLevelOption);
    if (logLevel == "DEBUG") {
        logger.setLogLevel(LogLevel::DEBUG);
    } else if (logLevel == "WARNING") {
        logger.setLogLevel(LogLevel::WARNING);
    } else if (logLevel == "ERROR") {
        logger.setLogLevel(LogLevel::ERROR);
    } else if (logLevel == "CRITICAL") {
        logger.setLogLevel(LogLevel::CRITICAL);
    } else {
        logger.setLogLevel(LogLevel::INFO);
    }
    
    // Enable debug mode if requested
    if (parser.isSet(debugOption)) {
        logger.setConsoleOutput(true);
        LOG_INFO("Main", "Debug mode enabled");
    }
    
    LOG_INFO("Main", "AutoDash OS starting up...");
    LOG_INFO("Main", QString("Application version: %1").arg(app.applicationVersion()));
    LOG_INFO("Main", QString("Qt version: %1").arg(QT_VERSION_STR));
    
    // Initialize configuration manager
    ConfigManager& configManager = ConfigManager::getInstance();
    if (!configManager.loadConfiguration()) {
        LOG_WARNING("Main", "Failed to load configuration, using defaults");
    }
    
    // Set application theme
    QString theme = parser.value(themeOption);
    if (theme == "light") {
        app.setStyle(QStyleFactory::create("Fusion"));
        app.setPalette(app.style()->standardPalette());
    } else if (theme == "dark") {
        app.setStyle(QStyleFactory::create("Fusion"));
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);
        app.setPalette(darkPalette);
    }
    
    // Load custom fonts if available
    QFontDatabase::addApplicationFont("assets/fonts/Roboto-Regular.ttf");
    QFontDatabase::addApplicationFont("assets/fonts/Roboto-Bold.ttf");
    
    // Create splash screen
    QPixmap splashPixmap(400, 300);
    splashPixmap.fill(Qt::black);
    
    QSplashScreen splash(splashPixmap);
    splash.show();
    
    // Initialize system components
    LOG_INFO("Main", "Initializing system components...");
    
    // Initialize Mock I2C
    MockI2C& mockI2C = MockI2C::getInstance();
    if (!mockI2C.begin(0x48)) {
        LOG_ERROR("Main", "Failed to initialize Mock I2C");
    } else {
        LOG_INFO("Main", "Mock I2C initialized successfully");
    }
    
    // Initialize USB Monitor
    USBMonitor& usbMonitor = USBMonitor::getInstance();
    usbMonitor.startMonitoring();
    LOG_INFO("Main", "USB Monitor initialized");
    
    // Initialize Bluetooth Simulator
    BluetoothSim& bluetoothSim = BluetoothSim::getInstance();
    if (!bluetoothSim.initialize()) {
        LOG_ERROR("Main", "Failed to initialize Bluetooth simulator");
    } else {
        LOG_INFO("Main", "Bluetooth simulator initialized");
    }
    
    // Update splash screen
    splash.showMessage("Initializing UI components...", Qt::AlignCenter | Qt::AlignBottom, Qt::white);
    app.processEvents();
    
    // Create and show main window
    MainWindow mainWindow;
    
    // Simulate some startup delay for realistic feel
    QTimer::singleShot(2000, [&]() {
        splash.showMessage("Starting infotainment system...", Qt::AlignCenter | Qt::AlignBottom, Qt::white);
        app.processEvents();
        
        QTimer::singleShot(1000, [&]() {
            splash.showMessage("System ready!", Qt::AlignCenter | Qt::AlignBottom, Qt::white);
            app.processEvents();
            
            QTimer::singleShot(500, [&]() {
                splash.finish(&mainWindow);
                mainWindow.show();
                
                LOG_INFO("Main", "AutoDash OS startup complete");
                
                // Simulate some initial system state
                usbMonitor.simulateUSBInsertion("USB_DRIVE_01");
                bluetoothSim.simulateDeviceAppearance("iPhone 15 Pro", BluetoothDeviceType::PHONE);
                bluetoothSim.simulateDeviceAppearance("Sony WH-1000XM5", BluetoothDeviceType::HEADSET);
            });
        });
    });
    
    // Set up application-wide error handling
    auto handleError = [&](const QString& error) {
        LOG_ERROR("Main", QString("Application error: %1").arg(error));
        QMessageBox::critical(&mainWindow, "AutoDash OS Error", error);
    };
    
    // Connect error signals
    QObject::connect(&mockI2C, &MockI2C::connectionError, 
                     [&](const QString& error) { handleError("I2C Error: " + error); });
    QObject::connect(&mockI2C, &MockI2C::sensorError, 
                     [&](const QString& error) { handleError("Sensor Error: " + error); });
    QObject::connect(&usbMonitor, &USBMonitor::mountError, 
                     [&](const QString& deviceId, const QString& error) { 
                         handleError("USB Mount Error: " + error); 
                     });
    QObject::connect(&bluetoothSim, &BluetoothSim::connectionError, 
                     [&](const QString& deviceId, const QString& error) { 
                         handleError("Bluetooth Error: " + error); 
                     });
    QObject::connect(&bluetoothSim, &BluetoothSim::pairingError, 
                     [&](const QString& deviceId, const QString& error) { 
                         handleError("Bluetooth Pairing Error: " + error); 
                     });
    
    // Handle application shutdown
    QObject::connect(&app, &QApplication::aboutToQuit, [&]() {
        LOG_INFO("Main", "AutoDash OS shutting down...");
        
        // Save configuration
        configManager.saveConfiguration();
        
        // Stop monitoring
        usbMonitor.stopMonitoring();
        
        LOG_INFO("Main", "AutoDash OS shutdown complete");
    });
    
    // Start the event loop
    int result = app.exec();
    
    LOG_INFO("Main", QString("AutoDash OS exited with code %1").arg(result));
    return result;
} 