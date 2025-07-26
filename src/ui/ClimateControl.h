#ifndef CLIMATECONTROL_H
#define CLIMATECONTROL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QProgressBar>
#include <QGroupBox>
#include <QFrame>
#include <QTimer>
#include <QMessageBox>
#include <QInputDialog>
#include <QMenu>
#include <QAction>
#include <QStyle>
#include <QApplication>
#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>

#include "../system/Logger.h"
#include "../system/MockI2C.h"

class ClimateControl : public QWidget
{
    Q_OBJECT

public:
    explicit ClimateControl(QWidget *parent = nullptr);
    ~ClimateControl();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateSensorData();
    void setTargetTemperature(int temperature);
    void setTargetHumidity(int humidity);
    void toggleAutoMode();
    void toggleHeating();
    void toggleCooling();
    void toggleFan();
    void setFanSpeed(int speed);
    void setClimateMode(const QString& mode);
    void calibrateSensors();
    void showClimateSettings();
    void onSensorDataUpdated(const SensorData& data);
    void onConnectionError(const QString& error);
    void onSensorError(const QString& error);
    void onCalibrationChanged();
    void simulateSensorFailure();
    void simulateConnectionError();
    void resetToDefaults();

private:
    void setupUI();
    void setupTemperatureControls();
    void setupHumidityControls();
    void setupFanControls();
    void setupSensorDisplay();
    void setupModeControls();
    void updateDisplays();
    void updateClimateStatus();
    void updateSensorDisplays();
    void showErrorMessage(const QString& message);
    void showInfoMessage(const QString& message);
    QString getClimateModeString(const QString& mode);
    QColor getTemperatureColor(double temperature);
    QColor getHumidityColor(double humidity);
    void createTemperatureGradient();
    void createHumidityGradient();
    
    // UI Components
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_controlsLayout;
    QHBoxLayout *m_sensorLayout;
    
    // Temperature controls
    QSlider *m_targetTemperatureSlider;
    QLabel *m_targetTemperatureLabel;
    QLabel *m_currentTemperatureLabel;
    QProgressBar *m_temperatureBar;
    
    // Humidity controls
    QSlider *m_targetHumiditySlider;
    QLabel *m_targetHumidityLabel;
    QLabel *m_currentHumidityLabel;
    QProgressBar *m_humidityBar;
    
    // Fan controls
    QSlider *m_fanSpeedSlider;
    QLabel *m_fanSpeedLabel;
    QPushButton *m_fanButton;
    QPushButton *m_heatingButton;
    QPushButton *m_coolingButton;
    
    // Mode controls
    QPushButton *m_autoModeButton;
    QPushButton *m_manualModeButton;
    QPushButton *m_ecoModeButton;
    
    // Sensor displays
    QLabel *m_pressureLabel;
    QLabel *m_lightLabel;
    QLabel *m_sensorStatusLabel;
    QLabel *m_calibrationLabel;
    
    // Status displays
    QLabel *m_climateStatusLabel;
    QLabel *m_systemStatusLabel;
    QProgressBar *m_systemStatusBar;
    
    // Control buttons
    QPushButton *m_calibrateButton;
    QPushButton *m_settingsButton;
    QPushButton *m_simulateErrorButton;
    QPushButton *m_resetButton;
    
    // Mock I2C sensor
    MockI2C *m_mockI2C;
    
    // Timers
    QTimer *m_updateTimer;
    
    // State
    double m_targetTemperature;
    double m_targetHumidity;
    int m_fanSpeed;
    bool m_autoModeEnabled;
    bool m_heatingEnabled;
    bool m_coolingEnabled;
    bool m_fanEnabled;
    QString m_climateMode;
    SensorData m_currentSensorData;
    bool m_sensorConnected;
    
    // Gradients for visual effects
    QLinearGradient m_temperatureGradient;
    QLinearGradient m_humidityGradient;
    
    // Constants
    static const int UPDATE_INTERVAL = 1000; // 1 second
    static const double MIN_TEMPERATURE = -10.0;
    static const double MAX_TEMPERATURE = 40.0;
    static const double MIN_HUMIDITY = 0.0;
    static const double MAX_HUMIDITY = 100.0;
    static const int MIN_FAN_SPEED = 0;
    static const int MAX_FAN_SPEED = 10;
};

#endif // CLIMATECONTROL_H 