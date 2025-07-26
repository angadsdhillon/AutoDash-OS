#include "MainWindow.h"
#include "Logger.h"
#include <QCloseEvent>
#include <QResizeEvent>
#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QFontMetrics>
#include <QScreen>
#include <QApplication>
#include <QDesktopWidget>
#include <QStyleFactory>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QFile>
#include <QTextStream>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_tabWidget(nullptr)
    , m_sidebar(nullptr)
    , m_centralWidget(nullptr)
    , m_statusBar(nullptr)
    , m_systemTray(nullptr)
    , m_mediaPlayer(nullptr)
    , m_bluetoothPanel(nullptr)
    , m_climateControl(nullptr)
    , m_cameraModule(nullptr)
    , m_timeLabel(nullptr)
    , m_dateLabel(nullptr)
    , m_temperatureLabel(nullptr)
    , m_connectionStatusLabel(nullptr)
    , m_batteryProgressBar(nullptr)
    , m_signalStrengthLabel(nullptr)
    , m_statusUpdateTimer(nullptr)
    , m_animationTimer(nullptr)
    , m_sidebarAnimation(nullptr)
    , m_sidebarOpacity(nullptr)
    , m_isFullscreen(false)
    , m_isMinimizedToTray(false)
{
    setWindowTitle("AutoDash OS - Embedded Infotainment System");
    setWindowIcon(QIcon("assets/icons/autodash.ico"));
    
    // Set window properties
    setMinimumSize(800, 600);
    resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // Center window on screen
    QScreen *screen = QApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->geometry();
        int x = (screenGeometry.width() - WINDOW_WIDTH) / 2;
        int y = (screenGeometry.height() - WINDOW_HEIGHT) / 2;
        move(x, y);
    }
    
    // Setup UI components
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupSystemTray();
    setupCentralWidget();
    setupTabWidget();
    setupSidebar();
    setupAnimations();
    
    // Load stylesheet
    loadStylesheet();
    
    // Restore window state
    restoreWindowState();
    
    // Create timers
    m_statusUpdateTimer = new QTimer(this);
    connect(m_statusUpdateTimer, &QTimer::timeout, this, &MainWindow::updateStatusBar);
    m_statusUpdateTimer->start(STATUS_UPDATE_INTERVAL);
    
    m_animationTimer = new QTimer(this);
    connect(m_animationTimer, &QTimer::timeout, this, [this]() {
        update();
    });
    m_animationTimer->start(50); // 20 FPS for smooth animations
    
    // Create dummy data for demonstration
    createDummyData();
    
    LOG_INFO("MainWindow", "Main window initialized");
}

MainWindow::~MainWindow()
{
    saveWindowState();
    LOG_INFO("MainWindow", "Main window destroyed");
}

void MainWindow::setupUI()
{
    // Set window flags for automotive display
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    
    // Enable mouse tracking for hover effects
    setMouseTracking(true);
    
    // Set window background
    setStyleSheet("QMainWindow { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
                  "stop:0 #1a1a1a, stop:1 #2d2d2d); }");
}

void MainWindow::setupMenuBar()
{
    QMenuBar *menuBar = this->menuBar();
    menuBar->setStyleSheet("QMenuBar { background-color: #2d2d2d; color: white; "
                           "border-bottom: 1px solid #404040; }"
                           "QMenuBar::item { padding: 8px 12px; }"
                           "QMenuBar::item:selected { background-color: #404040; }");
    
    // File menu
    QMenu *fileMenu = menuBar->addMenu("&File");
    fileMenu->addAction("&Settings", this, &MainWindow::showSettingsDialog, QKeySequence("Ctrl+,"));
    fileMenu->addSeparator();
    fileMenu->addAction("&Exit", this, &QWidget::close, QKeySequence("Ctrl+Q"));
    
    // View menu
    QMenu *viewMenu = menuBar->addMenu("&View");
    viewMenu->addAction("&Fullscreen", this, &MainWindow::toggleFullscreen, QKeySequence("F11"));
    viewMenu->addAction("&Minimize to Tray", this, &MainWindow::minimizeToTray);
    
    // Tools menu
    QMenu *toolsMenu = menuBar->addMenu("&Tools");
    toolsMenu->addAction("&System Info", this, &MainWindow::showSystemInfo);
    toolsMenu->addAction("&Log Viewer", this, &MainWindow::showLogViewer);
    toolsMenu->addAction("&Debug Panel", this, &MainWindow::showDebugPanel);
    
    // Help menu
    QMenu *helpMenu = menuBar->addMenu("&Help");
    helpMenu->addAction("&About", this, &MainWindow::showAboutDialog);
}

void MainWindow::setupToolBar()
{
    QToolBar *toolBar = addToolBar("Main Toolbar");
    toolBar->setMovable(false);
    toolBar->setStyleSheet("QToolBar { background-color: #2d2d2d; border: none; }"
                           "QToolButton { background-color: transparent; border: none; "
                           "color: white; padding: 8px; }"
                           "QToolButton:hover { background-color: #404040; }");
    
    // Add toolbar actions
    toolBar->addAction("Home", this, [this]() { m_tabWidget->setCurrentIndex(0); });
    toolBar->addAction("Media", this, [this]() { m_tabWidget->setCurrentIndex(1); });
    toolBar->addAction("Bluetooth", this, [this]() { m_tabWidget->setCurrentIndex(2); });
    toolBar->addAction("Climate", this, [this]() { m_tabWidget->setCurrentIndex(3); });
    toolBar->addAction("Camera", this, [this]() { m_tabWidget->setCurrentIndex(4); });
}

void MainWindow::setupStatusBar()
{
    m_statusBar = statusBar();
    m_statusBar->setStyleSheet("QStatusBar { background-color: #1a1a1a; color: white; "
                               "border-top: 1px solid #404040; }");
    
    // Create status widgets
    m_timeLabel = new QLabel(this);
    m_dateLabel = new QLabel(this);
    m_temperatureLabel = new QLabel(this);
    m_connectionStatusLabel = new QLabel(this);
    m_batteryProgressBar = new QProgressBar(this);
    m_signalStrengthLabel = new QLabel(this);
    
    // Configure battery progress bar
    m_batteryProgressBar->setRange(0, 100);
    m_batteryProgressBar->setValue(85);
    m_batteryProgressBar->setTextVisible(false);
    m_batteryProgressBar->setFixedSize(60, 20);
    m_batteryProgressBar->setStyleSheet("QProgressBar { border: 1px solid #404040; "
                                        "background-color: #2d2d2d; }"
                                        "QProgressBar::chunk { background-color: #4CAF50; }");
    
    // Add widgets to status bar
    m_statusBar->addPermanentWidget(m_signalStrengthLabel);
    m_statusBar->addPermanentWidget(m_batteryProgressBar);
    m_statusBar->addPermanentWidget(m_connectionStatusLabel);
    m_statusBar->addPermanentWidget(m_temperatureLabel);
    m_statusBar->addPermanentWidget(m_dateLabel);
    m_statusBar->addPermanentWidget(m_timeLabel);
    
    // Initial update
    updateStatusBar();
}

void MainWindow::setupSystemTray()
{
    m_systemTray = new QSystemTrayIcon(this);
    m_systemTray->setIcon(QIcon("assets/icons/autodash.ico"));
    m_systemTray->setToolTip("AutoDash OS");
    
    // Create tray menu
    QMenu *trayMenu = new QMenu(this);
    trayMenu->addAction("&Restore", this, &MainWindow::restoreFromTray);
    trayMenu->addAction("&Settings", this, &MainWindow::showSettingsDialog);
    trayMenu->addSeparator();
    trayMenu->addAction("&Exit", this, &QWidget::close);
    
    m_systemTray->setContextMenu(trayMenu);
    m_systemTray->show();
    
    connect(m_systemTray, &QSystemTrayIcon::activated, 
            this, &MainWindow::handleSystemTrayActivation);
}

void MainWindow::setupCentralWidget()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    QHBoxLayout *mainLayout = new QHBoxLayout(m_centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    
    // Sidebar will be added here
    // Tab widget will be added here
    
    m_centralWidget->setLayout(mainLayout);
}

void MainWindow::setupTabWidget()
{
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setStyleSheet("QTabWidget::pane { border: none; background-color: #1a1a1a; }"
                               "QTabBar::tab { background-color: #2d2d2d; color: white; "
                               "padding: 12px 20px; margin-right: 2px; }"
                               "QTabBar::tab:selected { background-color: #404040; }"
                               "QTabBar::tab:hover { background-color: #353535; }");
    
    // Create main modules
    m_mediaPlayer = new MediaPlayer(this);
    m_bluetoothPanel = new BluetoothPanel(this);
    m_climateControl = new ClimateControl(this);
    m_cameraModule = new CameraModule(this);
    
    // Add tabs
    m_tabWidget->addTab(createHomeTab(), "Home");
    m_tabWidget->addTab(m_mediaPlayer, "Media");
    m_tabWidget->addTab(m_bluetoothPanel, "Bluetooth");
    m_tabWidget->addTab(m_climateControl, "Climate");
    m_tabWidget->addTab(m_cameraModule, "Camera");
    
    // Add to main layout
    QHBoxLayout *mainLayout = qobject_cast<QHBoxLayout*>(m_centralWidget->layout());
    if (mainLayout) {
        mainLayout->addWidget(m_tabWidget, 1);
    }
}

QWidget* MainWindow::createHomeTab()
{
    QWidget *homeTab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(homeTab);
    
    // Welcome message
    QLabel *welcomeLabel = new QLabel("Welcome to AutoDash OS", homeTab);
    welcomeLabel->setStyleSheet("font-size: 24px; color: white; padding: 20px;");
    welcomeLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(welcomeLabel);
    
    // System status grid
    QGridLayout *statusGrid = new QGridLayout();
    
    // Add status widgets
    QLabel *statusLabel = new QLabel("System Status:", homeTab);
    statusLabel->setStyleSheet("color: white; font-size: 16px;");
    statusGrid->addWidget(statusLabel, 0, 0, 1, 2);
    
    // Add more status information here...
    
    layout->addLayout(statusGrid);
    layout->addStretch();
    
    return homeTab;
}

void MainWindow::setupSidebar()
{
    m_sidebar = new QWidget(this);
    m_sidebar->setFixedWidth(SIDEBAR_WIDTH);
    m_sidebar->setStyleSheet("QWidget { background-color: #2d2d2d; border-right: 1px solid #404040; }");
    
    QVBoxLayout *sidebarLayout = new QVBoxLayout(m_sidebar);
    sidebarLayout->setContentsMargins(10, 10, 10, 10);
    
    // Logo
    QLabel *logoLabel = new QLabel("AutoDash", m_sidebar);
    logoLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: white; padding: 10px;");
    logoLabel->setAlignment(Qt::AlignCenter);
    sidebarLayout->addWidget(logoLabel);
    
    // Navigation buttons
    QPushButton *homeBtn = new QPushButton("Home", m_sidebar);
    QPushButton *mediaBtn = new QPushButton("Media", m_sidebar);
    QPushButton *bluetoothBtn = new QPushButton("Bluetooth", m_sidebar);
    QPushButton *climateBtn = new QPushButton("Climate", m_sidebar);
    QPushButton *cameraBtn = new QPushButton("Camera", m_sidebar);
    
    QString buttonStyle = "QPushButton { background-color: #404040; color: white; "
                          "border: none; padding: 12px; text-align: left; "
                          "border-radius: 5px; margin: 2px; }"
                          "QPushButton:hover { background-color: #505050; }"
                          "QPushButton:pressed { background-color: #606060; }";
    
    homeBtn->setStyleSheet(buttonStyle);
    mediaBtn->setStyleSheet(buttonStyle);
    bluetoothBtn->setStyleSheet(buttonStyle);
    climateBtn->setStyleSheet(buttonStyle);
    cameraBtn->setStyleSheet(buttonStyle);
    
    // Connect buttons
    connect(homeBtn, &QPushButton::clicked, this, [this]() { m_tabWidget->setCurrentIndex(0); });
    connect(mediaBtn, &QPushButton::clicked, this, [this]() { m_tabWidget->setCurrentIndex(1); });
    connect(bluetoothBtn, &QPushButton::clicked, this, [this]() { m_tabWidget->setCurrentIndex(2); });
    connect(climateBtn, &QPushButton::clicked, this, [this]() { m_tabWidget->setCurrentIndex(3); });
    connect(cameraBtn, &QPushButton::clicked, this, [this]() { m_tabWidget->setCurrentIndex(4); });
    
    sidebarLayout->addWidget(homeBtn);
    sidebarLayout->addWidget(mediaBtn);
    sidebarLayout->addWidget(bluetoothBtn);
    sidebarLayout->addWidget(climateBtn);
    sidebarLayout->addWidget(cameraBtn);
    sidebarLayout->addStretch();
    
    // Add sidebar to main layout
    QHBoxLayout *mainLayout = qobject_cast<QHBoxLayout*>(m_centralWidget->layout());
    if (mainLayout) {
        mainLayout->insertWidget(0, m_sidebar);
    }
}

void MainWindow::setupAnimations()
{
    // Setup sidebar opacity effect
    m_sidebarOpacity = new QGraphicsOpacityEffect(this);
    m_sidebar->setGraphicsEffect(m_sidebarOpacity);
    
    // Setup sidebar animation
    m_sidebarAnimation = new QPropertyAnimation(m_sidebarOpacity, "opacity", this);
    m_sidebarAnimation->setDuration(300);
    m_sidebarAnimation->setStartValue(1.0);
    m_sidebarAnimation->setEndValue(0.8);
}

void MainWindow::loadStylesheet()
{
    QFile file("assets/styles/main.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        setStyleSheet(stream.readAll());
        file.close();
    } else {
        // Use default stylesheet
        setStyleSheet("QMainWindow { background-color: #1a1a1a; }");
    }
}

void MainWindow::saveWindowState()
{
    m_settings.setValue("geometry", saveGeometry());
    m_settings.setValue("windowState", saveState());
    m_settings.setValue("fullscreen", m_isFullscreen);
    m_settings.setValue("minimizedToTray", m_isMinimizedToTray);
}

void MainWindow::restoreWindowState()
{
    if (m_settings.contains("geometry")) {
        restoreGeometry(m_settings.value("geometry").toByteArray());
    }
    if (m_settings.contains("windowState")) {
        restoreState(m_settings.value("windowState").toByteArray());
    }
    m_isFullscreen = m_settings.value("fullscreen", false).toBool();
    m_isMinimizedToTray = m_settings.value("minimizedToTray", false).toBool();
}

void MainWindow::createDummyData()
{
    // This would populate the interface with sample data
    LOG_INFO("MainWindow", "Creating dummy data for demonstration");
}

void MainWindow::updateStatusBar()
{
    QDateTime now = QDateTime::currentDateTime();
    
    m_timeLabel->setText(now.toString("hh:mm:ss"));
    m_dateLabel->setText(now.toString("MMM dd, yyyy"));
    m_temperatureLabel->setText("22°C");
    m_connectionStatusLabel->setText("Connected");
    m_signalStrengthLabel->setText("📶 85%");
    
    // Update battery level (simulate gradual discharge)
    static int batteryLevel = 85;
    batteryLevel = qMax(0, batteryLevel - (rand() % 2));
    m_batteryProgressBar->setValue(batteryLevel);
}

void MainWindow::showAboutDialog()
{
    QMessageBox::about(this, "About AutoDash OS",
                       "AutoDash OS v1.0.0\n\n"
                       "Embedded Infotainment System Simulator\n"
                       "© 2025 Rivian & Volkswagen Group Technologies\n\n"
                       "This is a demonstration project showcasing\n"
                       "embedded systems development skills.");
}

void MainWindow::showSettingsDialog()
{
    // TODO: Implement settings dialog
    QMessageBox::information(this, "Settings", "Settings dialog not implemented yet.");
}

void MainWindow::toggleFullscreen()
{
    if (m_isFullscreen) {
        showNormal();
        m_isFullscreen = false;
    } else {
        showFullScreen();
        m_isFullscreen = true;
    }
}

void MainWindow::showSystemInfo()
{
    QString info = "AutoDash OS System Information\n\n"
                   "Version: 1.0.0\n"
                   "Build Date: " + QString(__DATE__) + "\n"
                   "Qt Version: " + QString(QT_VERSION_STR) + "\n"
                   "Platform: " + QSysInfo::prettyProductName() + "\n"
                   "Architecture: " + QSysInfo::currentCpuArchitecture();
    
    QMessageBox::information(this, "System Information", info);
}

void MainWindow::showLogViewer()
{
    // TODO: Implement log viewer
    QMessageBox::information(this, "Log Viewer", "Log viewer not implemented yet.");
}

void MainWindow::showDebugPanel()
{
    // TODO: Implement debug panel
    QMessageBox::information(this, "Debug Panel", "Debug panel not implemented yet.");
}

void MainWindow::handleSystemTrayActivation(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::DoubleClick) {
        restoreFromTray();
    }
}

void MainWindow::minimizeToTray()
{
    hide();
    m_isMinimizedToTray = true;
    m_systemTray->showMessage("AutoDash OS", "Minimized to system tray", 
                              QSystemTrayIcon::Information, 2000);
}

void MainWindow::restoreFromTray()
{
    show();
    raise();
    activateWindow();
    m_isMinimizedToTray = false;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_systemTray->isVisible()) {
        QMessageBox::information(this, "AutoDash OS",
                               "The application will keep running in the system tray.\n"
                               "To terminate the program, choose <b>Quit</b> in the "
                               "context menu of the system tray entry.");
        hide();
        event->ignore();
    } else {
        event->accept();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    // Handle resize-specific logic here
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
    
    // Custom painting for automotive-style effects
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Add subtle gradient overlay
    QLinearGradient gradient(0, 0, width(), height());
    gradient.setColorAt(0, QColor(0, 0, 0, 30));
    gradient.setColorAt(1, QColor(0, 0, 0, 10));
    painter.fillRect(rect(), gradient);
} 