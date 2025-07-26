#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>
#include <QDebug>
#include <memory>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL
};

class Logger : public QObject
{
    Q_OBJECT

public:
    static Logger& getInstance();
    
    void log(LogLevel level, const QString& module, const QString& message);
    void debug(const QString& module, const QString& message);
    void info(const QString& module, const QString& message);
    void warning(const QString& module, const QString& message);
    void error(const QString& module, const QString& message);
    void critical(const QString& module, const QString& message);
    
    void setLogFile(const QString& filePath);
    void setConsoleOutput(bool enabled);
    void setLogLevel(LogLevel level);
    
    QString getLogBuffer() const;
    void clearLogBuffer();

signals:
    void logMessageAdded(const QString& timestamp, const QString& level, 
                        const QString& module, const QString& message);

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    QString levelToString(LogLevel level) const;
    QString getCurrentTimestamp() const;
    void writeToFile(const QString& logEntry);
    void writeToConsole(const QString& logEntry);
    
    std::unique_ptr<QFile> m_logFile;
    QTextStream m_logStream;
    QMutex m_mutex;
    bool m_consoleOutput;
    LogLevel m_currentLevel;
    QString m_logBuffer;
    static const int MAX_BUFFER_SIZE = 1000;
};

// Convenience macros for easier logging
#define LOG_DEBUG(module, message) Logger::getInstance().debug(module, message)
#define LOG_INFO(module, message) Logger::getInstance().info(module, message)
#define LOG_WARNING(module, message) Logger::getInstance().warning(module, message)
#define LOG_ERROR(module, message) Logger::getInstance().error(module, message)
#define LOG_CRITICAL(module, message) Logger::getInstance().critical(module, message)

#endif // LOGGER_H 