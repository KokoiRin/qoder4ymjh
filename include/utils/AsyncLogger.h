#ifndef ASYNCLOGGER_H
#define ASYNCLOGGER_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QQueue>
#include <QString>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QColor>
#include <QPoint>

// 日志级别枚举
enum class LogLevel {
    Debug,      // 调试信息
    Info,       // 一般信息
    Warning,    // 警告
    Error,      // 错误
    Action      // 用户操作记录
};

// 日志条目结构
struct LogEntry {
    QDateTime timestamp;
    LogLevel level;
    QString category;       // 日志分类（如：Button, ColorPicker, ClickSimulator）
    QString message;        // 日志消息
    QString details;        // 详细信息（JSON格式）
    
    LogEntry() = default;
    LogEntry(LogLevel lvl, const QString& cat, const QString& msg, const QString& det = QString())
        : timestamp(QDateTime::currentDateTime()), level(lvl), category(cat), message(msg), details(det) {}
};

// 日志写入线程
class LogWriterThread : public QThread
{
    Q_OBJECT

public:
    explicit LogWriterThread(QObject *parent = nullptr);
    ~LogWriterThread();

    void stop();
    void addLogEntry(const LogEntry& entry);

protected:
    void run() override;

private:
    bool isRunning;
    QMutex mutex;
    QWaitCondition condition;
    QQueue<LogEntry> logQueue;
    QFile logFile;
    QTextStream* textStream;
    
    void initLogFile();
    void writeLogEntry(const LogEntry& entry);
    QString formatLogEntry(const LogEntry& entry) const;
    QString levelToString(LogLevel level) const;
};

// 异步日志管理器
class AsyncLogger : public QObject
{
    Q_OBJECT

public:
    static AsyncLogger* instance();
    ~AsyncLogger();

    // 基础日志方法
    void debug(const QString& category, const QString& message, const QString& details = QString());
    void info(const QString& category, const QString& message, const QString& details = QString());
    void warning(const QString& category, const QString& message, const QString& details = QString());
    void error(const QString& category, const QString& message, const QString& details = QString());
    void action(const QString& category, const QString& message, const QString& details = QString());

    // 便捷的业务日志方法
    void logButtonClick(const QString& buttonName, const QString& additionalInfo = QString());
    void logColorPicked(const QColor& color, const QPoint& position, const QString& windowInfo = QString());
    void logClickSimulated(const QPoint& position, const QString& coordinateType, const QString& mouseButton, bool success);
    void logWindowBound(const QString& windowTitle, const QString& windowInfo);
    void logApplicationStart();
    void logApplicationExit();

    // 配置方法
    void setLogLevel(LogLevel minLevel);
    void setLogFilePath(const QString& filePath);
    void setMaxLogFileSize(qint64 maxSize);  // 最大日志文件大小（字节）
    void setAutoBackup(bool enabled);        // 是否自动备份日志文件

signals:
    void logEntryAdded(const LogEntry& entry);

private:
    explicit AsyncLogger(QObject* parent = nullptr);
    static AsyncLogger* m_instance;
    
    LogWriterThread* writerThread;
    LogLevel minLogLevel;
    QString logFilePath;
    qint64 maxLogFileSize;
    bool autoBackupEnabled;
    
    void log(LogLevel level, const QString& category, const QString& message, const QString& details = QString());
    QString createDetailsJson(const QVariantMap& data) const;
    void checkAndBackupLogFile();
};

// 便捷宏定义
#define LOG_DEBUG(category, message, ...) AsyncLogger::instance()->debug(category, message, ##__VA_ARGS__)
#define LOG_INFO(category, message, ...) AsyncLogger::instance()->info(category, message, ##__VA_ARGS__)
#define LOG_WARNING(category, message, ...) AsyncLogger::instance()->warning(category, message, ##__VA_ARGS__)
#define LOG_ERROR(category, message, ...) AsyncLogger::instance()->error(category, message, ##__VA_ARGS__)
#define LOG_ACTION(category, message, ...) AsyncLogger::instance()->action(category, message, ##__VA_ARGS__)

// 业务操作便捷宏
#define LOG_BUTTON_CLICK(buttonName, ...) AsyncLogger::instance()->logButtonClick(buttonName, ##__VA_ARGS__)
#define LOG_COLOR_PICKED(color, position, ...) AsyncLogger::instance()->logColorPicked(color, position, ##__VA_ARGS__)
#define LOG_CLICK_SIMULATED(position, coordType, mouseButton, success) AsyncLogger::instance()->logClickSimulated(position, coordType, mouseButton, success)
#define LOG_WINDOW_BOUND(title, info) AsyncLogger::instance()->logWindowBound(title, info)

#endif // ASYNCLOGGER_H