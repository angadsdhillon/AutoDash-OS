#ifndef BLUETOOTHPANEL_H
#define BLUETOOTHPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QProgressBar>
#include <QSlider>
#include <QGroupBox>
#include <QFrame>
#include <QTimer>
#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>
#include <QAction>
#include <QStyle>
#include <QApplication>

#include "../system/Logger.h"
#include "../system/BluetoothSim.h"

class BluetoothPanel : public QWidget
{
    Q_OBJECT

public:
    explicit BluetoothPanel(QWidget *parent = nullptr);
    ~BluetoothPanel();

private slots:
    void startDiscovery();
    void stopDiscovery();
    void pairDevice();
    void unpairDevice();
    void connectDevice();
    void disconnectDevice();
    void onDeviceDiscovered(const BluetoothDevice& device);
    void onDeviceRemoved(const QString& deviceId);
    void onDevicePaired(const QString& deviceId);
    void onDeviceUnpaired(const QString& deviceId);
    void onDeviceConnected(const QString& deviceId);
    void onDeviceDisconnected(const QString& deviceId);
    void onConnectionStateChanged(const QString& deviceId, ConnectionState state);
    void onSignalStrengthChanged(const QString& deviceId, int strength);
    void onPairingError(const QString& deviceId, const QString& error);
    void onConnectionError(const QString& deviceId, const QString& error);
    void onDiscoveryStarted();
    void onDiscoveryStopped();
    void onDeviceListContextMenu(const QPoint& pos);
    void onPairedDeviceListContextMenu(const QPoint& pos);
    void showDeviceDetails();
    void enableProfile();
    void disableProfile();
    void updateSignalStrength();
    void simulateDeviceAppearance();
    void simulateDeviceRemoval();
    void showBluetoothSettings();

private:
    void setupUI();
    void setupDeviceLists();
    void setupControls();
    void updateDeviceLists();
    void updateConnectionStatus();
    void updateSignalStrengthDisplay();
    void showErrorMessage(const QString& message);
    void showInfoMessage(const QString& message);
    QString getDeviceTypeString(BluetoothDeviceType type);
    QString getConnectionStateString(ConnectionState state);
    QColor getSignalStrengthColor(int strength);
    void createDummyDevices();
    
    // UI Components
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_controlsLayout;
    QHBoxLayout *m_listsLayout;
    
    // Device lists
    QListWidget *m_availableDevicesList;
    QListWidget *m_pairedDevicesList;
    
    // Control buttons
    QPushButton *m_discoverButton;
    QPushButton *m_pairButton;
    QPushButton *m_connectButton;
    QPushButton *m_disconnectButton;
    QPushButton *m_unpairButton;
    QPushButton *m_settingsButton;
    QPushButton *m_simulateButton;
    
    // Status displays
    QLabel *m_discoveryStatusLabel;
    QLabel *m_connectionStatusLabel;
    QProgressBar *m_signalStrengthBar;
    QLabel *m_signalStrengthLabel;
    
    // Bluetooth simulator
    BluetoothSim *m_bluetoothSim;
    
    // Timers
    QTimer *m_signalUpdateTimer;
    
    // State
    QString m_selectedAvailableDevice;
    QString m_selectedPairedDevice;
    bool m_isDiscovering;
    
    // Constants
    static const int SIGNAL_UPDATE_INTERVAL = 2000; // 2 seconds
};

#endif // BLUETOOTHPANEL_H 