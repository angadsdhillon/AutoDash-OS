#include "Logger.h"
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>

Logger::Logger() 
    : m_consoleOutput(true)
    , m_currentLevel(LogLevel::DEBUG)
{
    // Create log directory if it doesn't exist
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs";
    QDir().mkpath(logDir);
    
    // Set default log file
    QString logFile = logDir + "/autodash.log";
    setLogFile(logFile);
    
    LOG_INFO("Logger", "AutoDash OS Logger initialized");
}

Logger::~Logger()
{
    if (m_logFile && m_logFile->isOpen()) {
        LOG_INFO("Logger", "Shutting down logger");
        m_logFile->close();
    }
}

Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

void Logger::log(LogLevel level, const QString& module, const QString& message)
{
    if (level < m_currentLevel) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    QString timestamp = getCurrentTimestamp();
    QString levelStr = levelToString(level);
    QString logEntry = QString("[%1] [%2] [%3] %4")
                      .arg(timestamp, levelStr, module, message);
    
    // Add to buffer
    m_logBuffer += logEntry + "\n";
    if (m_logBuffer.split('\n').size() > MAX_BUFFER_SIZE) {
        QStringList lines = m_logBuffer.split('\n');
        lines.removeFirst();
        m_logBuffer = lines.join('\n');
    }
    
    // Write to file
    writeToFile(logEntry);
    
    // Write to console if enabled
    if (m_consoleOutput) {
        writeToConsole(logEntry);
    }
    
    // Emit signal for UI updates
    emit logMessageAdded(timestamp, levelStr, module, message);
}

void Logger::debug(const QString& module, const QString& message)
{
    log(LogLevel::DEBUG, module, message);
}

void Logger::info(const QString& module, const QString& message)
{
    log(LogLevel::INFO, module, message);
}

void Logger::warning(const QString& module, const QString& message)
{
    log(LogLevel::WARNING, module, message);
}

void Logger::error(const QString& module, const QString& message)
{
    log(LogLevel::ERROR, module, message);
}

void Logger::critical(const QString& module, const QString& message)
{
    log(LogLevel::CRITICAL, module, message);
}

void Logger::setLogFile(const QString& filePath)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_logFile && m_logFile->isOpen()) {
        m_logFile->close();
    }
    
    m_logFile = std::make_unique<QFile>(filePath);
    if (!m_logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        qWarning() << "Failed to open log file:" << filePath;
        return;
    }
    
    m_logStream.setDevice(m_logFile.get());
    LOG_INFO("Logger", QString("Log file set to: %1").arg(filePath));
}

void Logger::setConsoleOutput(bool enabled)
{
    m_consoleOutput = enabled;
    LOG_INFO("Logger", QString("Console output %1").arg(enabled ? "enabled" : "disabled"));
}

void Logger::setLogLevel(LogLevel level)
{
    m_currentLevel = level;
    LOG_INFO("Logger", QString("Log level set to: %1").arg(levelToString(level)));
}

QString Logger::getLogBuffer() const
{
    QMutexLocker locker(&m_mutex);
    return m_logBuffer;
}

void Logger::clearLogBuffer()
{
    QMutexLocker locker(&m_mutex);
    m_logBuffer.clear();
}

QString Logger::levelToString(LogLevel level) const
{
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default:               return "UNKNOWN";
    }
}

QString Logger::getCurrentTimestamp() const
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
}

void Logger::writeToFile(const QString& logEntry)
{
    if (m_logFile && m_logFile->isOpen()) {
        m_logStream << logEntry << Qt::endl;
        m_logStream.flush();
    }
}

void Logger::writeToConsole(const QString& logEntry)
{
    qDebug().noquote() << logEntry;
} 