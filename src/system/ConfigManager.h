#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QMap>
#include <memory>

struct UserSettings {
    // Media settings
    QString lastPlayedSong;
    QString lastPlayedDevice;
    int volumeLevel;
    bool shuffleEnabled;
    bool repeatEnabled;
    QString equalizerPreset;
    
    // Climate settings
    double preferredTemperature;
    double preferredHumidity;
    bool autoClimateEnabled;
    QString climateMode; // "auto", "manual", "eco"
    
    // Bluetooth settings
    QString lastConnectedDevice;
    bool autoConnectEnabled;
    bool discoverableEnabled;
    int discoveryTimeout;
    
    // Display settings
    int brightnessLevel;
    QString theme; // "light", "dark", "auto"
    bool nightModeEnabled;
    int screenTimeout;
    
    // System settings
    QString language;
    QString timezone;
    bool debugModeEnabled;
    QString logLevel;
    bool autoUpdateEnabled;
    
    // Navigation settings
    QString homeAddress;
    QString workAddress;
    bool trafficEnabled;
    QString mapProvider; // "google", "here", "openstreetmap"
    
    // Vehicle settings
    QString vehicleModel;
    QString vinNumber;
    bool diagnosticModeEnabled;
    QString firmwareVersion;
};

class ConfigManager : public QObject
{
    Q_OBJECT

public:
    static ConfigManager& getInstance();
    
    // Configuration management
    bool loadConfiguration();
    bool saveConfiguration();
    void resetToDefaults();
    
    // Settings access
    QVariant getSetting(const QString& key, const QVariant& defaultValue = QVariant()) const;
    void setSetting(const QString& key, const QVariant& value);
    bool hasSetting(const QString& key) const;
    void removeSetting(const QString& key);
    
    // User settings
    UserSettings getUserSettings() const;
    void setUserSettings(const UserSettings& settings);
    void updateUserSetting(const QString& key, const QVariant& value);
    
    // Configuration validation
    bool validateConfiguration() const;
    QStringList getConfigurationErrors() const;
    
    // Backup and restore
    bool backupConfiguration(const QString& backupPath);
    bool restoreConfiguration(const QString& backupPath);
    QStringList getBackupFiles() const;
    
    // System information
    QString getSystemInfo() const;
    QString getVersionInfo() const;
    QString getBuildInfo() const;
    
    // Configuration categories
    QStringList getConfigurationCategories() const;
    QJsonObject getCategorySettings(const QString& category) const;
    void setCategorySettings(const QString& category, const QJsonObject& settings);
    
    // Environment-specific settings
    void setEnvironment(const QString& environment); // "development", "testing", "production"
    QString getCurrentEnvironment() const;
    bool isDevelopmentMode() const;
    bool isProductionMode() const;

signals:
    void configurationLoaded();
    void configurationSaved();
    void settingChanged(const QString& key, const QVariant& value);
    void userSettingsChanged(const UserSettings& settings);
    void configurationError(const QString& error);

private:
    ConfigManager();
    ~ConfigManager();
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    void initializeDefaultSettings();
    void loadUserSettings();
    void saveUserSettings();
    void validateUserSettings();
    QString getConfigFilePath() const;
    QString getBackupDirectory() const;
    void createBackupDirectory();
    QJsonObject settingsToJson(const UserSettings& settings) const;
    UserSettings jsonToSettings(const QJsonObject& json) const;
    
    QSettings m_settings;
    UserSettings m_userSettings;
    QMap<QString, QVariant> m_configCache;
    QString m_currentEnvironment;
    QString m_configFilePath;
    QString m_backupDirectory;
    
    static const QString CONFIG_FILE;
    static const QString BACKUP_DIR;
    static const QString DEFAULT_CONFIG_FILE;
};

#endif // CONFIGMANAGER_H 