#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QStatusBar>
#include <QMenuBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QProgressBar>
#include <QListWidget>
#include <QTextEdit>
#include <QGroupBox>
#include <QFrame>
#include <QTimer>
#include <QDateTime>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QGraphicsDropShadowEffect>
#include <QPainter>
#include <QStyle>
#include <QApplication>

#include "MediaPlayer.h"
#include "BluetoothPanel.h"
#include "ClimateControl.h"
#include "CameraModule.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateStatusBar();
    void showAboutDialog();
    void showSettingsDialog();
    void toggleFullscreen();
    void showSystemInfo();
    void showLogViewer();
    void showDebugPanel();
    void handleSystemTrayActivation(QSystemTrayIcon::ActivationReason reason);
    void minimizeToTray();
    void restoreFromTray();

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupSystemTray();
    void setupCentralWidget();
    void setupTabWidget();
    void setupSidebar();
    void setupAnimations();
    void loadStylesheet();
    void saveWindowState();
    void restoreWindowState();
    void createDummyData();
    
    // UI Components
    QTabWidget *m_tabWidget;
    QWidget *m_sidebar;
    QWidget *m_centralWidget;
    QStatusBar *m_statusBar;
    QSystemTrayIcon *m_systemTray;
    
    // Main modules
    MediaPlayer *m_mediaPlayer;
    BluetoothPanel *m_bluetoothPanel;
    ClimateControl *m_climateControl;
    CameraModule *m_cameraModule;
    
    // Status components
    QLabel *m_timeLabel;
    QLabel *m_dateLabel;
    QLabel *m_temperatureLabel;
    QLabel *m_connectionStatusLabel;
    QProgressBar *m_batteryProgressBar;
    QLabel *m_signalStrengthLabel;
    
    // Timers
    QTimer *m_statusUpdateTimer;
    QTimer *m_animationTimer;
    
    // Animations
    QPropertyAnimation *m_sidebarAnimation;
    QGraphicsOpacityEffect *m_sidebarOpacity;
    
    // Settings
    QSettings m_settings;
    bool m_isFullscreen;
    bool m_isMinimizedToTray;
    
    // Constants
    static const int WINDOW_WIDTH = 1200;
    static const int WINDOW_HEIGHT = 800;
    static const int SIDEBAR_WIDTH = 200;
    static const int STATUS_UPDATE_INTERVAL = 1000; // 1 second
};

#endif // MAINWINDOW_H 