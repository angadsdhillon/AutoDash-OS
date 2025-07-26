#include "ConfigManager.h"
#include "Logger.h"
#include <QStandardPaths>
#include <QDateTime>
#include <QJsonArray>
#include <QDebug>
#include <QApplication>
#include <QSysInfo>

const QString ConfigManager::CONFIG_FILE = "config/autodash_config.json";
const QString ConfigManager::BACKUP_DIR = "config/backups";
const QString ConfigManager::DEFAULT_CONFIG_FILE = "config/default_config.json";

ConfigManager::ConfigManager()
    : m_settings("AutoDash", "AutoDash-OS")
    , m_currentEnvironment("development")
{
    m_configFilePath = getConfigFilePath();
    m_backupDirectory = getBackupDirectory();
    
    initializeDefaultSettings();
    loadUserSettings();
    
    LOG_INFO("ConfigManager", "Configuration manager initialized");
}

ConfigManager::~ConfigManager()
{
    saveConfiguration();
    LOG_INFO("ConfigManager", "Configuration manager shutdown");
}

ConfigManager& ConfigManager::getInstance()
{
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::loadConfiguration()
{
    QFile file(m_configFilePath);
    if (!file.exists()) {
        LOG_WARNING("ConfigManager", "Configuration file not found, creating default configuration");
        saveConfiguration();
        return true;
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR("ConfigManager", QString("Failed to open configuration file: %1").arg(m_configFilePath));
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull()) {
        LOG_ERROR("ConfigManager", "Invalid JSON in configuration file");
        return false;
    }
    
    QJsonObject config = doc.object();
    
    // Load general settings
    QJsonObject generalSettings = config["general"].toObject();
    for (auto it = generalSettings.begin(); it != generalSettings.end(); ++it) {
        m_configCache[it.key()] = it.value().toVariant();
    }
    
    // Load user settings
    if (config.contains("user_settings")) {
        QJsonObject userSettingsObj = config["user_settings"].toObject();
        m_userSettings = jsonToSettings(userSettingsObj);
    }
    
    // Load environment-specific settings
    if (config.contains("environments")) {
        QJsonObject environments = config["environments"].toObject();
        if (environments.contains(m_currentEnvironment)) {
            QJsonObject envSettings = environments[m_currentEnvironment].toObject();
            for (auto it = envSettings.begin(); it != envSettings.end(); ++it) {
                m_configCache[it.key()] = it.value().toVariant();
            }
        }
    }
    
    validateUserSettings();
    
    LOG_INFO("ConfigManager", "Configuration loaded successfully");
    emit configurationLoaded();
    return true;
}

bool ConfigManager::saveConfiguration()
{
    QJsonObject config;
    
    // Save general settings
    QJsonObject generalSettings;
    for (auto it = m_configCache.begin(); it != m_configCache.end(); ++it) {
        generalSettings[it.key()] = QJsonValue::fromVariant(it.value());
    }
    config["general"] = generalSettings;
    
    // Save user settings
    config["user_settings"] = settingsToJson(m_userSettings);
    
    // Save environment-specific settings
    QJsonObject environments;
    QJsonObject envSettings;
    for (auto it = m_configCache.begin(); it != m_configCache.end(); ++it) {
        if (it.key().startsWith(m_currentEnvironment + ".")) {
            QString key = it.key().mid(m_currentEnvironment.length() + 1);
            envSettings[key] = QJsonValue::fromVariant(it.value());
        }
    }
    environments[m_currentEnvironment] = envSettings;
    config["environments"] = environments;
    
    // Add metadata
    config["last_modified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    config["version"] = "1.0.0";
    config["environment"] = m_currentEnvironment;
    
    QFile file(m_configFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR("ConfigManager", QString("Failed to save configuration file: %1").arg(m_configFilePath));
        return false;
    }
    
    file.write(QJsonDocument(config).toJson());
    
    LOG_INFO("ConfigManager", "Configuration saved successfully");
    emit configurationSaved();
    return true;
}

void ConfigManager::resetToDefaults()
{
    m_configCache.clear();
    initializeDefaultSettings();
    loadUserSettings();
    
    LOG_INFO("ConfigManager", "Configuration reset to defaults");
}

QVariant ConfigManager::getSetting(const QString& key, const QVariant& defaultValue) const
{
    if (m_configCache.contains(key)) {
        return m_configCache[key];
    }
    
    // Fallback to QSettings
    return m_settings.value(key, defaultValue);
}

void ConfigManager::setSetting(const QString& key, const QVariant& value)
{
    m_configCache[key] = value;
    m_settings.setValue(key, value);
    
    LOG_DEBUG("ConfigManager", QString("Setting updated: %1 = %2").arg(key).arg(value.toString()));
    emit settingChanged(key, value);
}

bool ConfigManager::hasSetting(const QString& key) const
{
    return m_configCache.contains(key) || m_settings.contains(key);
}

void ConfigManager::removeSetting(const QString& key)
{
    m_configCache.remove(key);
    m_settings.remove(key);
    
    LOG_DEBUG("ConfigManager", QString("Setting removed: %1").arg(key));
}

UserSettings ConfigManager::getUserSettings() const
{
    return m_userSettings;
}

void ConfigManager::setUserSettings(const UserSettings& settings)
{
    m_userSettings = settings;
    saveUserSettings();
    
    LOG_INFO("ConfigManager", "User settings updated");
    emit userSettingsChanged(settings);
}

void ConfigManager::updateUserSetting(const QString& key, const QVariant& value)
{
    // Update the appropriate field in UserSettings
    if (key == "lastPlayedSong") {
        m_userSettings.lastPlayedSong = value.toString();
    } else if (key == "lastPlayedDevice") {
        m_userSettings.lastPlayedDevice = value.toString();
    } else if (key == "volumeLevel") {
        m_userSettings.volumeLevel = value.toInt();
    } else if (key == "shuffleEnabled") {
        m_userSettings.shuffleEnabled = value.toBool();
    } else if (key == "repeatEnabled") {
        m_userSettings.repeatEnabled = value.toBool();
    } else if (key == "equalizerPreset") {
        m_userSettings.equalizerPreset = value.toString();
    } else if (key == "preferredTemperature") {
        m_userSettings.preferredTemperature = value.toDouble();
    } else if (key == "preferredHumidity") {
        m_userSettings.preferredHumidity = value.toDouble();
    } else if (key == "autoClimateEnabled") {
        m_userSettings.autoClimateEnabled = value.toBool();
    } else if (key == "climateMode") {
        m_userSettings.climateMode = value.toString();
    } else if (key == "lastConnectedDevice") {
        m_userSettings.lastConnectedDevice = value.toString();
    } else if (key == "autoConnectEnabled") {
        m_userSettings.autoConnectEnabled = value.toBool();
    } else if (key == "discoverableEnabled") {
        m_userSettings.discoverableEnabled = value.toBool();
    } else if (key == "discoveryTimeout") {
        m_userSettings.discoveryTimeout = value.toInt();
    } else if (key == "brightnessLevel") {
        m_userSettings.brightnessLevel = value.toInt();
    } else if (key == "theme") {
        m_userSettings.theme = value.toString();
    } else if (key == "nightModeEnabled") {
        m_userSettings.nightModeEnabled = value.toBool();
    } else if (key == "screenTimeout") {
        m_userSettings.screenTimeout = value.toInt();
    } else if (key == "language") {
        m_userSettings.language = value.toString();
    } else if (key == "timezone") {
        m_userSettings.timezone = value.toString();
    } else if (key == "debugModeEnabled") {
        m_userSettings.debugModeEnabled = value.toBool();
    } else if (key == "logLevel") {
        m_userSettings.logLevel = value.toString();
    } else if (key == "autoUpdateEnabled") {
        m_userSettings.autoUpdateEnabled = value.toBool();
    } else if (key == "homeAddress") {
        m_userSettings.homeAddress = value.toString();
    } else if (key == "workAddress") {
        m_userSettings.workAddress = value.toString();
    } else if (key == "trafficEnabled") {
        m_userSettings.trafficEnabled = value.toBool();
    } else if (key == "mapProvider") {
        m_userSettings.mapProvider = value.toString();
    } else if (key == "vehicleModel") {
        m_userSettings.vehicleModel = value.toString();
    } else if (key == "vinNumber") {
        m_userSettings.vinNumber = value.toString();
    } else if (key == "diagnosticModeEnabled") {
        m_userSettings.diagnosticModeEnabled = value.toBool();
    } else if (key == "firmwareVersion") {
        m_userSettings.firmwareVersion = value.toString();
    }
    
    saveUserSettings();
    emit userSettingsChanged(m_userSettings);
}

bool ConfigManager::validateConfiguration() const
{
    QStringList errors = getConfigurationErrors();
    return errors.isEmpty();
}

QStringList ConfigManager::getConfigurationErrors() const
{
    QStringList errors;
    
    // Validate user settings
    if (m_userSettings.volumeLevel < 0 || m_userSettings.volumeLevel > 100) {
        errors << "Invalid volume level (must be 0-100)";
    }
    
    if (m_userSettings.preferredTemperature < -40 || m_userSettings.preferredTemperature > 50) {
        errors << "Invalid preferred temperature (must be -40 to 50°C)";
    }
    
    if (m_userSettings.preferredHumidity < 0 || m_userSettings.preferredHumidity > 100) {
        errors << "Invalid preferred humidity (must be 0-100%)";
    }
    
    if (m_userSettings.brightnessLevel < 0 || m_userSettings.brightnessLevel > 100) {
        errors << "Invalid brightness level (must be 0-100)";
    }
    
    if (m_userSettings.screenTimeout < 0 || m_userSettings.screenTimeout > 3600) {
        errors << "Invalid screen timeout (must be 0-3600 seconds)";
    }
    
    if (m_userSettings.discoveryTimeout < 5 || m_userSettings.discoveryTimeout > 300) {
        errors << "Invalid discovery timeout (must be 5-300 seconds)";
    }
    
    return errors;
}

bool ConfigManager::backupConfiguration(const QString& backupPath)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString backupFile = QString("%1/autodash_backup_%2.json").arg(backupPath).arg(timestamp);
    
    QFile file(backupFile);
    if (!file.open(QIODevice::WriteOnly)) {
        LOG_ERROR("ConfigManager", QString("Failed to create backup file: %1").arg(backupFile));
        return false;
    }
    
    QJsonObject backup;
    backup["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    backup["version"] = "1.0.0";
    backup["environment"] = m_currentEnvironment;
    backup["user_settings"] = settingsToJson(m_userSettings);
    backup["general_settings"] = QJsonObject::fromVariantMap(m_configCache);
    
    file.write(QJsonDocument(backup).toJson());
    
    LOG_INFO("ConfigManager", QString("Configuration backed up to: %1").arg(backupFile));
    return true;
}

bool ConfigManager::restoreConfiguration(const QString& backupPath)
{
    QFile file(backupPath);
    if (!file.open(QIODevice::ReadOnly)) {
        LOG_ERROR("ConfigManager", QString("Failed to open backup file: %1").arg(backupPath));
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isNull()) {
        LOG_ERROR("ConfigManager", "Invalid backup file format");
        return false;
    }
    
    QJsonObject backup = doc.object();
    
    // Restore user settings
    if (backup.contains("user_settings")) {
        m_userSettings = jsonToSettings(backup["user_settings"].toObject());
    }
    
    // Restore general settings
    if (backup.contains("general_settings")) {
        QJsonObject generalSettings = backup["general_settings"].toObject();
        for (auto it = generalSettings.begin(); it != generalSettings.end(); ++it) {
            m_configCache[it.key()] = it.value().toVariant();
        }
    }
    
    saveConfiguration();
    
    LOG_INFO("ConfigManager", QString("Configuration restored from: %1").arg(backupPath));
    return true;
}

QStringList ConfigManager::getBackupFiles() const
{
    QStringList backupFiles;
    QDir backupDir(m_backupDirectory);
    
    if (backupDir.exists()) {
        QStringList filters;
        filters << "autodash_backup_*.json";
        backupFiles = backupDir.entryList(filters, QDir::Files, QDir::Time);
    }
    
    return backupFiles;
}

QString ConfigManager::getSystemInfo() const
{
    QString info;
    info += QString("OS: %1\n").arg(QSysInfo::prettyProductName());
    info += QString("Architecture: %1\n").arg(QSysInfo::currentCpuArchitecture());
    info += QString("Kernel: %1\n").arg(QSysInfo::kernelType());
    info += QString("Version: %1\n").arg(QSysInfo::kernelVersion());
    info += QString("Machine: %1\n").arg(QSysInfo::machineHostName());
    info += QString("Environment: %1\n").arg(m_currentEnvironment);
    
    return info;
}

QString ConfigManager::getVersionInfo() const
{
    return "AutoDash OS v1.0.0";
}

QString ConfigManager::getBuildInfo() const
{
    QString buildInfo;
    buildInfo += QString("Build Date: %1\n").arg(__DATE__);
    buildInfo += QString("Build Time: %1\n").arg(__TIME__);
    buildInfo += QString("Compiler: %1\n").arg(QString(__VERSION__));
    buildInfo += QString("Qt Version: %1\n").arg(QT_VERSION_STR);
    
    return buildInfo;
}

QStringList ConfigManager::getConfigurationCategories() const
{
    return {"media", "climate", "bluetooth", "display", "system", "navigation", "vehicle"};
}

QJsonObject ConfigManager::getCategorySettings(const QString& category) const
{
    QJsonObject settings;
    
    if (category == "media") {
        settings["lastPlayedSong"] = m_userSettings.lastPlayedSong;
        settings["lastPlayedDevice"] = m_userSettings.lastPlayedDevice;
        settings["volumeLevel"] = m_userSettings.volumeLevel;
        settings["shuffleEnabled"] = m_userSettings.shuffleEnabled;
        settings["repeatEnabled"] = m_userSettings.repeatEnabled;
        settings["equalizerPreset"] = m_userSettings.equalizerPreset;
    } else if (category == "climate") {
        settings["preferredTemperature"] = m_userSettings.preferredTemperature;
        settings["preferredHumidity"] = m_userSettings.preferredHumidity;
        settings["autoClimateEnabled"] = m_userSettings.autoClimateEnabled;
        settings["climateMode"] = m_userSettings.climateMode;
    } else if (category == "bluetooth") {
        settings["lastConnectedDevice"] = m_userSettings.lastConnectedDevice;
        settings["autoConnectEnabled"] = m_userSettings.autoConnectEnabled;
        settings["discoverableEnabled"] = m_userSettings.discoverableEnabled;
        settings["discoveryTimeout"] = m_userSettings.discoveryTimeout;
    } else if (category == "display") {
        settings["brightnessLevel"] = m_userSettings.brightnessLevel;
        settings["theme"] = m_userSettings.theme;
        settings["nightModeEnabled"] = m_userSettings.nightModeEnabled;
        settings["screenTimeout"] = m_userSettings.screenTimeout;
    } else if (category == "system") {
        settings["language"] = m_userSettings.language;
        settings["timezone"] = m_userSettings.timezone;
        settings["debugModeEnabled"] = m_userSettings.debugModeEnabled;
        settings["logLevel"] = m_userSettings.logLevel;
        settings["autoUpdateEnabled"] = m_userSettings.autoUpdateEnabled;
    } else if (category == "navigation") {
        settings["homeAddress"] = m_userSettings.homeAddress;
        settings["workAddress"] = m_userSettings.workAddress;
        settings["trafficEnabled"] = m_userSettings.trafficEnabled;
        settings["mapProvider"] = m_userSettings.mapProvider;
    } else if (category == "vehicle") {
        settings["vehicleModel"] = m_userSettings.vehicleModel;
        settings["vinNumber"] = m_userSettings.vinNumber;
        settings["diagnosticModeEnabled"] = m_userSettings.diagnosticModeEnabled;
        settings["firmwareVersion"] = m_userSettings.firmwareVersion;
    }
    
    return settings;
}

void ConfigManager::setCategorySettings(const QString& category, const QJsonObject& settings)
{
    if (category == "media") {
        m_userSettings.lastPlayedSong = settings["lastPlayedSong"].toString();
        m_userSettings.lastPlayedDevice = settings["lastPlayedDevice"].toString();
        m_userSettings.volumeLevel = settings["volumeLevel"].toInt();
        m_userSettings.shuffleEnabled = settings["shuffleEnabled"].toBool();
        m_userSettings.repeatEnabled = settings["repeatEnabled"].toBool();
        m_userSettings.equalizerPreset = settings["equalizerPreset"].toString();
    } else if (category == "climate") {
        m_userSettings.preferredTemperature = settings["preferredTemperature"].toDouble();
        m_userSettings.preferredHumidity = settings["preferredHumidity"].toDouble();
        m_userSettings.autoClimateEnabled = settings["autoClimateEnabled"].toBool();
        m_userSettings.climateMode = settings["climateMode"].toString();
    } else if (category == "bluetooth") {
        m_userSettings.lastConnectedDevice = settings["lastConnectedDevice"].toString();
        m_userSettings.autoConnectEnabled = settings["autoConnectEnabled"].toBool();
        m_userSettings.discoverableEnabled = settings["discoverableEnabled"].toBool();
        m_userSettings.discoveryTimeout = settings["discoveryTimeout"].toInt();
    } else if (category == "display") {
        m_userSettings.brightnessLevel = settings["brightnessLevel"].toInt();
        m_userSettings.theme = settings["theme"].toString();
        m_userSettings.nightModeEnabled = settings["nightModeEnabled"].toBool();
        m_userSettings.screenTimeout = settings["screenTimeout"].toInt();
    } else if (category == "system") {
        m_userSettings.language = settings["language"].toString();
        m_userSettings.timezone = settings["timezone"].toString();
        m_userSettings.debugModeEnabled = settings["debugModeEnabled"].toBool();
        m_userSettings.logLevel = settings["logLevel"].toString();
        m_userSettings.autoUpdateEnabled = settings["autoUpdateEnabled"].toBool();
    } else if (category == "navigation") {
        m_userSettings.homeAddress = settings["homeAddress"].toString();
        m_userSettings.workAddress = settings["workAddress"].toString();
        m_userSettings.trafficEnabled = settings["trafficEnabled"].toBool();
        m_userSettings.mapProvider = settings["mapProvider"].toString();
    } else if (category == "vehicle") {
        m_userSettings.vehicleModel = settings["vehicleModel"].toString();
        m_userSettings.vinNumber = settings["vinNumber"].toString();
        m_userSettings.diagnosticModeEnabled = settings["diagnosticModeEnabled"].toBool();
        m_userSettings.firmwareVersion = settings["firmwareVersion"].toString();
    }
    
    saveUserSettings();
    emit userSettingsChanged(m_userSettings);
}

void ConfigManager::setEnvironment(const QString& environment)
{
    m_currentEnvironment = environment;
    LOG_INFO("ConfigManager", QString("Environment set to: %1").arg(environment));
}

QString ConfigManager::getCurrentEnvironment() const
{
    return m_currentEnvironment;
}

bool ConfigManager::isDevelopmentMode() const
{
    return m_currentEnvironment == "development";
}

bool ConfigManager::isProductionMode() const
{
    return m_currentEnvironment == "production";
}

void ConfigManager::initializeDefaultSettings()
{
    // Set default values
    setSetting("app_name", "AutoDash OS");
    setSetting("app_version", "1.0.0");
    setSetting("debug_enabled", true);
    setSetting("log_level", "INFO");
    setSetting("auto_save_interval", 300); // 5 minutes
    setSetting("max_log_size", 10485760); // 10MB
    setSetting("backup_enabled", true);
    setSetting("backup_interval", 86400); // 24 hours
}

void ConfigManager::loadUserSettings()
{
    // Load from QSettings
    m_userSettings.lastPlayedSong = m_settings.value("media/lastPlayedSong", "").toString();
    m_userSettings.lastPlayedDevice = m_settings.value("media/lastPlayedDevice", "").toString();
    m_userSettings.volumeLevel = m_settings.value("media/volumeLevel", 50).toInt();
    m_userSettings.shuffleEnabled = m_settings.value("media/shuffleEnabled", false).toBool();
    m_userSettings.repeatEnabled = m_settings.value("media/repeatEnabled", false).toBool();
    m_userSettings.equalizerPreset = m_settings.value("media/equalizerPreset", "normal").toString();
    
    m_userSettings.preferredTemperature = m_settings.value("climate/preferredTemperature", 22.0).toDouble();
    m_userSettings.preferredHumidity = m_settings.value("climate/preferredHumidity", 50.0).toDouble();
    m_userSettings.autoClimateEnabled = m_settings.value("climate/autoClimateEnabled", true).toBool();
    m_userSettings.climateMode = m_settings.value("climate/climateMode", "auto").toString();
    
    m_userSettings.lastConnectedDevice = m_settings.value("bluetooth/lastConnectedDevice", "").toString();
    m_userSettings.autoConnectEnabled = m_settings.value("bluetooth/autoConnectEnabled", true).toBool();
    m_userSettings.discoverableEnabled = m_settings.value("bluetooth/discoverableEnabled", false).toBool();
    m_userSettings.discoveryTimeout = m_settings.value("bluetooth/discoveryTimeout", 30).toInt();
    
    m_userSettings.brightnessLevel = m_settings.value("display/brightnessLevel", 80).toInt();
    m_userSettings.theme = m_settings.value("display/theme", "auto").toString();
    m_userSettings.nightModeEnabled = m_settings.value("display/nightModeEnabled", false).toBool();
    m_userSettings.screenTimeout = m_settings.value("display/screenTimeout", 300).toInt();
    
    m_userSettings.language = m_settings.value("system/language", "en_US").toString();
    m_userSettings.timezone = m_settings.value("system/timezone", "UTC").toString();
    m_userSettings.debugModeEnabled = m_settings.value("system/debugModeEnabled", false).toBool();
    m_userSettings.logLevel = m_settings.value("system/logLevel", "INFO").toString();
    m_userSettings.autoUpdateEnabled = m_settings.value("system/autoUpdateEnabled", true).toBool();
    
    m_userSettings.homeAddress = m_settings.value("navigation/homeAddress", "").toString();
    m_userSettings.workAddress = m_settings.value("navigation/workAddress", "").toString();
    m_userSettings.trafficEnabled = m_settings.value("navigation/trafficEnabled", true).toBool();
    m_userSettings.mapProvider = m_settings.value("navigation/mapProvider", "google").toString();
    
    m_userSettings.vehicleModel = m_settings.value("vehicle/vehicleModel", "Rivian R1T").toString();
    m_userSettings.vinNumber = m_settings.value("vehicle/vinNumber", "").toString();
    m_userSettings.diagnosticModeEnabled = m_settings.value("vehicle/diagnosticModeEnabled", false).toBool();
    m_userSettings.firmwareVersion = m_settings.value("vehicle/firmwareVersion", "1.0.0").toString();
}

void ConfigManager::saveUserSettings()
{
    // Save to QSettings
    m_settings.setValue("media/lastPlayedSong", m_userSettings.lastPlayedSong);
    m_settings.setValue("media/lastPlayedDevice", m_userSettings.lastPlayedDevice);
    m_settings.setValue("media/volumeLevel", m_userSettings.volumeLevel);
    m_settings.setValue("media/shuffleEnabled", m_userSettings.shuffleEnabled);
    m_settings.setValue("media/repeatEnabled", m_userSettings.repeatEnabled);
    m_settings.setValue("media/equalizerPreset", m_userSettings.equalizerPreset);
    
    m_settings.setValue("climate/preferredTemperature", m_userSettings.preferredTemperature);
    m_settings.setValue("climate/preferredHumidity", m_userSettings.preferredHumidity);
    m_settings.setValue("climate/autoClimateEnabled", m_userSettings.autoClimateEnabled);
    m_settings.setValue("climate/climateMode", m_userSettings.climateMode);
    
    m_settings.setValue("bluetooth/lastConnectedDevice", m_userSettings.lastConnectedDevice);
    m_settings.setValue("bluetooth/autoConnectEnabled", m_userSettings.autoConnectEnabled);
    m_settings.setValue("bluetooth/discoverableEnabled", m_userSettings.discoverableEnabled);
    m_settings.setValue("bluetooth/discoveryTimeout", m_userSettings.discoveryTimeout);
    
    m_settings.setValue("display/brightnessLevel", m_userSettings.brightnessLevel);
    m_settings.setValue("display/theme", m_userSettings.theme);
    m_settings.setValue("display/nightModeEnabled", m_userSettings.nightModeEnabled);
    m_settings.setValue("display/screenTimeout", m_userSettings.screenTimeout);
    
    m_settings.setValue("system/language", m_userSettings.language);
    m_settings.setValue("system/timezone", m_userSettings.timezone);
    m_settings.setValue("system/debugModeEnabled", m_userSettings.debugModeEnabled);
    m_settings.setValue("system/logLevel", m_userSettings.logLevel);
    m_settings.setValue("system/autoUpdateEnabled", m_userSettings.autoUpdateEnabled);
    
    m_settings.setValue("navigation/homeAddress", m_userSettings.homeAddress);
    m_settings.setValue("navigation/workAddress", m_userSettings.workAddress);
    m_settings.setValue("navigation/trafficEnabled", m_userSettings.trafficEnabled);
    m_settings.setValue("navigation/mapProvider", m_userSettings.mapProvider);
    
    m_settings.setValue("vehicle/vehicleModel", m_userSettings.vehicleModel);
    m_settings.setValue("vehicle/vinNumber", m_userSettings.vinNumber);
    m_settings.setValue("vehicle/diagnosticModeEnabled", m_userSettings.diagnosticModeEnabled);
    m_settings.setValue("vehicle/firmwareVersion", m_userSettings.firmwareVersion);
    
    m_settings.sync();
}

void ConfigManager::validateUserSettings()
{
    QStringList errors = getConfigurationErrors();
    if (!errors.isEmpty()) {
        for (const QString& error : errors) {
            LOG_WARNING("ConfigManager", QString("Configuration validation error: %1").arg(error));
            emit configurationError(error);
        }
    }
}

QString ConfigManager::getConfigFilePath() const
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config";
    QDir().mkpath(configDir);
    return configDir + "/autodash_config.json";
}

QString ConfigManager::getBackupDirectory() const
{
    QString backupDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/config/backups";
    createBackupDirectory();
    return backupDir;
}

void ConfigManager::createBackupDirectory()
{
    QDir().mkpath(m_backupDirectory);
}

QJsonObject ConfigManager::settingsToJson(const UserSettings& settings) const
{
    QJsonObject json;
    
    json["lastPlayedSong"] = settings.lastPlayedSong;
    json["lastPlayedDevice"] = settings.lastPlayedDevice;
    json["volumeLevel"] = settings.volumeLevel;
    json["shuffleEnabled"] = settings.shuffleEnabled;
    json["repeatEnabled"] = settings.repeatEnabled;
    json["equalizerPreset"] = settings.equalizerPreset;
    
    json["preferredTemperature"] = settings.preferredTemperature;
    json["preferredHumidity"] = settings.preferredHumidity;
    json["autoClimateEnabled"] = settings.autoClimateEnabled;
    json["climateMode"] = settings.climateMode;
    
    json["lastConnectedDevice"] = settings.lastConnectedDevice;
    json["autoConnectEnabled"] = settings.autoConnectEnabled;
    json["discoverableEnabled"] = settings.discoverableEnabled;
    json["discoveryTimeout"] = settings.discoveryTimeout;
    
    json["brightnessLevel"] = settings.brightnessLevel;
    json["theme"] = settings.theme;
    json["nightModeEnabled"] = settings.nightModeEnabled;
    json["screenTimeout"] = settings.screenTimeout;
    
    json["language"] = settings.language;
    json["timezone"] = settings.timezone;
    json["debugModeEnabled"] = settings.debugModeEnabled;
    json["logLevel"] = settings.logLevel;
    json["autoUpdateEnabled"] = settings.autoUpdateEnabled;
    
    json["homeAddress"] = settings.homeAddress;
    json["workAddress"] = settings.workAddress;
    json["trafficEnabled"] = settings.trafficEnabled;
    json["mapProvider"] = settings.mapProvider;
    
    json["vehicleModel"] = settings.vehicleModel;
    json["vinNumber"] = settings.vinNumber;
    json["diagnosticModeEnabled"] = settings.diagnosticModeEnabled;
    json["firmwareVersion"] = settings.firmwareVersion;
    
    return json;
}

UserSettings ConfigManager::jsonToSettings(const QJsonObject& json) const
{
    UserSettings settings;
    
    settings.lastPlayedSong = json["lastPlayedSong"].toString();
    settings.lastPlayedDevice = json["lastPlayedDevice"].toString();
    settings.volumeLevel = json["volumeLevel"].toInt(50);
    settings.shuffleEnabled = json["shuffleEnabled"].toBool(false);
    settings.repeatEnabled = json["repeatEnabled"].toBool(false);
    settings.equalizerPreset = json["equalizerPreset"].toString("normal");
    
    settings.preferredTemperature = json["preferredTemperature"].toDouble(22.0);
    settings.preferredHumidity = json["preferredHumidity"].toDouble(50.0);
    settings.autoClimateEnabled = json["autoClimateEnabled"].toBool(true);
    settings.climateMode = json["climateMode"].toString("auto");
    
    settings.lastConnectedDevice = json["lastConnectedDevice"].toString();
    settings.autoConnectEnabled = json["autoConnectEnabled"].toBool(true);
    settings.discoverableEnabled = json["discoverableEnabled"].toBool(false);
    settings.discoveryTimeout = json["discoveryTimeout"].toInt(30);
    
    settings.brightnessLevel = json["brightnessLevel"].toInt(80);
    settings.theme = json["theme"].toString("auto");
    settings.nightModeEnabled = json["nightModeEnabled"].toBool(false);
    settings.screenTimeout = json["screenTimeout"].toInt(300);
    
    settings.language = json["language"].toString("en_US");
    settings.timezone = json["timezone"].toString("UTC");
    settings.debugModeEnabled = json["debugModeEnabled"].toBool(false);
    settings.logLevel = json["logLevel"].toString("INFO");
    settings.autoUpdateEnabled = json["autoUpdateEnabled"].toBool(true);
    
    settings.homeAddress = json["homeAddress"].toString();
    settings.workAddress = json["workAddress"].toString();
    settings.trafficEnabled = json["trafficEnabled"].toBool(true);
    settings.mapProvider = json["mapProvider"].toString("google");
    
    settings.vehicleModel = json["vehicleModel"].toString("Rivian R1T");
    settings.vinNumber = json["vinNumber"].toString();
    settings.diagnosticModeEnabled = json["diagnosticModeEnabled"].toBool(false);
    settings.firmwareVersion = json["firmwareVersion"].toString("1.0.0");
    
    return settings;
} 