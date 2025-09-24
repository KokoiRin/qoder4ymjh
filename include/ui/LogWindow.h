#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QScrollBar>
#include <QFileDialog>
#include <QFont>
#include <QMessageBox>
#include <QList>

#include "utils/AsyncLogger.h"

/**
 * @brief 日志显示窗口类
 * 
 * 专门负责日志的显示、过滤、保存等功能，从MainWindow中分离出来
 * 遵循单一职责原则，只处理日志相关的UI和逻辑
 */
class LogWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LogWindow(QWidget *parent = nullptr);
    ~LogWindow();

    // 公共接口
    void setMaxDisplayLines(int maxLines);
    int getMaxDisplayLines() const;
    
    void setAutoScrollEnabled(bool enabled);
    bool isAutoScrollEnabled() const;
    
    void setLogLevelFilter(LogLevel level);
    LogLevel getLogLevelFilter() const;
    
    // 日志管理
    void clearLogs();
    void saveLogsToFile();
    int getVisibleLogCount() const;

public slots:
    void onLogEntryAdded(const LogEntry& entry);

private slots:
    void onClearLog();
    void onSaveLog();
    void onAutoScrollToggled(bool enabled);
    void onLogLevelFilterChanged(int index);

private:
    // UI初始化
    void setupUI();
    void connectSignals();
    
    // 日志显示相关方法
    void updateLogDisplay();
    QString formatLogEntry(const LogEntry& entry) const;
    QString levelToString(LogLevel level) const;
    QString levelToColor(LogLevel level) const;
    void scrollToBottom();
    void updateLogCount();
    bool shouldDisplayLog(const LogEntry& entry) const;
    
    // 数据成员
    QList<LogEntry> displayedLogs;
    int maxDisplayLines;
    bool autoScrollEnabled;
    LogLevel currentLogLevelFilter;
    
    // UI组件
    QVBoxLayout* mainLayout;
    QGroupBox* logWindowGroup;
    QVBoxLayout* logLayout;
    QHBoxLayout* logControlLayout;
    
    // 控制区域组件
    QPushButton* clearLogButton;
    QPushButton* saveLogButton;
    QCheckBox* autoScrollCheckBox;
    QComboBox* logLevelFilter;
    QLabel* logCountLabel;
    
    // 显示区域组件
    QTextEdit* logTextEdit;
};

#endif // LOGWINDOW_H