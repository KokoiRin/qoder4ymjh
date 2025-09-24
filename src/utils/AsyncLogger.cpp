#include "utils/AsyncLogger.h"
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>
#include <QCoreApplication>

// ============ LogWriterThread 实现 ============

LogWriterThread::LogWriterThread(QObject *parent)
    : QThread(parent), isRunning(false), textStream(nullptr)
{
    initLogFile();
}

LogWriterThread::~LogWriterThread()
{
    stop();
    if (textStream) {
        delete textStream;
        textStream = nullptr;
    }
    if (logFile.isOpen()) {
        logFile.close();
    }
}

void LogWriterThread::stop()
{
    QMutexLocker locker(&mutex);
    isRunning = false;
    condition.wakeOne();
    locker.unlock();
    
    if (!wait(3000)) {
        terminate();
        wait(1000);
    }
}

void LogWriterThread::addLogEntry(const LogEntry& entry)
{
    QMutexLocker locker(&mutex);
    logQueue.enqueue(entry);
    condition.wakeOne();
}

void LogWriterThread::run()
{
    isRunning = true;
    
    while (isRunning) {
        QMutexLocker locker(&mutex);
        
        if (logQueue.isEmpty()) {
            condition.wait(&mutex, 1000); // 等待1秒或直到有新日志
            continue;
        }
        
        LogEntry entry = logQueue.dequeue();
        locker.unlock();
        
        writeLogEntry(entry);
    }
}

void LogWriterThread::initLogFile()
{
    // 创建日志目录
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(logDir);
    
    // 设置日志文件路径
    QString logFileName = QString("QtDemo_%1.log")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd"));
    QString logFilePath = QDir(logDir).filePath(logFileName);
    
    logFile.setFileName(logFilePath);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        textStream = new QTextStream(&logFile);
        textStream->setEncoding(QStringConverter::Utf8);
    }
}

void LogWriterThread::writeLogEntry(const LogEntry& entry)
{
    if (!textStream) return;
    
    QString formattedEntry = formatLogEntry(entry);
    *textStream << formattedEntry << Qt::endl;
    textStream->flush();
}

QString LogWriterThread::formatLogEntry(const LogEntry& entry) const
{
    QString levelStr = levelToString(entry.level);
    QString timestamp = entry.timestamp.toString("yyyy-MM-dd hh:mm:ss.zzz");
    
    QString formatted = QString("[%1] [%2] [%3] %4")
        .arg(timestamp)
        .arg(levelStr.leftJustified(7))
        .arg(entry.category.leftJustified(12))
        .arg(entry.message);
    
    if (!entry.details.isEmpty()) {
        formatted += QString(" | Details: %1").arg(entry.details);
    }
    
    return formatted;
}

QString LogWriterThread::levelToString(LogLevel level) const
{
    switch (level) {
        case LogLevel::Debug:   return "DEBUG";
        case LogLevel::Info:    return "INFO";
        case LogLevel::Warning: return "WARNING";
        case LogLevel::Error:   return "ERROR";
        case LogLevel::Action:  return "ACTION";
        default:                return "UNKNOWN";
    }
}

// ============ AsyncLogger 实现 ============

AsyncLogger* AsyncLogger::m_instance = nullptr;

AsyncLogger* AsyncLogger::instance()
{
    if (!m_instance) {
        m_instance = new AsyncLogger();
    }
    return m_instance;
}

AsyncLogger::AsyncLogger(QObject* parent)
    : QObject(parent)
    , writerThread(new LogWriterThread(this))
    , minLogLevel(LogLevel::Info)
    , maxLogFileSize(10 * 1024 * 1024) // 10MB
    , autoBackupEnabled(true)
{
    writerThread->start();
    
    // 记录应用程序启动
    logApplicationStart();
}

AsyncLogger::~AsyncLogger()
{
    logApplicationExit();
    
    if (writerThread) {
        writerThread->stop();
        delete writerThread;
    }
}

void AsyncLogger::debug(const QString& category, const QString& message, const QString& details)
{
    log(LogLevel::Debug, category, message, details);
}

void AsyncLogger::info(const QString& category, const QString& message, const QString& details)
{
    log(LogLevel::Info, category, message, details);
}

void AsyncLogger::warning(const QString& category, const QString& message, const QString& details)
{
    log(LogLevel::Warning, category, message, details);
}

void AsyncLogger::error(const QString& category, const QString& message, const QString& details)
{
    log(LogLevel::Error, category, message, details);
}

void AsyncLogger::action(const QString& category, const QString& message, const QString& details)
{
    log(LogLevel::Action, category, message, details);
}

void AsyncLogger::logButtonClick(const QString& buttonName, const QString& additionalInfo)
{
    QVariantMap details;
    details["button"] = buttonName;
    details["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    if (!additionalInfo.isEmpty()) {
        details["info"] = additionalInfo;
    }
    
    QString message = QString("按钮点击: %1").arg(buttonName);
    action("Button", message, createDetailsJson(details));
}

void AsyncLogger::logColorPicked(const QColor& color, const QPoint& position, const QString& windowInfo)
{
    QVariantMap details;
    details["color_rgb"] = QString("RGB(%1, %2, %3)").arg(color.red()).arg(color.green()).arg(color.blue());
    details["color_hex"] = color.name().toUpper();
    details["position_x"] = position.x();
    details["position_y"] = position.y();
    details["position"] = QString("(%1, %2)").arg(position.x()).arg(position.y());
    details["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    if (!windowInfo.isEmpty()) {
        details["window"] = windowInfo;
    }
    
    QString message = QString("颜色拾取: %1 at (%2, %3)")
        .arg(color.name().toUpper())
        .arg(position.x())
        .arg(position.y());
    
    action("ColorPicker", message, createDetailsJson(details));
}

void AsyncLogger::logClickSimulated(const QPoint& position, const QString& coordinateType, const QString& mouseButton, bool success)
{
    QVariantMap details;
    details["position_x"] = position.x();
    details["position_y"] = position.y();
    details["position"] = QString("(%1, %2)").arg(position.x()).arg(position.y());
    details["coordinate_type"] = coordinateType;
    details["mouse_button"] = mouseButton;
    details["success"] = success;
    details["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    QString message = QString("点击模拟: %1 %2 at (%3, %4) - %5")
        .arg(mouseButton)
        .arg(coordinateType)
        .arg(position.x())
        .arg(position.y())
        .arg(success ? "成功" : "失败");
    
    action("ClickSimulator", message, createDetailsJson(details));
}

void AsyncLogger::logWindowBound(const QString& windowTitle, const QString& windowInfo)
{
    QVariantMap details;
    details["window_title"] = windowTitle;
    details["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    if (!windowInfo.isEmpty()) {
        details["window_info"] = windowInfo;
    }
    
    QString message = QString("窗口绑定: %1").arg(windowTitle);
    action("WindowManager", message, createDetailsJson(details));
}

void AsyncLogger::logApplicationStart()
{
    QVariantMap details;
    details["version"] = QCoreApplication::applicationVersion();
    details["qt_version"] = qVersion();
    details["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    action("Application", "应用程序启动", createDetailsJson(details));
}

void AsyncLogger::logApplicationExit()
{
    QVariantMap details;
    details["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    action("Application", "应用程序退出", createDetailsJson(details));
}

void AsyncLogger::setLogLevel(LogLevel minLevel)
{
    minLogLevel = minLevel;
}

void AsyncLogger::setLogFilePath(const QString& filePath)
{
    logFilePath = filePath;
}

void AsyncLogger::setMaxLogFileSize(qint64 maxSize)
{
    maxLogFileSize = maxSize;
}

void AsyncLogger::setAutoBackup(bool enabled)
{
    autoBackupEnabled = enabled;
}

void AsyncLogger::log(LogLevel level, const QString& category, const QString& message, const QString& details)
{
    if (level < minLogLevel) {
        return;
    }
    
    LogEntry entry(level, category, message, details);
    
    if (writerThread) {
        writerThread->addLogEntry(entry);
    }
    
    emit logEntryAdded(entry);
}

QString AsyncLogger::createDetailsJson(const QVariantMap& data) const
{
    QJsonObject jsonObj = QJsonObject::fromVariantMap(data);
    QJsonDocument doc(jsonObj);
    return doc.toJson(QJsonDocument::Compact);
}

void AsyncLogger::checkAndBackupLogFile()
{
    if (!autoBackupEnabled) return;
    
    // 这里可以实现日志文件大小检查和备份逻辑
    // 当前实现保持简单，实际使用时可以扩展
}