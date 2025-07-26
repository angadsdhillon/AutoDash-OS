#include <catch2/catch_test_macros.hpp>
#include <QApplication>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QDir>

#include "../src/system/ConfigManager.h"

TEST_CASE("Configuration Manager Tests", "[config]") {
    // Create QApplication instance for Qt tests
    int argc = 1;
    char* argv[] = {(char*)"test"};
    QApplication app(argc, argv);
    
    SECTION("Configuration Loading") {
        ConfigManager& config = ConfigManager::getInstance();
        
        // Test default configuration loading
        REQUIRE(config.loadConfiguration() == true);
        
        // Test setting and getting values
        config.setSetting("test_key", "test_value");
        REQUIRE(config.getSetting("test_key").toString() == "test_value");
        
        // Test default values
        REQUIRE(config.getSetting("nonexistent_key", "default").toString() == "default");
    }
    
    SECTION("User Settings") {
        ConfigManager& config = ConfigManager::getInstance();
        
        UserSettings settings = config.getUserSettings();
        
        // Test default settings
        REQUIRE(settings.volumeLevel >= 0);
        REQUIRE(settings.volumeLevel <= 100);
        REQUIRE(settings.preferredTemperature >= -40.0);
        REQUIRE(settings.preferredTemperature <= 50.0);
        
        // Test updating settings
        config.updateUserSetting("volumeLevel", 75);
        settings = config.getUserSettings();
        REQUIRE(settings.volumeLevel == 75);
    }
    
    SECTION("Configuration Validation") {
        ConfigManager& config = ConfigManager::getInstance();
        
        // Test valid configuration
        REQUIRE(config.validateConfiguration() == true);
        
        // Test configuration errors
        QStringList errors = config.getConfigurationErrors();
        // Should be empty for valid configuration
        REQUIRE(errors.isEmpty());
    }
    
    SECTION("Environment Settings") {
        ConfigManager& config = ConfigManager::getInstance();
        
        // Test environment switching
        config.setEnvironment("development");
        REQUIRE(config.getCurrentEnvironment() == "development");
        REQUIRE(config.isDevelopmentMode() == true);
        REQUIRE(config.isProductionMode() == false);
        
        config.setEnvironment("production");
        REQUIRE(config.getCurrentEnvironment() == "production");
        REQUIRE(config.isDevelopmentMode() == false);
        REQUIRE(config.isProductionMode() == true);
    }
    
    SECTION("Configuration Categories") {
        ConfigManager& config = ConfigManager::getInstance();
        
        QStringList categories = config.getConfigurationCategories();
        REQUIRE(categories.contains("media"));
        REQUIRE(categories.contains("climate"));
        REQUIRE(categories.contains("bluetooth"));
        REQUIRE(categories.contains("display"));
        REQUIRE(categories.contains("system"));
        REQUIRE(categories.contains("navigation"));
        REQUIRE(categories.contains("vehicle"));
    }
    
    SECTION("Category Settings") {
        ConfigManager& config = ConfigManager::getInstance();
        
        // Test getting category settings
        QJsonObject mediaSettings = config.getCategorySettings("media");
        REQUIRE(mediaSettings.contains("volumeLevel"));
        REQUIRE(mediaSettings.contains("lastPlayedSong"));
        
        // Test setting category settings
        QJsonObject newSettings;
        newSettings["volumeLevel"] = 80;
        newSettings["lastPlayedSong"] = "test_song.mp3";
        
        config.setCategorySettings("media", newSettings);
        
        UserSettings settings = config.getUserSettings();
        REQUIRE(settings.volumeLevel == 80);
        REQUIRE(settings.lastPlayedSong == "test_song.mp3");
    }
    
    SECTION("Backup and Restore") {
        ConfigManager& config = ConfigManager::getInstance();
        
        // Test backup creation
        QString backupPath = "test_backup.json";
        REQUIRE(config.backupConfiguration(backupPath) == true);
        
        // Test backup file existence
        QFile backupFile(backupPath);
        REQUIRE(backupFile.exists() == true);
        
        // Test restore functionality
        REQUIRE(config.restoreConfiguration(backupPath) == true);
        
        // Cleanup
        backupFile.remove();
    }
    
    SECTION("System Information") {
        ConfigManager& config = ConfigManager::getInstance();
        
        // Test system info retrieval
        QString systemInfo = config.getSystemInfo();
        REQUIRE(systemInfo.contains("OS:"));
        REQUIRE(systemInfo.contains("Architecture:"));
        
        QString versionInfo = config.getVersionInfo();
        REQUIRE(versionInfo.contains("AutoDash OS"));
        
        QString buildInfo = config.getBuildInfo();
        REQUIRE(buildInfo.contains("Build Date:"));
        REQUIRE(buildInfo.contains("Qt Version:"));
    }
} 