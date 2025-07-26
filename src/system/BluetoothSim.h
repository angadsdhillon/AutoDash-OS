#ifndef BLUETOOTHSIM_H
#define BLUETOOTHSIM_H

#include <QObject>
#include <QTimer>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QMap>
#include <QSet>
#include <random>
#include <memory>

enum class BluetoothDeviceType {
    PHONE,
    HEADSET,
    SPEAKER,
    CAR_AUDIO,
    SMARTWATCH,
    TABLET,
    LAPTOP
};

enum class ConnectionState {
    DISCONNECTED,
    SEARCHING,
    CONNECTING,
    CONNECTED,
    PAIRING,
    PAIRED,
    ERROR
};

struct BluetoothDevice {
    QString deviceId;
    QString deviceName;
    QString deviceAddress;
    BluetoothDeviceType deviceType;
    ConnectionState connectionState;
    bool isPaired;
    bool isTrusted;
    int signalStrength;
    QString lastSeen;
    QDateTime pairedTime;
    QStringList supportedProfiles;
    QString manufacturer;
    QString model;
    QString firmwareVersion;
};

class BluetoothSim : public QObject
{
    Q_OBJECT

public:
    static BluetoothSim& getInstance();
    
    // Bluetooth stack simulation
    bool initialize();
    bool isInitialized() const;
    void startDiscovery();
    void stopDiscovery();
    bool isDiscovering() const;
    
    // Device management
    QList<BluetoothDevice> getAvailableDevices() const;
    QList<BluetoothDevice> getPairedDevices() const;
    BluetoothDevice getDevice(const QString& deviceId) const;
    bool isDevicePaired(const QString& deviceId) const;
    
    // Connection management
    bool pairDevice(const QString& deviceId);
    bool unpairDevice(const QString& deviceId);
    bool connectDevice(const QString& deviceId);
    bool disconnectDevice(const QString& deviceId);
    ConnectionState getConnectionState(const QString& deviceId) const;
    
    // Device simulation
    void simulateDeviceAppearance(const QString& deviceName, BluetoothDeviceType type);
    void simulateDeviceDisappearance(const QString& deviceId);
    void simulateConnectionError(const QString& deviceId, bool enable);
    void simulatePairingError(const QString& deviceId, bool enable);
    
    // Configuration
    void setDiscoveryTimeout(int seconds);
    void setPairingTimeout(int seconds);
    void setConnectionTimeout(int seconds);
    void enableAutoReconnect(bool enable);
    
    // Profile management
    QStringList getSupportedProfiles() const;
    bool isProfileSupported(const QString& profile) const;
    bool enableProfile(const QString& deviceId, const QString& profile);
    bool disableProfile(const QString& deviceId, const QString& profile);
    
    // Signal strength simulation
    void updateSignalStrength(const QString& deviceId, int strength);
    int getSignalStrength(const QString& deviceId) const;
    
    // Error simulation
    void simulateBluetoothOff(bool enable);
    void simulateInterference(bool enable);
    void simulateLowBattery(bool enable);

signals:
    void deviceDiscovered(const BluetoothDevice& device);
    void deviceRemoved(const QString& deviceId);
    void devicePaired(const QString& deviceId);
    void deviceUnpaired(const QString& deviceId);
    void deviceConnected(const QString& deviceId);
    void deviceDisconnected(const QString& deviceId);
    void connectionStateChanged(const QString& deviceId, ConnectionState state);
    void signalStrengthChanged(const QString& deviceId, int strength);
    void pairingError(const QString& deviceId, const QString& error);
    void connectionError(const QString& deviceId, const QString& error);
    void discoveryStarted();
    void discoveryStopped();

private:
    BluetoothSim();
    ~BluetoothSim();
    BluetoothSim(const BluetoothSim&) = delete;
    BluetoothSim& operator=(const BluetoothSim&) = delete;
    
    void generateMockDevices();
    void updateDeviceStates();
    void simulatePairingProcess(const QString& deviceId);
    void simulateConnectionProcess(const QString& deviceId);
    void savePairedDevices();
    void loadPairedDevices();
    QString generateDeviceAddress() const;
    QString generateDeviceName(BluetoothDeviceType type) const;
    QStringList generateSupportedProfiles(BluetoothDeviceType type) const;
    void updateSignalStrengths();
    
    std::unique_ptr<QTimer> m_discoveryTimer;
    std::unique_ptr<QTimer> m_stateTimer;
    std::unique_ptr<QTimer> m_signalTimer;
    std::mt19937 m_randomGenerator;
    
    QList<BluetoothDevice> m_availableDevices;
    QList<BluetoothDevice> m_pairedDevices;
    QMap<QString, QTimer*> m_pairingTimers;
    QMap<QString, QTimer*> m_connectionTimers;
    
    bool m_isInitialized;
    bool m_isDiscovering;
    bool m_autoReconnect;
    bool m_simulateBluetoothOff;
    bool m_simulateInterference;
    bool m_simulateLowBattery;
    
    int m_discoveryTimeout;
    int m_pairingTimeout;
    int m_connectionTimeout;
    
    QStringList m_supportedProfiles;
    
    static const QString CONFIG_FILE;
    static const QStringList DEFAULT_SUPPORTED_PROFILES;
};

#endif // BLUETOOTHSIM_H 