#ifndef MOCKI2C_H
#define MOCKI2C_H

#include <QObject>
#include <QTimer>
#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <random>
#include <memory>

struct SensorData {
    double temperature;    // Celsius
    double humidity;       // Percentage
    double pressure;       // hPa
    double lightLevel;     // lux
    bool isValid;
    QString timestamp;
};

class MockI2C : public QObject
{
    Q_OBJECT

public:
    static MockI2C& getInstance();
    
    // I2C-like interface
    bool begin(uint8_t address = 0x48);
    bool isConnected() const;
    uint8_t readRegister(uint8_t reg);
    bool writeRegister(uint8_t reg, uint8_t value);
    
    // Sensor data access
    SensorData getCurrentData() const;
    double getTemperature() const;
    double getHumidity() const;
    double getPressure() const;
    double getLightLevel() const;
    
    // Configuration
    void setUpdateInterval(int milliseconds);
    void setTemperatureRange(double min, double max);
    void setHumidityRange(double min, double max);
    void setPressureRange(double min, double max);
    void setLightRange(double min, double max);
    
    // Error simulation
    void simulateConnectionError(bool enable);
    void simulateSensorFailure(bool enable);
    void simulateDataCorruption(bool enable);
    
    // Calibration
    void calibrateTemperature(double offset);
    void calibrateHumidity(double offset);
    void calibratePressure(double offset);
    void calibrateLight(double offset);
    
    // Data logging
    void enableDataLogging(bool enable);
    void saveCalibrationData();
    void loadCalibrationData();

signals:
    void dataUpdated(const SensorData& data);
    void connectionError(const QString& error);
    void sensorError(const QString& error);
    void calibrationChanged();

private:
    MockI2C();
    ~MockI2C();
    MockI2C(const MockI2C&) = delete;
    MockI2C& operator=(const MockI2C&) = delete;
    
    void updateSensorData();
    void generateRandomData();
    void applyCalibration();
    void logData();
    QString getCurrentTimestamp() const;
    
    std::unique_ptr<QTimer> m_updateTimer;
    std::mt19937 m_randomGenerator;
    std::uniform_real_distribution<double> m_tempDist;
    std::uniform_real_distribution<double> m_humidityDist;
    std::uniform_real_distribution<double> m_pressureDist;
    std::uniform_real_distribution<double> m_lightDist;
    
    SensorData m_currentData;
    bool m_isConnected;
    bool m_simulateConnectionError;
    bool m_simulateSensorFailure;
    bool m_simulateDataCorruption;
    bool m_dataLoggingEnabled;
    
    // Calibration offsets
    double m_tempOffset;
    double m_humidityOffset;
    double m_pressureOffset;
    double m_lightOffset;
    
    // Configuration ranges
    double m_tempMin, m_tempMax;
    double m_humidityMin, m_humidityMax;
    double m_pressureMin, m_pressureMax;
    double m_lightMin, m_lightMax;
    
    static const QString CONFIG_FILE;
};

#endif // MOCKI2C_H 