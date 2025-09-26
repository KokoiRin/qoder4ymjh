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
#include <QTabWidget>
// 引入我们的核心模块
#include "core/InteractionFacade.h"
#include "core/ColorPicker.h"
#include "ui/LogWindow.h"
#include "ui/WindowPreviewPage.h"

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
    
    // 键盘模拟
    void onSendKey();
    void onKeyExecuted(KeyCode key, const QString& modifiers);
    void onKeyFailed(const QString& reason);
    
    // 坐标显示
    void onCoordinateChanged(const QPoint& screenPos, const QPoint& windowPos, const QPoint& clientPos);
    void onCoordinateCaptured(const QPoint& position, CoordinateType coordType);
    void onToggleCoordinateDisplay();
    
    // 标签页切换
    void onTabChanged(int index);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private:
    // UI组件
    void setupUI();
    void setupWindowManagePage();
    void setupPreviewPage();
    void setupLogPage();
    
    // 核心模块 - 使用新的架构
    InteractionFacade* interactionFacade;
    ColorPicker* colorPicker;
    
    // 日志窗口
    LogWindow* logWindow;
    
    // UI控件
    QWidget* centralWidget;
    QVBoxLayout* mainLayout;
    QTabWidget* tabWidget;
    
    // 各个功能页面
    QWidget* windowManagePage;
    WindowPreviewPage* previewPage;
    

    
    // 窗口绑定区域
    QGroupBox* windowBindGroup;
    QComboBox* windowComboBox;
    QPushButton* refreshButton;
    QPushButton* bindButton;
    QPushButton* previewButton;
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
    
    // 键盘模拟区域
    QGroupBox* keySimulatorGroup;
    QComboBox* keyCombo;
    QLineEdit* textEdit;
    QCheckBox* ctrlCheckBox;
    QCheckBox* altCheckBox;
    QCheckBox* shiftCheckBox;
    QPushButton* sendKeyButton;
    QPushButton* sendTextButton;
    QSpinBox* keyDelaySpinBox;
    QLabel* keyStatusLabel;
    
    // 坐标显示区域
    QGroupBox* coordinateGroup;
    QLabel* coordinateDisplayLabel;
    QPushButton* toggleCoordinateButton;
    QLabel* captureKeyLabel;
    QComboBox* captureKeyCombo;
    QLabel* coordinateStatusLabel;
    
    // 辅助方法
    void updateWindowInfo();
    void updateColorDisplay(const QColor& color, const QPoint& position);
    void updateClickStatus(const QString& message, bool isError = false);
    void updateKeyStatus(const QString& message, bool isError = false);
    void updateCoordinateDisplay(const QPoint& screenPos, const QPoint& windowPos, const QPoint& clientPos);
    QString formatColorInfo(const QColor& color, const QPoint& position) const;
    void connectSignals();
};

#endif // MAINWINDOW_H