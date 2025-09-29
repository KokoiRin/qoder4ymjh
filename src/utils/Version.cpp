#include "utils/Version.h"
#include <QDateTime>
#include <QStringList>

// 预发布标识
const QString Version::PRE_RELEASE = "";  // 正式版本为空，预发布版本如 "beta.1"

// 构建信息
const QString Version::BUILD_DATE = __DATE__ " " __TIME__;
const QString Version::BUILD_NUMBER = "001";
const QString Version::GIT_COMMIT = "";  // 实际项目中可通过CMake或Git钩子自动生成

// 项目信息
const QString Version::PROJECT_NAME = "高级窗口操作工具";
const QString Version::PROJECT_DESCRIPTION = "基于Qt的高级窗口操作、颜色拾取和点击模拟工具";
const QString Version::COMPANY_NAME = "开发团队";
const QString Version::COPYRIGHT = QString("Copyright © %1 %2. All rights reserved.")
                                   .arg(QDateTime::currentDateTime().date().year())
                                   .arg(COMPANY_NAME);

QString Version::getVersionString()
{
    QString version = QString("%1.%2.%3")
                     .arg(MAJOR_VERSION)
                     .arg(MINOR_VERSION)
                     .arg(PATCH_VERSION);
    
    if (!PRE_RELEASE.isEmpty()) {
        version += "-" + PRE_RELEASE;
    }
    
    if (!BUILD_NUMBER.isEmpty()) {
        version += "+" + BUILD_NUMBER;
    }
    
    return version;
}

QString Version::getShortVersionString()
{
    return QString("%1.%2.%3")
           .arg(MAJOR_VERSION)
           .arg(MINOR_VERSION)
           .arg(PATCH_VERSION);
}

QString Version::getBuildInfo()
{
    QStringList info;
    info << QString("构建日期: %1").arg(BUILD_DATE);
    
    if (!BUILD_NUMBER.isEmpty()) {
        info << QString("构建编号: %1").arg(BUILD_NUMBER);
    }
    
    if (!GIT_COMMIT.isEmpty()) {
        info << QString("Git提交: %1").arg(GIT_COMMIT.left(8));
    }
    
    return info.join(", ");
}

QString Version::getFullInfo()
{
    QStringList info;
    info << QString("%1 v%2").arg(PROJECT_NAME, getVersionString());
    info << PROJECT_DESCRIPTION;
    info << "";
    info << getBuildInfo();
    info << "";
    info << COPYRIGHT;
    
    return info.join("\n");
}

int Version::compareVersion(int major, int minor, int patch)
{
    if (MAJOR_VERSION > major) return 1;
    if (MAJOR_VERSION < major) return -1;
    
    if (MINOR_VERSION > minor) return 1;
    if (MINOR_VERSION < minor) return -1;
    
    if (PATCH_VERSION > patch) return 1;
    if (PATCH_VERSION < patch) return -1;
    
    return 0;
}

bool Version::isNewerThan(int major, int minor, int patch)
{
    return compareVersion(major, minor, patch) > 0;
}

bool Version::isOlderThan(int major, int minor, int patch)
{
    return compareVersion(major, minor, patch) < 0;
}

bool Version::hasFeature(const QString& featureName)
{
    QStringList supportedFeatures = getSupportedFeatures();
    return supportedFeatures.contains(featureName, Qt::CaseInsensitive);
}

QStringList Version::getSupportedFeatures()
{
    QStringList features;
    
    // 核心功能
    features << "WindowManagement";    // 窗口管理
    features << "ColorPicking";        // 颜色拾取
    features << "ClickSimulation";     // 点击模拟
    features << "KeyboardSimulation";  // 键盘模拟
    features << "WindowCapture";       // 窗口捕获
    features << "CoordinateConversion"; // 坐标转换
    features << "CoordinateDisplay";   // 坐标显示
    
    // 图像处理功能
    features << "ImageProcessing";     // 图像处理
    features << "ImageResize";         // 图像缩放
    features << "ImageRotation";       // 图像旋转
    features << "ImageFilters";        // 图像滤镜
    features << "TemplateMatching";    // 模板匹配
    
    // OCR功能
#ifdef _WIN32
    features << "OCRTextRecognition";  // OCR文字识别
    features << "OCRTextSearch";       // OCR文字搜索
    features << "OCRChineseSupport";   // OCR中文支持
#endif
    
    // OpenCV支持
#ifdef OPENCV_ENABLED
    features << "OpenCVSupport";       // OpenCV支持
    features << "AdvancedImageProcessing"; // 高级图像处理
    features << "ComputerVision";      // 计算机视觉
#endif
    
    // 平台特性
#ifdef _WIN32
    features << "WindowsAPI";          // Windows API支持
    features << "Win32Integration";    // Win32集成
#endif
    
    // UI功能
    features << "QtGUI";               // Qt图形界面
    features << "AsyncLogging";        // 异步日志
    features << "WindowPreview";       // 窗口预览
    features << "LogWindow";           // 日志窗口
    
    // 高级功能
    features << "InteractionFacade";   // 交互门面模式
    features << "AsyncCapture";        // 异步捕获
    features << "GpuAcceleration";     // GPU加速（如果支持）
    
    return features;
}