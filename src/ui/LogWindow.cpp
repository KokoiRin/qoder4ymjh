#include "ui/LogWindow.h"
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>

LogWindow::LogWindow(QWidget *parent)
    : QWidget(parent)
    , maxDisplayLines(1000)
    , autoScrollEnabled(true)
    , currentLogLevelFilter(LogLevel::Debug)
{
    setupUI();
    connectSignals();
}

LogWindow::~LogWindow()
{
    // Qt会自动清理子对象
}

void LogWindow::setupUI()
{
    mainLayout = new QVBoxLayout(this);
    
    // 创建日志窗口组
    logWindowGroup = new QGroupBox("日志窗口", this);
    logLayout = new QVBoxLayout(logWindowGroup);
    
    // 控制区域
    logControlLayout = new QHBoxLayout();
    
    clearLogButton = new QPushButton("清空日志", this);
    clearLogButton->setMaximumWidth(80);
    
    saveLogButton = new QPushButton("保存日志", this);
    saveLogButton->setMaximumWidth(80);
    
    autoScrollCheckBox = new QCheckBox("自动滚动", this);
    autoScrollCheckBox->setChecked(autoScrollEnabled);
    
    QLabel* filterLabel = new QLabel("级别过滤:", this);
    logLevelFilter = new QComboBox(this);
    logLevelFilter->addItem("全部", static_cast<int>(LogLevel::Debug));
    logLevelFilter->addItem("调试", static_cast<int>(LogLevel::Debug));
    logLevelFilter->addItem("信息", static_cast<int>(LogLevel::Info));
    logLevelFilter->addItem("警告", static_cast<int>(LogLevel::Warning));
    logLevelFilter->addItem("错误", static_cast<int>(LogLevel::Error));
    logLevelFilter->addItem("操作", static_cast<int>(LogLevel::Action));
    
    logCountLabel = new QLabel("日志: 0 条", this);
    logCountLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    logCountLabel->setStyleSheet("QLabel { color: #666; }");
    
    logControlLayout->addWidget(clearLogButton);
    logControlLayout->addWidget(saveLogButton);
    logControlLayout->addWidget(autoScrollCheckBox);
    logControlLayout->addWidget(filterLabel);
    logControlLayout->addWidget(logLevelFilter);
    logControlLayout->addStretch();
    logControlLayout->addWidget(logCountLabel);
    
    logLayout->addLayout(logControlLayout);
    
    // 日志显示区域
    logTextEdit = new QTextEdit(this);
    logTextEdit->setReadOnly(true);
    logTextEdit->setMinimumHeight(200);
    logTextEdit->setMaximumHeight(300);
    logTextEdit->setFont(QFont("Consolas", 9));
    logTextEdit->setStyleSheet(
        "QTextEdit {"
        "    background-color: #f8f8f8;"
        "    border: 1px solid #ddd;"
        "    padding: 5px;"
        "}"
    );
    
    logLayout->addWidget(logTextEdit);
    
    mainLayout->addWidget(logWindowGroup);
}

void LogWindow::connectSignals()
{
    // 连接日志窗口信号
    connect(clearLogButton, &QPushButton::clicked, this, &LogWindow::onClearLog);
    connect(saveLogButton, &QPushButton::clicked, this, &LogWindow::onSaveLog);
    connect(autoScrollCheckBox, &QCheckBox::toggled, this, &LogWindow::onAutoScrollToggled);
    connect(logLevelFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &LogWindow::onLogLevelFilterChanged);
    
    // 连接异步日志系统
    connect(AsyncLogger::instance(), &AsyncLogger::logEntryAdded, 
            this, &LogWindow::onLogEntryAdded);
}

// ============ 公共接口实现 ============

void LogWindow::setMaxDisplayLines(int maxLines)
{
    maxDisplayLines = maxLines;
    while (displayedLogs.size() > maxDisplayLines) {
        displayedLogs.removeFirst();
    }
    updateLogDisplay();
    updateLogCount();
}

int LogWindow::getMaxDisplayLines() const
{
    return maxDisplayLines;
}

void LogWindow::setAutoScrollEnabled(bool enabled)
{
    autoScrollEnabled = enabled;
    autoScrollCheckBox->setChecked(enabled);
    if (enabled) {
        scrollToBottom();
    }
}

bool LogWindow::isAutoScrollEnabled() const
{
    return autoScrollEnabled;
}

void LogWindow::setLogLevelFilter(LogLevel level)
{
    currentLogLevelFilter = level;
    // 更新组合框选择
    for (int i = 0; i < logLevelFilter->count(); ++i) {
        if (static_cast<LogLevel>(logLevelFilter->itemData(i).toInt()) == level) {
            logLevelFilter->setCurrentIndex(i);
            break;
        }
    }
    updateLogDisplay();
    updateLogCount();
}

LogLevel LogWindow::getLogLevelFilter() const
{
    return currentLogLevelFilter;
}

void LogWindow::clearLogs()
{
    displayedLogs.clear();
    logTextEdit->clear();
    updateLogCount();
}

void LogWindow::saveLogsToFile()
{
    onSaveLog();
}

int LogWindow::getVisibleLogCount() const
{
    int visibleCount = 0;
    for (const LogEntry& entry : displayedLogs) {
        if (shouldDisplayLog(entry)) {
            visibleCount++;
        }
    }
    return visibleCount;
}

// ============ 槽函数实现 ============

void LogWindow::onLogEntryAdded(const LogEntry& entry)
{
    if (!shouldDisplayLog(entry)) {
        return;
    }
    
    // 添加到显示列表
    displayedLogs.append(entry);
    
    // 限制显示数量
    while (displayedLogs.size() > maxDisplayLines) {
        displayedLogs.removeFirst();
    }
    
    updateLogDisplay();
    updateLogCount();
    
    if (autoScrollEnabled) {
        scrollToBottom();
    }
}

void LogWindow::onClearLog()
{
    clearLogs();
    LOG_ACTION("LogWindow", "用户清空了日志显示窗口");
}

void LogWindow::onSaveLog()
{
    QString fileName = QFileDialog::getSaveFileName(this, 
        "保存日志文件", 
        QString("QtDemo_log_%1.txt").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss")),
        "Text Files (*.txt);;All Files (*)");
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        // Qt 6中QTextStream默认使用UTF-8编码，无需设置setCodec
        
        for (const LogEntry& entry : displayedLogs) {
            stream << formatLogEntry(entry) << "\n";
        }
        
        file.close();
        
        // 通过信号通知父窗口保存成功（如果需要的话）
        QMessageBox::information(this, "保存成功", 
                                QString("日志已保存到: %1").arg(fileName));
        LOG_ACTION("LogWindow", QString("用户保存日志到文件: %1").arg(fileName));
    } else {
        QMessageBox::warning(this, "保存失败", 
                           QString("无法写入文件: %1").arg(fileName));
        LOG_ERROR("LogWindow", "保存日志失败", QString("无法写入文件: %1").arg(fileName));
    }
}

void LogWindow::onAutoScrollToggled(bool enabled)
{
    autoScrollEnabled = enabled;
    if (enabled) {
        scrollToBottom();
    }
    LOG_ACTION("LogWindow", QString("用户%1自动滚动").arg(enabled ? "开启" : "关闭"));
}

void LogWindow::onLogLevelFilterChanged(int index)
{
    LogLevel newFilter;
    if (index == 0) {
        newFilter = LogLevel::Debug; // 全部
    } else {
        newFilter = static_cast<LogLevel>(logLevelFilter->itemData(index).toInt());
    }
    
    currentLogLevelFilter = newFilter;
    updateLogDisplay();
    updateLogCount();
    
    LOG_ACTION("LogWindow", QString("用户设置日志级别过滤: %1").arg(logLevelFilter->currentText()));
}

// ============ 日志相关辅助方法实现 ============

void LogWindow::updateLogDisplay()
{
    logTextEdit->clear();
    
    for (const LogEntry& entry : displayedLogs) {
        if (shouldDisplayLog(entry)) {
            QString formattedEntry = formatLogEntry(entry);
            logTextEdit->append(formattedEntry);
        }
    }
}

QString LogWindow::formatLogEntry(const LogEntry& entry) const
{
    QString timestamp = entry.timestamp.toString("yyyy-MM-dd hh:mm:ss");
    QString level = levelToString(entry.level);
    QString color = levelToColor(entry.level);
    
    QString formattedEntry = QString(
        "<span style='color: #666;'>%1</span> "
        "<span style='color: %2; font-weight: bold;'>[%3]</span> "
        "<span style='color: #333; font-weight: bold;'>%4:</span> "
        "<span style='color: #000;'>%5</span>"
    ).arg(timestamp).arg(color).arg(level).arg(entry.category).arg(entry.message);
    
    if (!entry.details.isEmpty()) {
        formattedEntry += QString(" <span style='color: #666; font-style: italic;'>(%1)</span>")
            .arg(entry.details);
    }
    
    return formattedEntry;
}

QString LogWindow::levelToString(LogLevel level) const
{
    switch (level) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Action: return "ACTION";
        default: return "UNKNOWN";
    }
}

QString LogWindow::levelToColor(LogLevel level) const
{
    switch (level) {
        case LogLevel::Debug: return "#808080";    // 灰色
        case LogLevel::Info: return "#0066cc";     // 蓝色
        case LogLevel::Warning: return "#ff8800";  // 橙色
        case LogLevel::Error: return "#cc0000";    // 红色
        case LogLevel::Action: return "#008800";   // 绿色
        default: return "#000000";                 // 黑色
    }
}

void LogWindow::scrollToBottom()
{
    QScrollBar* scrollBar = logTextEdit->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void LogWindow::updateLogCount()
{
    int visibleCount = getVisibleLogCount();
    logCountLabel->setText(QString("日志: %1 条").arg(visibleCount));
}

bool LogWindow::shouldDisplayLog(const LogEntry& entry) const
{
    if (logLevelFilter->currentIndex() == 0) {
        return true; // 全部
    }
    
    LogLevel filterLevel = static_cast<LogLevel>(logLevelFilter->currentData().toInt());
    return entry.level == filterLevel;
}