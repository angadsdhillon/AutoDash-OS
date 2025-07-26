#include "MockI2C.h"
#include "Logger.h"
#include <QStandardPaths>
#include <QJsonArray>
#include <QDateTime>
#include <cmath>

const QString MockI2C::CONFIG_FILE = "config/i2c_calibration.json";

MockI2C::MockI2C()
    : m_randomGenerator(std::random_device{}())
    , m_tempDist(18.0, 25.0)
    , m_humidityDist(40.0, 60.0)
    , m_pressureDist(1013.0, 1013.5)
    , m_lightDist(100.0, 1000.0)
    , m_isConnected(false)
    , m_simulateConnectionError(false)
    , m_simulateSensorFailure(false)
    , m_simulateDataCorruption(false)
    , m_dataLoggingEnabled(false)
    , m_tempOffset(0.0)
    , m_humidityOffset(0.0)
    , m_pressureOffset(0.0)
    , m_lightOffset(0.0)
    , m_tempMin(18.0), m_tempMax(25.0)
    , m_humidityMin(40.0), m_humidityMax(60.0)
    , m_pressureMin(1013.0), m_pressureMax(1013.5)
    , m_lightMin(100.0), m_lightMax(1000.0)
{
    m_updateTimer = std::make_unique<QTimer>(this);
    connect(m_updateTimer.get(), &QTimer::timeout, this, &MockI2C::updateSensorData);
    
    // Load calibration data
    loadCalibrationData();
    
    LOG_INFO("MockI2C", "Mock I2C sensor system initialized");
}

MockI2C::~MockI2C()
{
    if (m_dataLoggingEnabled) {
        saveCalibrationData();
    }
    LOG_INFO("MockI2C", "Mock I2C sensor system shutdown");
}

MockI2C& MockI2C::getInstance()
{
    static MockI2C instance;
    return instance;
}

bool MockI2C::begin(uint8_t address)
{
    if (m_simulateConnectionError) {
        LOG_ERROR("MockI2C", QString("Failed to connect to I2C device at address 0x%1").arg(address, 0, 16));
        emit connectionError("I2C connection failed - device not responding");
        return false;
    }
    
    m_isConnected = true;
    LOG_INFO("MockI2C", QString("Connected to I2C device at address 0x%1").arg(address, 0, 16));
    
    // Start periodic updates
    m_updateTimer->start(5000); // Update every 5 seconds
    
    return true;
}

bool MockI2C::isConnected() const
{
    return m_isConnected && !m_simulateConnectionError;
}

uint8_t MockI2C::readRegister(uint8_t reg)
{
    if (!isConnected()) {
        LOG_ERROR("MockI2C", "Cannot read register - device not connected");
        return 0;
    }
    
    // Simulate different register values based on sensor data
    switch (reg) {
        case 0x00: // Temperature register
            return static_cast<uint8_t>(m_currentData.temperature * 2);
        case 0x01: // Humidity register
            return static_cast<uint8_t>(m_currentData.humidity * 2.55);
        case 0x02: // Pressure register (high byte)
            return static_cast<uint8_t>((static_cast<int>(m_currentData.pressure) >> 8) & 0xFF);
        case 0x03: // Pressure register (low byte)
            return static_cast<uint8_t>(static_cast<int>(m_currentData.pressure) & 0xFF);
        case 0x04: // Light register (high byte)
            return static_cast<uint8_t>((static_cast<int>(m_currentData.lightLevel) >> 8) & 0xFF);
        case 0x05: // Light register (low byte)
            return static_cast<uint8_t>(static_cast<int>(m_currentData.lightLevel) & 0xFF);
        default:
            return 0;
    }
}

bool MockI2C::writeRegister(uint8_t reg, uint8_t value)
{
    if (!isConnected()) {
        LOG_ERROR("MockI2C", "Cannot write register - device not connected");
        return false;
    }
    
    LOG_DEBUG("MockI2C", QString("Writing 0x%1 to register 0x%2").arg(value, 0, 16).arg(reg, 0, 16));
    return true;
}

SensorData MockI2C::getCurrentData() const
{
    return m_currentData;
}

double MockI2C::getTemperature() const
{
    return m_currentData.temperature;
}

double MockI2C::getHumidity() const
{
    return m_currentData.humidity;
}

double MockI2C::getPressure() const
{
    return m_currentData.pressure;
}

double MockI2C::getLightLevel() const
{
    return m_currentData.lightLevel;
}

void MockI2C::setUpdateInterval(int milliseconds)
{
    if (m_updateTimer->isActive()) {
        m_updateTimer->stop();
    }
    m_updateTimer->start(milliseconds);
    LOG_INFO("MockI2C", QString("Update interval set to %1 ms").arg(milliseconds));
}

void MockI2C::setTemperatureRange(double min, double max)
{
    m_tempMin = min;
    m_tempMax = max;
    m_tempDist = std::uniform_real_distribution<double>(min, max);
    LOG_INFO("MockI2C", QString("Temperature range set to %1-%2°C").arg(min).arg(max));
}

void MockI2C::setHumidityRange(double min, double max)
{
    m_humidityMin = min;
    m_humidityMax = max;
    m_humidityDist = std::uniform_real_distribution<double>(min, max);
    LOG_INFO("MockI2C", QString("Humidity range set to %1-%2%%").arg(min).arg(max));
}

void MockI2C::setPressureRange(double min, double max)
{
    m_pressureMin = min;
    m_pressureMax = max;
    m_pressureDist = std::uniform_real_distribution<double>(min, max);
    LOG_INFO("MockI2C", QString("Pressure range set to %1-%2 hPa").arg(min).arg(max));
}

void MockI2C::setLightRange(double min, double max)
{
    m_lightMin = min;
    m_lightMax = max;
    m_lightDist = std::uniform_real_distribution<double>(min, max);
    LOG_INFO("MockI2C", QString("Light range set to %1-%2 lux").arg(min).arg(max));
}

void MockI2C::simulateConnectionError(bool enable)
{
    m_simulateConnectionError = enable;
    if (enable) {
        LOG_WARNING("MockI2C", "Connection error simulation enabled");
        emit connectionError("Simulated I2C connection error");
    } else {
        LOG_INFO("MockI2C", "Connection error simulation disabled");
    }
}

void MockI2C::simulateSensorFailure(bool enable)
{
    m_simulateSensorFailure = enable;
    if (enable) {
        LOG_WARNING("MockI2C", "Sensor failure simulation enabled");
        emit sensorError("Simulated sensor failure");
    } else {
        LOG_INFO("MockI2C", "Sensor failure simulation disabled");
    }
}

void MockI2C::simulateDataCorruption(bool enable)
{
    m_simulateDataCorruption = enable;
    if (enable) {
        LOG_WARNING("MockI2C", "Data corruption simulation enabled");
    } else {
        LOG_INFO("MockI2C", "Data corruption simulation disabled");
    }
}

void MockI2C::calibrateTemperature(double offset)
{
    m_tempOffset = offset;
    LOG_INFO("MockI2C", QString("Temperature calibration offset set to %1°C").arg(offset));
    emit calibrationChanged();
}

void MockI2C::calibrateHumidity(double offset)
{
    m_humidityOffset = offset;
    LOG_INFO("MockI2C", QString("Humidity calibration offset set to %1%%").arg(offset));
    emit calibrationChanged();
}

void MockI2C::calibratePressure(double offset)
{
    m_pressureOffset = offset;
    LOG_INFO("MockI2C", QString("Pressure calibration offset set to %1 hPa").arg(offset));
    emit calibrationChanged();
}

void MockI2C::calibrateLight(double offset)
{
    m_lightOffset = offset;
    LOG_INFO("MockI2C", QString("Light calibration offset set to %1 lux").arg(offset));
    emit calibrationChanged();
}

void MockI2C::enableDataLogging(bool enable)
{
    m_dataLoggingEnabled = enable;
    LOG_INFO("MockI2C", QString("Data logging %1").arg(enable ? "enabled" : "disabled"));
}

void MockI2C::saveCalibrationData()
{
    QJsonObject config;
    config["temperature_offset"] = m_tempOffset;
    config["humidity_offset"] = m_humidityOffset;
    config["pressure_offset"] = m_pressureOffset;
    config["light_offset"] = m_lightOffset;
    config["temperature_range"] = QJsonArray{m_tempMin, m_tempMax};
    config["humidity_range"] = QJsonArray{m_humidityMin, m_humidityMax};
    config["pressure_range"] = QJsonArray{m_pressureMin, m_pressureMax};
    config["light_range"] = QJsonArray{m_lightMin, m_lightMax};
    
    QFile file(CONFIG_FILE);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(config).toJson());
        LOG_INFO("MockI2C", "Calibration data saved");
    } else {
        LOG_ERROR("MockI2C", "Failed to save calibration data");
    }
}

void MockI2C::loadCalibrationData()
{
    QFile file(CONFIG_FILE);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonObject config = doc.object();
        
        m_tempOffset = config["temperature_offset"].toDouble(0.0);
        m_humidityOffset = config["humidity_offset"].toDouble(0.0);
        m_pressureOffset = config["pressure_offset"].toDouble(0.0);
        m_lightOffset = config["light_offset"].toDouble(0.0);
        
        QJsonArray tempRange = config["temperature_range"].toArray();
        if (tempRange.size() == 2) {
            m_tempMin = tempRange[0].toDouble(18.0);
            m_tempMax = tempRange[1].toDouble(25.0);
            m_tempDist = std::uniform_real_distribution<double>(m_tempMin, m_tempMax);
        }
        
        LOG_INFO("MockI2C", "Calibration data loaded");
    }
}

void MockI2C::updateSensorData()
{
    if (m_simulateSensorFailure) {
        m_currentData.isValid = false;
        LOG_ERROR("MockI2C", "Sensor failure detected - invalid data");
        emit sensorError("Sensor failure - invalid readings");
        return;
    }
    
    generateRandomData();
    applyCalibration();
    
    m_currentData.timestamp = getCurrentTimestamp();
    m_currentData.isValid = true;
    
    if (m_dataLoggingEnabled) {
        logData();
    }
    
    LOG_DEBUG("MockI2C", QString("Sensor data updated: T=%1°C, H=%2%%, P=%3 hPa, L=%4 lux")
              .arg(m_currentData.temperature, 0, 'f', 1)
              .arg(m_currentData.humidity, 0, 'f', 1)
              .arg(m_currentData.pressure, 0, 'f', 1)
              .arg(m_currentData.lightLevel, 0, 'f', 0));
    
    emit dataUpdated(m_currentData);
}

void MockI2C::generateRandomData()
{
    m_currentData.temperature = m_tempDist(m_randomGenerator);
    m_currentData.humidity = m_humidityDist(m_randomGenerator);
    m_currentData.pressure = m_pressureDist(m_randomGenerator);
    m_currentData.lightLevel = m_lightDist(m_randomGenerator);
    
    if (m_simulateDataCorruption) {
        // Add some noise to simulate data corruption
        m_currentData.temperature += (m_randomGenerator() % 100 - 50) * 0.1;
        m_currentData.humidity += (m_randomGenerator() % 100 - 50) * 0.1;
    }
}

void MockI2C::applyCalibration()
{
    m_currentData.temperature += m_tempOffset;
    m_currentData.humidity += m_humidityOffset;
    m_currentData.pressure += m_pressureOffset;
    m_currentData.lightLevel += m_lightOffset;
    
    // Clamp values to reasonable ranges
    m_currentData.temperature = std::clamp(m_currentData.temperature, -40.0, 80.0);
    m_currentData.humidity = std::clamp(m_currentData.humidity, 0.0, 100.0);
    m_currentData.pressure = std::clamp(m_currentData.pressure, 800.0, 1200.0);
    m_currentData.lightLevel = std::clamp(m_currentData.lightLevel, 0.0, 10000.0);
}

void MockI2C::logData()
{
    QJsonObject dataPoint;
    dataPoint["timestamp"] = m_currentData.timestamp;
    dataPoint["temperature"] = m_currentData.temperature;
    dataPoint["humidity"] = m_currentData.humidity;
    dataPoint["pressure"] = m_currentData.pressure;
    dataPoint["light_level"] = m_currentData.lightLevel;
    
    // Append to log file
    QFile logFile("config/sensor_data.json");
    QJsonArray dataArray;
    
    if (logFile.exists() && logFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(logFile.readAll());
        dataArray = doc.array();
        logFile.close();
    }
    
    dataArray.append(dataPoint);
    
    if (logFile.open(QIODevice::WriteOnly)) {
        logFile.write(QJsonDocument(dataArray).toJson());
    }
}

QString MockI2C::getCurrentTimestamp() const
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
} 