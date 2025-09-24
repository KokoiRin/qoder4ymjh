#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QMessageBox>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QApplication>
// 引入我们的核心模块
#include "core/WindowManager.h"
#include "core/ColorPicker.h"
#include "core/ClickSimulator.h"
#include "ui/LogWindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    
    // 窗口管理
    void onRefreshWindows();
    void onBindWindow();
    void onWindowSelectionChanged();
    
    // 取色功能
    void onStartColorPicker();
    void onColorChanged(const QColor& color, const QPoint& position);
    void onColorPicked(const QColor& color, const QPoint& position);
    void onPickingStarted();
    void onPickingStopped();
    
    // 点击模拟
    void onSimulateClick();
    void onClickExecuted(const QPoint& position, CoordinateType coordType, MouseButton button);
    void onClickFailed(const QString& reason);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    // UI组件
    void setupUI();
    void setupWindowBindingUI();
    void setupColorPickerUI();
    void setupClickSimulatorUI();
    
    // 核心模块
    WindowManager* windowManager;
    ColorPicker* colorPicker;
    ClickSimulator* clickSimulator;
    
    // 日志窗口
    LogWindow* logWindow;
    
    // UI控件
    QWidget* centralWidget;
    QVBoxLayout* mainLayout;
    

    
    // 窗口绑定区域
    QGroupBox* windowBindGroup;
    QComboBox* windowComboBox;
    QPushButton* refreshButton;
    QPushButton* bindButton;
    QLabel* windowInfoLabel;
    
    // 取色区域
    QGroupBox* colorPickerGroup;
    QPushButton* colorPickerButton;
    QLabel* colorDisplayLabel;
    QLabel* colorInfoLabel;
    QSpinBox* updateIntervalSpinBox;
    
    // 点击模拟区域
    QGroupBox* clickSimulatorGroup;
    QLineEdit* clickPosEdit;
    QComboBox* coordTypeCombo;
    QComboBox* mouseButtonCombo;
    QCheckBox* doubleClickCheckBox;
    QPushButton* simulateClickButton;
    QSpinBox* clickDelaySpinBox;
    QPushButton* bringToFrontButton;
    QLabel* clickStatusLabel;
    
    // 辅助方法
    void updateWindowInfo();
    void updateColorDisplay(const QColor& color, const QPoint& position);
    void updateClickStatus(const QString& message, bool isError = false);
    QString formatColorInfo(const QColor& color, const QPoint& position) const;
    void connectSignals();
};

#endif // MAINWINDOW_H