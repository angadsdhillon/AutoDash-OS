#ifndef USBMONITOR_H
#define USBMONITOR_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QDir>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QTimer>
#include <memory>

struct MediaFile {
    QString fileName;
    QString filePath;
    QString title;
    QString artist;
    QString album;
    QString duration;
    qint64 fileSize;
    QString fileType;
    QDateTime lastModified;
};

struct USBDevice {
    QString deviceId;
    QString deviceName;
    QString mountPoint;
    qint64 totalSpace;
    qint64 freeSpace;
    QString fileSystem;
    bool isConnected;
    QDateTime connectedTime;
    QList<MediaFile> mediaFiles;
};

class USBMonitor : public QObject
{
    Q_OBJECT

public:
    static USBMonitor& getInstance();
    
    // USB device management
    void startMonitoring();
    void stopMonitoring();
    bool isMonitoring() const;
    
    // Device operations
    void simulateUSBInsertion(const QString& deviceName = "USB_DRIVE_01");
    void simulateUSBRemoval(const QString& deviceId = "");
    void mountDevice(const QString& deviceId, const QString& mountPoint);
    void unmountDevice(const QString& deviceId);
    
    // Media file operations
    void addMediaFile(const QString& deviceId, const MediaFile& file);
    void removeMediaFile(const QString& deviceId, const QString& fileName);
    void scanMediaFiles(const QString& deviceId);
    
    // Device information
    QList<USBDevice> getConnectedDevices() const;
    USBDevice getDevice(const QString& deviceId) const;
    QList<MediaFile> getMediaFiles(const QString& deviceId) const;
    bool isDeviceConnected(const QString& deviceId) const;
    
    // File system operations
    QStringList getSupportedFormats() const;
    bool isMediaFile(const QString& fileName) const;
    QString getFileMetadata(const QString& filePath) const;
    
    // Configuration
    void setWatchDirectories(const QStringList& directories);
    void setSupportedFormats(const QStringList& formats);
    void enableAutoScan(bool enable);
    
    // Error simulation
    void simulateMountError(bool enable);
    void simulateFileSystemError(bool enable);
    void simulateCorruptedFiles(bool enable);

signals:
    void deviceConnected(const USBDevice& device);
    void deviceDisconnected(const QString& deviceId);
    void mediaFilesChanged(const QString& deviceId, const QList<MediaFile>& files);
    void mountError(const QString& deviceId, const QString& error);
    void fileSystemError(const QString& deviceId, const QString& error);
    void mediaFileAdded(const QString& deviceId, const MediaFile& file);
    void mediaFileRemoved(const QString& deviceId, const QString& fileName);

private:
    USBMonitor();
    ~USBMonitor();
    USBMonitor(const USBMonitor&) = delete;
    USBMonitor& operator=(const USBMonitor&) = delete;
    
    void initializeFileSystemWatcher();
    void scanDirectory(const QString& path);
    void processMediaFile(const QString& filePath);
    void updateDeviceSpace(const QString& deviceId);
    void saveDeviceList();
    void loadDeviceList();
    QString generateDeviceId() const;
    QString getFileDuration(const QString& filePath) const;
    QString getFileMetadataInternal(const QString& filePath) const;
    
    std::unique_ptr<QFileSystemWatcher> m_fileSystemWatcher;
    std::unique_ptr<QTimer> m_scanTimer;
    
    QList<USBDevice> m_connectedDevices;
    QStringList m_watchDirectories;
    QStringList m_supportedFormats;
    
    bool m_isMonitoring;
    bool m_autoScan;
    bool m_simulateMountError;
    bool m_simulateFileSystemError;
    bool m_simulateCorruptedFiles;
    
    static const QString CONFIG_FILE;
    static const QStringList DEFAULT_SUPPORTED_FORMATS;
};

#endif // USBMONITOR_H 