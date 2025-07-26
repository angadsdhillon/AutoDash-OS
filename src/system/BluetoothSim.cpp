#include "BluetoothSim.h"
#include "Logger.h"
#include <QStandardPaths>
#include <QDateTime>
#include <QJsonArray>
#include <QRegularExpression>

const QString BluetoothSim::CONFIG_FILE = "config/bluetooth_devices.json";
const QStringList BluetoothSim::DEFAULT_SUPPORTED_PROFILES = {
    "A2DP", "AVRCP", "HFP", "HSP", "PBAP", "MAP", "OPP", "HID"
};

BluetoothSim::BluetoothSim()
    : m_discoveryTimer(std::make_unique<QTimer>(this))
    , m_stateTimer(std::make_unique<QTimer>(this))
    , m_signalTimer(std::make_unique<QTimer>(this))
    , m_randomGenerator(std::random_device{}())
    , m_isInitialized(false)
    , m_isDiscovering(false)
    , m_autoReconnect(true)
    , m_simulateBluetoothOff(false)
    , m_simulateInterference(false)
    , m_simulateLowBattery(false)
    , m_discoveryTimeout(30)
    , m_pairingTimeout(10)
    , m_connectionTimeout(15)
    , m_supportedProfiles(DEFAULT_SUPPORTED_PROFILES)
{
    // Connect timers
    connect(m_discoveryTimer.get(), &QTimer::timeout, this, &BluetoothSim::updateDeviceStates);
    connect(m_stateTimer.get(), &QTimer::timeout, this, &BluetoothSim::updateDeviceStates);
    connect(m_signalTimer.get(), &QTimer::timeout, this, &BluetoothSim::updateSignalStrengths);
    
    // Load paired devices
    loadPairedDevices();
    
    LOG_INFO("BluetoothSim", "Bluetooth simulation system initialized");
}

BluetoothSim::~BluetoothSim()
{
    // Clean up timers
    for (auto timer : m_pairingTimers.values()) {
        if (timer) {
            timer->stop();
            timer->deleteLater();
        }
    }
    for (auto timer : m_connectionTimers.values()) {
        if (timer) {
            timer->stop();
            timer->deleteLater();
        }
    }
    
    savePairedDevices();
    LOG_INFO("BluetoothSim", "Bluetooth simulation system shutdown");
}

BluetoothSim& BluetoothSim::getInstance()
{
    static BluetoothSim instance;
    return instance;
}

bool BluetoothSim::initialize()
{
    if (m_simulateBluetoothOff) {
        LOG_ERROR("BluetoothSim", "Bluetooth is turned off");
        return false;
    }
    
    m_isInitialized = true;
    m_stateTimer->start(5000); // Update device states every 5 seconds
    m_signalTimer->start(3000); // Update signal strength every 3 seconds
    
    LOG_INFO("BluetoothSim", "Bluetooth stack initialized");
    return true;
}

bool BluetoothSim::isInitialized() const
{
    return m_isInitialized && !m_simulateBluetoothOff;
}

void BluetoothSim::startDiscovery()
{
    if (!isInitialized()) {
        LOG_ERROR("BluetoothSim", "Cannot start discovery - Bluetooth not initialized");
        return;
    }
    
    m_isDiscovering = true;
    m_discoveryTimer->start(2000); // Discover devices every 2 seconds
    
    LOG_INFO("BluetoothSim", "Bluetooth discovery started");
    emit discoveryStarted();
}

void BluetoothSim::stopDiscovery()
{
    m_isDiscovering = false;
    m_discoveryTimer->stop();
    
    LOG_INFO("BluetoothSim", "Bluetooth discovery stopped");
    emit discoveryStopped();
}

bool BluetoothSim::isDiscovering() const
{
    return m_isDiscovering && isInitialized();
}

QList<BluetoothDevice> BluetoothSim::getAvailableDevices() const
{
    return m_availableDevices;
}

QList<BluetoothDevice> BluetoothSim::getPairedDevices() const
{
    return m_pairedDevices;
}

BluetoothDevice BluetoothSim::getDevice(const QString& deviceId) const
{
    // Check available devices
    for (const auto& device : m_availableDevices) {
        if (device.deviceId == deviceId) {
            return device;
        }
    }
    
    // Check paired devices
    for (const auto& device : m_pairedDevices) {
        if (device.deviceId == deviceId) {
            return device;
        }
    }
    
    return BluetoothDevice{};
}

bool BluetoothSim::isDevicePaired(const QString& deviceId) const
{
    for (const auto& device : m_pairedDevices) {
        if (device.deviceId == deviceId) {
            return device.isPaired;
        }
    }
    return false;
}

bool BluetoothSim::pairDevice(const QString& deviceId)
{
    if (!isInitialized()) {
        LOG_ERROR("BluetoothSim", "Cannot pair device - Bluetooth not initialized");
        return false;
    }
    
    // Find the device
    for (auto& device : m_availableDevices) {
        if (device.deviceId == deviceId) {
            if (device.connectionState == ConnectionState::PAIRING) {
                LOG_WARNING("BluetoothSim", QString("Device %1 is already being paired").arg(deviceId));
                return false;
            }
            
            device.connectionState = ConnectionState::PAIRING;
            emit connectionStateChanged(deviceId, ConnectionState::PAIRING);
            
            LOG_INFO("BluetoothSim", QString("Starting pairing process for device: %1").arg(device.deviceName));
            
            // Simulate pairing process
            simulatePairingProcess(deviceId);
            return true;
        }
    }
    
    LOG_ERROR("BluetoothSim", QString("Device %1 not found for pairing").arg(deviceId));
    return false;
}

bool BluetoothSim::unpairDevice(const QString& deviceId)
{
    for (auto it = m_pairedDevices.begin(); it != m_pairedDevices.end(); ++it) {
        if (it->deviceId == deviceId) {
            LOG_INFO("BluetoothSim", QString("Unpairing device: %1").arg(it->deviceName));
            
            // Disconnect if connected
            if (it->connectionState == ConnectionState::CONNECTED) {
                disconnectDevice(deviceId);
            }
            
            m_pairedDevices.erase(it);
            emit deviceUnpaired(deviceId);
            savePairedDevices();
            return true;
        }
    }
    
    LOG_ERROR("BluetoothSim", QString("Device %1 not found for unpairing").arg(deviceId));
    return false;
}

bool BluetoothSim::connectDevice(const QString& deviceId)
{
    if (!isInitialized()) {
        LOG_ERROR("BluetoothSim", "Cannot connect device - Bluetooth not initialized");
        return false;
    }
    
    // Check if device is paired
    if (!isDevicePaired(deviceId)) {
        LOG_ERROR("BluetoothSim", QString("Cannot connect device %1 - not paired").arg(deviceId));
        return false;
    }
    
    // Find the device
    for (auto& device : m_pairedDevices) {
        if (device.deviceId == deviceId) {
            if (device.connectionState == ConnectionState::CONNECTING) {
                LOG_WARNING("BluetoothSim", QString("Device %1 is already connecting").arg(deviceId));
                return false;
            }
            
            device.connectionState = ConnectionState::CONNECTING;
            emit connectionStateChanged(deviceId, ConnectionState::CONNECTING);
            
            LOG_INFO("BluetoothSim", QString("Starting connection process for device: %1").arg(device.deviceName));
            
            // Simulate connection process
            simulateConnectionProcess(deviceId);
            return true;
        }
    }
    
    LOG_ERROR("BluetoothSim", QString("Device %1 not found for connection").arg(deviceId));
    return false;
}

bool BluetoothSim::disconnectDevice(const QString& deviceId)
{
    for (auto& device : m_pairedDevices) {
        if (device.deviceId == deviceId) {
            if (device.connectionState == ConnectionState::CONNECTED) {
                device.connectionState = ConnectionState::DISCONNECTED;
                emit deviceDisconnected(deviceId);
                emit connectionStateChanged(deviceId, ConnectionState::DISCONNECTED);
                
                LOG_INFO("BluetoothSim", QString("Disconnected device: %1").arg(device.deviceName));
                return true;
            }
        }
    }
    
    LOG_ERROR("BluetoothSim", QString("Device %1 not found or not connected").arg(deviceId));
    return false;
}

ConnectionState BluetoothSim::getConnectionState(const QString& deviceId) const
{
    for (const auto& device : m_pairedDevices) {
        if (device.deviceId == deviceId) {
            return device.connectionState;
        }
    }
    return ConnectionState::DISCONNECTED;
}

void BluetoothSim::simulateDeviceAppearance(const QString& deviceName, BluetoothDeviceType type)
{
    BluetoothDevice device;
    device.deviceId = QString("BT_%1").arg(QDateTime::currentMSecsSinceEpoch());
    device.deviceName = deviceName;
    device.deviceAddress = generateDeviceAddress();
    device.deviceType = type;
    device.connectionState = ConnectionState::DISCONNECTED;
    device.isPaired = false;
    device.isTrusted = false;
    device.signalStrength = 75 + (m_randomGenerator() % 25); // 75-100%
    device.lastSeen = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    device.supportedProfiles = generateSupportedProfiles(type);
    device.manufacturer = "Generic Manufacturer";
    device.model = "Generic Model";
    device.firmwareVersion = "1.0.0";
    
    m_availableDevices.append(device);
    
    LOG_INFO("BluetoothSim", QString("Device appeared: %1 (%2)").arg(deviceName).arg(device.deviceAddress));
    emit deviceDiscovered(device);
}

void BluetoothSim::simulateDeviceDisappearance(const QString& deviceId)
{
    for (auto it = m_availableDevices.begin(); it != m_availableDevices.end(); ++it) {
        if (it->deviceId == deviceId) {
            LOG_INFO("BluetoothSim", QString("Device disappeared: %1").arg(it->deviceName));
            emit deviceRemoved(deviceId);
            m_availableDevices.erase(it);
            break;
        }
    }
}

void BluetoothSim::simulateConnectionError(const QString& deviceId, bool enable)
{
    // This would be implemented to simulate connection errors
    if (enable) {
        LOG_WARNING("BluetoothSim", QString("Connection error simulation enabled for device %1").arg(deviceId));
        emit connectionError(deviceId, "Simulated connection error");
    }
}

void BluetoothSim::simulatePairingError(const QString& deviceId, bool enable)
{
    // This would be implemented to simulate pairing errors
    if (enable) {
        LOG_WARNING("BluetoothSim", QString("Pairing error simulation enabled for device %1").arg(deviceId));
        emit pairingError(deviceId, "Simulated pairing error");
    }
}

void BluetoothSim::setDiscoveryTimeout(int seconds)
{
    m_discoveryTimeout = seconds;
    LOG_INFO("BluetoothSim", QString("Discovery timeout set to %1 seconds").arg(seconds));
}

void BluetoothSim::setPairingTimeout(int seconds)
{
    m_pairingTimeout = seconds;
    LOG_INFO("BluetoothSim", QString("Pairing timeout set to %1 seconds").arg(seconds));
}

void BluetoothSim::setConnectionTimeout(int seconds)
{
    m_connectionTimeout = seconds;
    LOG_INFO("BluetoothSim", QString("Connection timeout set to %1 seconds").arg(seconds));
}

void BluetoothSim::enableAutoReconnect(bool enable)
{
    m_autoReconnect = enable;
    LOG_INFO("BluetoothSim", QString("Auto reconnect %1").arg(enable ? "enabled" : "disabled"));
}

QStringList BluetoothSim::getSupportedProfiles() const
{
    return m_supportedProfiles;
}

bool BluetoothSim::isProfileSupported(const QString& profile) const
{
    return m_supportedProfiles.contains(profile.toUpper());
}

bool BluetoothSim::enableProfile(const QString& deviceId, const QString& profile)
{
    for (auto& device : m_pairedDevices) {
        if (device.deviceId == deviceId) {
            if (!device.supportedProfiles.contains(profile.toUpper())) {
                LOG_ERROR("BluetoothSim", QString("Profile %1 not supported by device %2").arg(profile).arg(deviceId));
                return false;
            }
            
            LOG_INFO("BluetoothSim", QString("Enabled profile %1 for device %2").arg(profile).arg(deviceId));
            return true;
        }
    }
    
    LOG_ERROR("BluetoothSim", QString("Device %1 not found for profile enable").arg(deviceId));
    return false;
}

bool BluetoothSim::disableProfile(const QString& deviceId, const QString& profile)
{
    for (auto& device : m_pairedDevices) {
        if (device.deviceId == deviceId) {
            LOG_INFO("BluetoothSim", QString("Disabled profile %1 for device %2").arg(profile).arg(deviceId));
            return true;
        }
    }
    
    LOG_ERROR("BluetoothSim", QString("Device %1 not found for profile disable").arg(deviceId));
    return false;
}

void BluetoothSim::updateSignalStrength(const QString& deviceId, int strength)
{
    for (auto& device : m_availableDevices) {
        if (device.deviceId == deviceId) {
            device.signalStrength = strength;
            emit signalStrengthChanged(deviceId, strength);
            break;
        }
    }
    
    for (auto& device : m_pairedDevices) {
        if (device.deviceId == deviceId) {
            device.signalStrength = strength;
            emit signalStrengthChanged(deviceId, strength);
            break;
        }
    }
}

int BluetoothSim::getSignalStrength(const QString& deviceId) const
{
    for (const auto& device : m_availableDevices) {
        if (device.deviceId == deviceId) {
            return device.signalStrength;
        }
    }
    
    for (const auto& device : m_pairedDevices) {
        if (device.deviceId == deviceId) {
            return device.signalStrength;
        }
    }
    
    return 0;
}

void BluetoothSim::simulateBluetoothOff(bool enable)
{
    m_simulateBluetoothOff = enable;
    if (enable) {
        stopDiscovery();
        LOG_WARNING("BluetoothSim", "Bluetooth turned off");
    } else {
        LOG_INFO("BluetoothSim", "Bluetooth turned on");
    }
}

void BluetoothSim::simulateInterference(bool enable)
{
    m_simulateInterference = enable;
    LOG_INFO("BluetoothSim", QString("Interference simulation %1").arg(enable ? "enabled" : "disabled"));
}

void BluetoothSim::simulateLowBattery(bool enable)
{
    m_simulateLowBattery = enable;
    LOG_INFO("BluetoothSim", QString("Low battery simulation %1").arg(enable ? "enabled" : "disabled"));
}

void BluetoothSim::generateMockDevices()
{
    // Generate some mock devices for demonstration
    simulateDeviceAppearance("iPhone 15 Pro", BluetoothDeviceType::PHONE);
    simulateDeviceAppearance("Samsung Galaxy S24", BluetoothDeviceType::PHONE);
    simulateDeviceAppearance("Sony WH-1000XM5", BluetoothDeviceType::HEADSET);
    simulateDeviceAppearance("Bose QuietComfort 45", BluetoothDeviceType::HEADSET);
    simulateDeviceAppearance("JBL Flip 6", BluetoothDeviceType::SPEAKER);
    simulateDeviceAppearance("VW Passat Audio", BluetoothDeviceType::CAR_AUDIO);
    simulateDeviceAppearance("Apple Watch Series 9", BluetoothDeviceType::SMARTWATCH);
}

void BluetoothSim::updateDeviceStates()
{
    if (!isInitialized()) {
        return;
    }
    
    // Update device states and simulate discovery
    if (m_isDiscovering) {
        // Randomly discover new devices
        if (m_randomGenerator() % 100 < 10) { // 10% chance
            generateMockDevices();
        }
    }
}

void BluetoothSim::simulatePairingProcess(const QString& deviceId)
{
    // Create a timer to simulate the pairing process
    QTimer* pairingTimer = new QTimer(this);
    pairingTimer->setSingleShot(true);
    
    connect(pairingTimer, &QTimer::timeout, this, [this, deviceId, pairingTimer]() {
        // Find the device and complete pairing
        for (auto& device : m_availableDevices) {
            if (device.deviceId == deviceId) {
                device.isPaired = true;
                device.connectionState = ConnectionState::PAIRED;
                device.pairedTime = QDateTime::currentDateTime();
                
                // Move to paired devices
                m_pairedDevices.append(device);
                m_availableDevices.removeOne(device);
                
                LOG_INFO("BluetoothSim", QString("Device paired successfully: %1").arg(device.deviceName));
                emit devicePaired(deviceId);
                emit connectionStateChanged(deviceId, ConnectionState::PAIRED);
                
                savePairedDevices();
                break;
            }
        }
        
        pairingTimer->deleteLater();
        m_pairingTimers.remove(deviceId);
    });
    
    pairingTimer->start(m_pairingTimeout * 1000);
    m_pairingTimers[deviceId] = pairingTimer;
}

void BluetoothSim::simulateConnectionProcess(const QString& deviceId)
{
    // Create a timer to simulate the connection process
    QTimer* connectionTimer = new QTimer(this);
    connectionTimer->setSingleShot(true);
    
    connect(connectionTimer, &QTimer::timeout, this, [this, deviceId, connectionTimer]() {
        // Find the device and complete connection
        for (auto& device : m_pairedDevices) {
            if (device.deviceId == deviceId) {
                device.connectionState = ConnectionState::CONNECTED;
                
                LOG_INFO("BluetoothSim", QString("Device connected successfully: %1").arg(device.deviceName));
                emit deviceConnected(deviceId);
                emit connectionStateChanged(deviceId, ConnectionState::CONNECTED);
                break;
            }
        }
        
        connectionTimer->deleteLater();
        m_connectionTimers.remove(deviceId);
    });
    
    connectionTimer->start(m_connectionTimeout * 1000);
    m_connectionTimers[deviceId] = connectionTimer;
}

void BluetoothSim::savePairedDevices()
{
    QJsonArray deviceArray;
    for (const auto& device : m_pairedDevices) {
        QJsonObject deviceObj;
        deviceObj["deviceId"] = device.deviceId;
        deviceObj["deviceName"] = device.deviceName;
        deviceObj["deviceAddress"] = device.deviceAddress;
        deviceObj["deviceType"] = static_cast<int>(device.deviceType);
        deviceObj["connectionState"] = static_cast<int>(device.connectionState);
        deviceObj["isPaired"] = device.isPaired;
        deviceObj["isTrusted"] = device.isTrusted;
        deviceObj["signalStrength"] = device.signalStrength;
        deviceObj["lastSeen"] = device.lastSeen;
        deviceObj["pairedTime"] = device.pairedTime.toString(Qt::ISODate);
        deviceObj["supportedProfiles"] = QJsonArray::fromStringList(device.supportedProfiles);
        deviceObj["manufacturer"] = device.manufacturer;
        deviceObj["model"] = device.model;
        deviceObj["firmwareVersion"] = device.firmwareVersion;
        
        deviceArray.append(deviceObj);
    }
    
    QFile file(CONFIG_FILE);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(deviceArray).toJson());
        LOG_DEBUG("BluetoothSim", "Paired devices saved");
    }
}

void BluetoothSim::loadPairedDevices()
{
    QFile file(CONFIG_FILE);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonArray deviceArray = doc.array();
        
        for (const QJsonValue& value : deviceArray) {
            QJsonObject deviceObj = value.toObject();
            BluetoothDevice device;
            device.deviceId = deviceObj["deviceId"].toString();
            device.deviceName = deviceObj["deviceName"].toString();
            device.deviceAddress = deviceObj["deviceAddress"].toString();
            device.deviceType = static_cast<BluetoothDeviceType>(deviceObj["deviceType"].toInt());
            device.connectionState = static_cast<ConnectionState>(deviceObj["connectionState"].toInt());
            device.isPaired = deviceObj["isPaired"].toBool();
            device.isTrusted = deviceObj["isTrusted"].toBool();
            device.signalStrength = deviceObj["signalStrength"].toInt();
            device.lastSeen = deviceObj["lastSeen"].toString();
            device.pairedTime = QDateTime::fromString(deviceObj["pairedTime"].toString(), Qt::ISODate);
            device.supportedProfiles = deviceObj["supportedProfiles"].toVariant().toStringList();
            device.manufacturer = deviceObj["manufacturer"].toString();
            device.model = deviceObj["model"].toString();
            device.firmwareVersion = deviceObj["firmwareVersion"].toString();
            
            m_pairedDevices.append(device);
        }
        
        LOG_DEBUG("BluetoothSim", QString("Loaded %1 paired devices from config").arg(m_pairedDevices.size()));
    }
}

QString BluetoothSim::generateDeviceAddress() const
{
    // Generate a mock Bluetooth address (XX:XX:XX:XX:XX:XX format)
    QString address;
    for (int i = 0; i < 6; ++i) {
        if (i > 0) address += ":";
        address += QString("%1").arg(m_randomGenerator() % 256, 2, 16, QChar('0')).toUpper();
    }
    return address;
}

QString BluetoothSim::generateDeviceName(BluetoothDeviceType type) const
{
    QStringList names;
    switch (type) {
        case BluetoothDeviceType::PHONE:
            names = {"iPhone", "Samsung Galaxy", "Google Pixel", "OnePlus", "Xiaomi"};
            break;
        case BluetoothDeviceType::HEADSET:
            names = {"Sony WH-1000XM", "Bose QuietComfort", "Apple AirPods", "Samsung Galaxy Buds", "Jabra Elite"};
            break;
        case BluetoothDeviceType::SPEAKER:
            names = {"JBL Flip", "Bose SoundLink", "Sony SRS", "UE Boom", "Anker Soundcore"};
            break;
        case BluetoothDeviceType::CAR_AUDIO:
            names = {"VW Passat Audio", "BMW iDrive", "Mercedes COMAND", "Audi MMI", "Tesla Audio"};
            break;
        case BluetoothDeviceType::SMARTWATCH:
            names = {"Apple Watch", "Samsung Galaxy Watch", "Garmin Fenix", "Fitbit Sense", "Amazfit"};
            break;
        default:
            names = {"Generic Device"};
            break;
    }
    
    return names[m_randomGenerator() % names.size()];
}

QStringList BluetoothSim::generateSupportedProfiles(BluetoothDeviceType type) const
{
    QStringList profiles;
    switch (type) {
        case BluetoothDeviceType::PHONE:
            profiles = {"A2DP", "AVRCP", "HFP", "HSP", "PBAP", "MAP"};
            break;
        case BluetoothDeviceType::HEADSET:
            profiles = {"A2DP", "AVRCP", "HFP", "HSP"};
            break;
        case BluetoothDeviceType::SPEAKER:
            profiles = {"A2DP", "AVRCP"};
            break;
        case BluetoothDeviceType::CAR_AUDIO:
            profiles = {"A2DP", "AVRCP", "HFP", "HSP", "PBAP", "MAP"};
            break;
        case BluetoothDeviceType::SMARTWATCH:
            profiles = {"HFP", "HSP", "OPP"};
            break;
        default:
            profiles = {"A2DP", "AVRCP"};
            break;
    }
    
    return profiles;
}

void BluetoothSim::updateSignalStrengths()
{
    if (!isInitialized()) {
        return;
    }
    
    // Update signal strengths for all devices
    for (auto& device : m_availableDevices) {
        int newStrength = device.signalStrength + (m_randomGenerator() % 11 - 5); // ±5
        newStrength = qBound(0, newStrength, 100);
        if (newStrength != device.signalStrength) {
            device.signalStrength = newStrength;
            emit signalStrengthChanged(device.deviceId, newStrength);
        }
    }
    
    for (auto& device : m_pairedDevices) {
        int newStrength = device.signalStrength + (m_randomGenerator() % 11 - 5); // ±5
        newStrength = qBound(0, newStrength, 100);
        if (newStrength != device.signalStrength) {
            device.signalStrength = newStrength;
            emit signalStrengthChanged(device.deviceId, newStrength);
        }
    }
} 