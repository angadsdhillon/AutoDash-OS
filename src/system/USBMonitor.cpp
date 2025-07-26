#include "USBMonitor.h"
#include "Logger.h"
#include <QStandardPaths>
#include <QStorageInfo>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>
#include <QJsonArray>
#include <QProcess>
#include <QRegularExpression>
#include <QUrl>
#include <QUrlQuery>

const QString USBMonitor::CONFIG_FILE = "config/usb_devices.json";
const QStringList USBMonitor::DEFAULT_SUPPORTED_FORMATS = {
    "mp3", "wav", "flac", "aac", "ogg", "wma", "m4a",
    "mp4", "avi", "mkv", "mov", "wmv", "flv", "webm"
};

USBMonitor::USBMonitor()
    : m_fileSystemWatcher(std::make_unique<QFileSystemWatcher>(this))
    , m_scanTimer(std::make_unique<QTimer>(this))
    , m_isMonitoring(false)
    , m_autoScan(true)
    , m_simulateMountError(false)
    , m_simulateFileSystemError(false)
    , m_simulateCorruptedFiles(false)
    , m_supportedFormats(DEFAULT_SUPPORTED_FORMATS)
{
    // Set up default watch directories
    m_watchDirectories << "mnt/usb" << "media/usb" << "tmp/usb";
    
    // Connect signals
    connect(m_fileSystemWatcher.get(), &QFileSystemWatcher::directoryChanged,
            this, &USBMonitor::scanDirectory);
    connect(m_scanTimer.get(), &QTimer::timeout, this, [this]() {
        for (const auto& device : m_connectedDevices) {
            if (device.isConnected) {
                scanMediaFiles(device.deviceId);
            }
        }
    });
    
    // Load saved device list
    loadDeviceList();
    
    LOG_INFO("USBMonitor", "USB Monitor system initialized");
}

USBMonitor::~USBMonitor()
{
    stopMonitoring();
    saveDeviceList();
    LOG_INFO("USBMonitor", "USB Monitor system shutdown");
}

USBMonitor& USBMonitor::getInstance()
{
    static USBMonitor instance;
    return instance;
}

void USBMonitor::startMonitoring()
{
    if (m_isMonitoring) {
        return;
    }
    
    initializeFileSystemWatcher();
    m_scanTimer->start(10000); // Scan every 10 seconds
    m_isMonitoring = true;
    
    LOG_INFO("USBMonitor", "USB monitoring started");
}

void USBMonitor::stopMonitoring()
{
    if (!m_isMonitoring) {
        return;
    }
    
    m_fileSystemWatcher->removePaths(m_fileSystemWatcher->directories());
    m_scanTimer->stop();
    m_isMonitoring = false;
    
    LOG_INFO("USBMonitor", "USB monitoring stopped");
}

bool USBMonitor::isMonitoring() const
{
    return m_isMonitoring;
}

void USBMonitor::simulateUSBInsertion(const QString& deviceName)
{
    QString deviceId = generateDeviceId();
    QString mountPoint = QString("mnt/usb/%1").arg(deviceId);
    
    USBDevice device;
    device.deviceId = deviceId;
    device.deviceName = deviceName;
    device.mountPoint = mountPoint;
    device.totalSpace = 32000000000; // 32GB
    device.freeSpace = 28000000000;  // 28GB
    device.fileSystem = "FAT32";
    device.isConnected = true;
    device.connectedTime = QDateTime::currentDateTime();
    
    // Create mount directory
    QDir().mkpath(mountPoint);
    
    m_connectedDevices.append(device);
    
    LOG_INFO("USBMonitor", QString("USB device inserted: %1 at %2").arg(deviceName).arg(mountPoint));
    emit deviceConnected(device);
    
    // Start monitoring the new directory
    if (m_isMonitoring) {
        m_fileSystemWatcher->addPath(mountPoint);
    }
}

void USBMonitor::simulateUSBRemoval(const QString& deviceId)
{
    QString targetDeviceId = deviceId;
    if (targetDeviceId.isEmpty() && !m_connectedDevices.isEmpty()) {
        targetDeviceId = m_connectedDevices.first().deviceId;
    }
    
    for (auto it = m_connectedDevices.begin(); it != m_connectedDevices.end(); ++it) {
        if (it->deviceId == targetDeviceId) {
            LOG_INFO("USBMonitor", QString("USB device removed: %1").arg(it->deviceName));
            emit deviceDisconnected(targetDeviceId);
            
            // Remove from file system watcher
            if (m_isMonitoring) {
                m_fileSystemWatcher->removePath(it->mountPoint);
            }
            
            m_connectedDevices.erase(it);
            break;
        }
    }
}

void USBMonitor::mountDevice(const QString& deviceId, const QString& mountPoint)
{
    if (m_simulateMountError) {
        LOG_ERROR("USBMonitor", QString("Failed to mount device %1").arg(deviceId));
        emit mountError(deviceId, "Simulated mount error - device not responding");
        return;
    }
    
    for (auto& device : m_connectedDevices) {
        if (device.deviceId == deviceId) {
            device.mountPoint = mountPoint;
            device.isConnected = true;
            
            // Create mount directory
            QDir().mkpath(mountPoint);
            
            LOG_INFO("USBMonitor", QString("Device %1 mounted at %2").arg(deviceId).arg(mountPoint));
            
            // Start monitoring the new mount point
            if (m_isMonitoring) {
                m_fileSystemWatcher->addPath(mountPoint);
            }
            
            return;
        }
    }
    
    LOG_ERROR("USBMonitor", QString("Device %1 not found for mounting").arg(deviceId));
}

void USBMonitor::unmountDevice(const QString& deviceId)
{
    for (auto& device : m_connectedDevices) {
        if (device.deviceId == deviceId) {
            device.isConnected = false;
            
            // Remove from file system watcher
            if (m_isMonitoring) {
                m_fileSystemWatcher->removePath(device.mountPoint);
            }
            
            LOG_INFO("USBMonitor", QString("Device %1 unmounted").arg(deviceId));
            return;
        }
    }
}

void USBMonitor::addMediaFile(const QString& deviceId, const MediaFile& file)
{
    for (auto& device : m_connectedDevices) {
        if (device.deviceId == deviceId) {
            device.mediaFiles.append(file);
            LOG_INFO("USBMonitor", QString("Added media file: %1 to device %2").arg(file.fileName).arg(deviceId));
            emit mediaFileAdded(deviceId, file);
            emit mediaFilesChanged(deviceId, device.mediaFiles);
            return;
        }
    }
}

void USBMonitor::removeMediaFile(const QString& deviceId, const QString& fileName)
{
    for (auto& device : m_connectedDevices) {
        if (device.deviceId == deviceId) {
            for (auto it = device.mediaFiles.begin(); it != device.mediaFiles.end(); ++it) {
                if (it->fileName == fileName) {
                    device.mediaFiles.erase(it);
                    LOG_INFO("USBMonitor", QString("Removed media file: %1 from device %2").arg(fileName).arg(deviceId));
                    emit mediaFileRemoved(deviceId, fileName);
                    emit mediaFilesChanged(deviceId, device.mediaFiles);
                    return;
                }
            }
        }
    }
}

void USBMonitor::scanMediaFiles(const QString& deviceId)
{
    for (auto& device : m_connectedDevices) {
        if (device.deviceId == deviceId && device.isConnected) {
            QDir dir(device.mountPoint);
            if (!dir.exists()) {
                continue;
            }
            
            QStringList filters;
            for (const QString& format : m_supportedFormats) {
                filters << QString("*.%1").arg(format);
            }
            
            QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
            QList<MediaFile> mediaFiles;
            
            for (const QFileInfo& fileInfo : files) {
                if (isMediaFile(fileInfo.fileName())) {
                    MediaFile mediaFile;
                    mediaFile.fileName = fileInfo.fileName();
                    mediaFile.filePath = fileInfo.absoluteFilePath();
                    mediaFile.fileSize = fileInfo.size();
                    mediaFile.fileType = fileInfo.suffix().toLower();
                    mediaFile.lastModified = fileInfo.lastModified();
                    
                    // Try to extract metadata
                    QString metadata = getFileMetadataInternal(fileInfo.absoluteFilePath());
                    if (!metadata.isEmpty()) {
                        QJsonDocument doc = QJsonDocument::fromJson(metadata.toUtf8());
                        QJsonObject obj = doc.object();
                        mediaFile.title = obj["title"].toString();
                        mediaFile.artist = obj["artist"].toString();
                        mediaFile.album = obj["album"].toString();
                        mediaFile.duration = obj["duration"].toString();
                    } else {
                        // Fallback to filename
                        mediaFile.title = fileInfo.baseName();
                        mediaFile.artist = "Unknown Artist";
                        mediaFile.album = "Unknown Album";
                        mediaFile.duration = "00:00";
                    }
                    
                    mediaFiles.append(mediaFile);
                }
            }
            
            device.mediaFiles = mediaFiles;
            updateDeviceSpace(deviceId);
            
            LOG_INFO("USBMonitor", QString("Scanned %1 media files from device %2").arg(mediaFiles.size()).arg(deviceId));
            emit mediaFilesChanged(deviceId, mediaFiles);
        }
    }
}

QList<USBDevice> USBMonitor::getConnectedDevices() const
{
    QList<USBDevice> connected;
    for (const auto& device : m_connectedDevices) {
        if (device.isConnected) {
            connected.append(device);
        }
    }
    return connected;
}

USBDevice USBMonitor::getDevice(const QString& deviceId) const
{
    for (const auto& device : m_connectedDevices) {
        if (device.deviceId == deviceId) {
            return device;
        }
    }
    return USBDevice{};
}

QList<MediaFile> USBMonitor::getMediaFiles(const QString& deviceId) const
{
    for (const auto& device : m_connectedDevices) {
        if (device.deviceId == deviceId) {
            return device.mediaFiles;
        }
    }
    return QList<MediaFile>{};
}

bool USBMonitor::isDeviceConnected(const QString& deviceId) const
{
    for (const auto& device : m_connectedDevices) {
        if (device.deviceId == deviceId && device.isConnected) {
            return true;
        }
    }
    return false;
}

QStringList USBMonitor::getSupportedFormats() const
{
    return m_supportedFormats;
}

bool USBMonitor::isMediaFile(const QString& fileName) const
{
    QString extension = QFileInfo(fileName).suffix().toLower();
    return m_supportedFormats.contains(extension);
}

QString USBMonitor::getFileMetadata(const QString& filePath) const
{
    return getFileMetadataInternal(filePath);
}

void USBMonitor::setWatchDirectories(const QStringList& directories)
{
    m_watchDirectories = directories;
    if (m_isMonitoring) {
        initializeFileSystemWatcher();
    }
}

void USBMonitor::setSupportedFormats(const QStringList& formats)
{
    m_supportedFormats = formats;
    LOG_INFO("USBMonitor", QString("Supported formats updated: %1").arg(formats.join(", ")));
}

void USBMonitor::enableAutoScan(bool enable)
{
    m_autoScan = enable;
    if (enable && m_isMonitoring) {
        m_scanTimer->start(10000);
    } else {
        m_scanTimer->stop();
    }
    LOG_INFO("USBMonitor", QString("Auto scan %1").arg(enable ? "enabled" : "disabled"));
}

void USBMonitor::simulateMountError(bool enable)
{
    m_simulateMountError = enable;
    LOG_INFO("USBMonitor", QString("Mount error simulation %1").arg(enable ? "enabled" : "disabled"));
}

void USBMonitor::simulateFileSystemError(bool enable)
{
    m_simulateFileSystemError = enable;
    LOG_INFO("USBMonitor", QString("File system error simulation %1").arg(enable ? "enabled" : "disabled"));
}

void USBMonitor::simulateCorruptedFiles(bool enable)
{
    m_simulateCorruptedFiles = enable;
    LOG_INFO("USBMonitor", QString("Corrupted files simulation %1").arg(enable ? "enabled" : "disabled"));
}

void USBMonitor::initializeFileSystemWatcher()
{
    m_fileSystemWatcher->removePaths(m_fileSystemWatcher->directories());
    
    for (const QString& directory : m_watchDirectories) {
        QDir dir(directory);
        if (dir.exists()) {
            m_fileSystemWatcher->addPath(directory);
            LOG_DEBUG("USBMonitor", QString("Monitoring directory: %1").arg(directory));
        }
    }
    
    // Add mount points of connected devices
    for (const auto& device : m_connectedDevices) {
        if (device.isConnected) {
            m_fileSystemWatcher->addPath(device.mountPoint);
        }
    }
}

void USBMonitor::scanDirectory(const QString& path)
{
    LOG_DEBUG("USBMonitor", QString("Directory changed: %1").arg(path));
    
    // Find which device this path belongs to
    QString deviceId;
    for (const auto& device : m_connectedDevices) {
        if (device.mountPoint == path) {
            deviceId = device.deviceId;
            break;
        }
    }
    
    if (!deviceId.isEmpty()) {
        scanMediaFiles(deviceId);
    }
}

void USBMonitor::processMediaFile(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    if (isMediaFile(fileInfo.fileName())) {
        LOG_DEBUG("USBMonitor", QString("Processing media file: %1").arg(filePath));
        
        // Find which device this file belongs to
        for (auto& device : m_connectedDevices) {
            if (filePath.startsWith(device.mountPoint)) {
                MediaFile mediaFile;
                mediaFile.fileName = fileInfo.fileName();
                mediaFile.filePath = filePath;
                mediaFile.fileSize = fileInfo.size();
                mediaFile.fileType = fileInfo.suffix().toLower();
                mediaFile.lastModified = fileInfo.lastModified();
                
                // Extract metadata
                QString metadata = getFileMetadataInternal(filePath);
                if (!metadata.isEmpty()) {
                    QJsonDocument doc = QJsonDocument::fromJson(metadata.toUtf8());
                    QJsonObject obj = doc.object();
                    mediaFile.title = obj["title"].toString();
                    mediaFile.artist = obj["artist"].toString();
                    mediaFile.album = obj["album"].toString();
                    mediaFile.duration = obj["duration"].toString();
                } else {
                    mediaFile.title = fileInfo.baseName();
                    mediaFile.artist = "Unknown Artist";
                    mediaFile.album = "Unknown Album";
                    mediaFile.duration = "00:00";
                }
                
                addMediaFile(device.deviceId, mediaFile);
                break;
            }
        }
    }
}

void USBMonitor::updateDeviceSpace(const QString& deviceId)
{
    for (auto& device : m_connectedDevices) {
        if (device.deviceId == deviceId) {
            QStorageInfo storage(device.mountPoint);
            if (storage.isValid()) {
                device.totalSpace = storage.bytesTotal();
                device.freeSpace = storage.bytesAvailable();
                device.fileSystem = storage.fileSystemType();
            }
            break;
        }
    }
}

void USBMonitor::saveDeviceList()
{
    QJsonArray deviceArray;
    for (const auto& device : m_connectedDevices) {
        QJsonObject deviceObj;
        deviceObj["deviceId"] = device.deviceId;
        deviceObj["deviceName"] = device.deviceName;
        deviceObj["mountPoint"] = device.mountPoint;
        deviceObj["totalSpace"] = device.totalSpace;
        deviceObj["freeSpace"] = device.freeSpace;
        deviceObj["fileSystem"] = device.fileSystem;
        deviceObj["isConnected"] = device.isConnected;
        deviceObj["connectedTime"] = device.connectedTime.toString(Qt::ISODate);
        
        QJsonArray mediaArray;
        for (const auto& media : device.mediaFiles) {
            QJsonObject mediaObj;
            mediaObj["fileName"] = media.fileName;
            mediaObj["filePath"] = media.filePath;
            mediaObj["title"] = media.title;
            mediaObj["artist"] = media.artist;
            mediaObj["album"] = media.album;
            mediaObj["duration"] = media.duration;
            mediaObj["fileSize"] = media.fileSize;
            mediaObj["fileType"] = media.fileType;
            mediaObj["lastModified"] = media.lastModified.toString(Qt::ISODate);
            mediaArray.append(mediaObj);
        }
        deviceObj["mediaFiles"] = mediaArray;
        
        deviceArray.append(deviceObj);
    }
    
    QFile file(CONFIG_FILE);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(deviceArray).toJson());
        LOG_DEBUG("USBMonitor", "Device list saved");
    }
}

void USBMonitor::loadDeviceList()
{
    QFile file(CONFIG_FILE);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        QJsonArray deviceArray = doc.array();
        
        for (const QJsonValue& value : deviceArray) {
            QJsonObject deviceObj = value.toObject();
            USBDevice device;
            device.deviceId = deviceObj["deviceId"].toString();
            device.deviceName = deviceObj["deviceName"].toString();
            device.mountPoint = deviceObj["mountPoint"].toString();
            device.totalSpace = deviceObj["totalSpace"].toVariant().toLongLong();
            device.freeSpace = deviceObj["freeSpace"].toVariant().toLongLong();
            device.fileSystem = deviceObj["fileSystem"].toString();
            device.isConnected = deviceObj["isConnected"].toBool();
            device.connectedTime = QDateTime::fromString(deviceObj["connectedTime"].toString(), Qt::ISODate);
            
            QJsonArray mediaArray = deviceObj["mediaFiles"].toArray();
            for (const QJsonValue& mediaValue : mediaArray) {
                QJsonObject mediaObj = mediaValue.toObject();
                MediaFile media;
                media.fileName = mediaObj["fileName"].toString();
                media.filePath = mediaObj["filePath"].toString();
                media.title = mediaObj["title"].toString();
                media.artist = mediaObj["artist"].toString();
                media.album = mediaObj["album"].toString();
                media.duration = mediaObj["duration"].toString();
                media.fileSize = mediaObj["fileSize"].toVariant().toLongLong();
                media.fileType = mediaObj["fileType"].toString();
                media.lastModified = QDateTime::fromString(mediaObj["lastModified"].toString(), Qt::ISODate);
                device.mediaFiles.append(media);
            }
            
            m_connectedDevices.append(device);
        }
        
        LOG_DEBUG("USBMonitor", QString("Loaded %1 devices from config").arg(m_connectedDevices.size()));
    }
}

QString USBMonitor::generateDeviceId() const
{
    return QString("USB_%1").arg(QDateTime::currentMSecsSinceEpoch());
}

QString USBMonitor::getFileDuration(const QString& filePath) const
{
    // Simulate duration extraction
    QFileInfo fileInfo(filePath);
    qint64 size = fileInfo.size();
    
    // Rough estimation based on file size and type
    QString extension = fileInfo.suffix().toLower();
    if (extension == "mp3") {
        // Assume ~128kbps
        int durationSeconds = (size * 8) / (128 * 1024);
        return QString("%1:%2").arg(durationSeconds / 60, 2, 10, QChar('0'))
                               .arg(durationSeconds % 60, 2, 10, QChar('0'));
    } else if (extension == "wav") {
        // Assume 44.1kHz, 16-bit, stereo
        int durationSeconds = size / (44100 * 2 * 2);
        return QString("%1:%2").arg(durationSeconds / 60, 2, 10, QChar('0'))
                               .arg(durationSeconds % 60, 2, 10, QChar('0'));
    }
    
    return "00:00";
}

QString USBMonitor::getFileMetadataInternal(const QString& filePath) const
{
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.baseName();
    
    // Simulate metadata extraction
    QJsonObject metadata;
    metadata["title"] = fileName;
    metadata["artist"] = "Unknown Artist";
    metadata["album"] = "Unknown Album";
    metadata["duration"] = getFileDuration(filePath);
    
    // Simulate some realistic metadata for common patterns
    if (fileName.contains(" - ")) {
        QStringList parts = fileName.split(" - ");
        if (parts.size() >= 2) {
            metadata["artist"] = parts[0].trimmed();
            metadata["title"] = parts[1].trimmed();
        }
    }
    
    return QJsonDocument(metadata).toJson();
} 