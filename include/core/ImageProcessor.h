#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QObject>
#include <QImage>
#include <QString>
#include <memory>
#include <functional>


/**
 * ImageProcessor - 图像处理模块
 * 
 * 基于Qt实现的图像处理功能，为WindowCapture提供图像处理支持：
 * 1. 基础图像处理（缩放、旋转、滤镜等）
 * 2. 图像分析（边缘检测、特征提取等）
 * 3. 模板匹配和目标检测
 * 4. OCR文字识别（可选，需要Tesseract支持）
 * 
 * 设计原则：
 * - 完全独立于Qt UI，纯图像处理逻辑
 * - 提供简洁的C++接口
 * - 支持异步处理
 * - 高性能优化
 */
class ImageProcessor : public QObject
{
    Q_OBJECT

public:
    // 图像处理结果状态
    enum class ProcessResult {
        Success,         // 成功
        InvalidInput,    // 输入无效
        ProcessingError, // 处理错误
        NotSupported,    // 不支持的操作
        OutOfMemory      // 内存不足
    };

    // 图像滤镜类型
    enum class FilterType {
        None,           // 无滤镜
        Blur,           // 模糊
        Sharpen,        // 锐化
        EdgeDetection,  // 边缘检测
        Emboss,         // 浮雕
        Grayscale,      // 灰度
        Sepia,          // 褐色
        Negative        // 反色
    };

    // 缩放算法
    enum class ScaleAlgorithm {
        Nearest,        // 最近邻
        Linear,         // 线性插值
        Cubic,          // 三次插值
        Lanczos         // Lanczos插值
    };

    explicit ImageProcessor(QObject *parent = nullptr);
    ~ImageProcessor();

    // ========== 基础图像操作 ==========
    
    // 图像缩放
    ProcessResult resizeImage(const QImage& input, QImage& output, 
                             int width, int height, 
                             ScaleAlgorithm algorithm = ScaleAlgorithm::Linear);
    
    // 图像旋转
    ProcessResult rotateImage(const QImage& input, QImage& output, 
                             double angle, bool autoResize = true);
    
    // 图像裁剪
    ProcessResult cropImage(const QImage& input, QImage& output,
                           int x, int y, int width, int height);

    // ========== 图像增强和滤镜 ==========
    
    // 应用滤镜
    ProcessResult applyFilter(const QImage& input, QImage& output, 
                             FilterType filter, double intensity = 1.0);
    
    // 调整亮度对比度
    ProcessResult adjustBrightnessContrast(const QImage& input, QImage& output,
                                          double brightness, double contrast);
    
    // 调整色调饱和度
    ProcessResult adjustHueSaturation(const QImage& input, QImage& output,
                                     double hue, double saturation);

    // ========== 图像分析 ==========
    
    // 计算图像相似度
    double calculateSimilarity(const QImage& image1, const QImage& image2);
    
    // 检测图像中的矩形
    ProcessResult detectRectangles(const QImage& input, 
                                  std::vector<QRect>& rectangles,
                                  double threshold = 0.02);
    
    // 模板匹配
    ProcessResult templateMatch(const QImage& source, const QImage& template_,
                               QPoint& bestMatch, double& confidence);
                               
    // 新增：OCR文字识别功能
    ProcessResult recognizeText(const QImage& input, QString& recognizedText, const QString& language = "chi_sim");
    bool searchText(const QImage& input, const QString& searchString, QRect& foundRect, double& confidence);
    ProcessResult preprocessForOCR(const QImage& input, QImage& output); // 预处理以提高OCR精度

    // ========== 异步处理 ==========
    
    // 异步图像处理回调类型
    using ProcessCallback = std::function<void(ProcessResult result, const QImage& output)>;
    
    // 异步缩放
    void resizeImageAsync(const QImage& input, int width, int height,
                         ScaleAlgorithm algorithm, ProcessCallback callback);
    
    // 异步滤镜应用
    void applyFilterAsync(const QImage& input, FilterType filter, 
                         double intensity, ProcessCallback callback);

    // ========== 实用功能 ==========
    
    // 检查图像是否有效
    static bool isValidImage(const QImage& image);
    
    // 获取图像信息
    static QString getImageInfo(const QImage& image);
    
    // 计算图像内存大小
    static size_t calculateImageMemorySize(const QImage& image);

    // ========== 配置方法 ==========
    
    // 设置是否启用GPU加速（如果可用）
    void setGpuAcceleration(bool enable);
    bool isGpuAccelerationEnabled() const;
    
    // 设置处理线程数
    void setProcessingThreads(int threadCount);
    int getProcessingThreads() const;
    
    // 新增：OCR配置
    void setOCRLanguage(const QString& language);
    QString getOCRLanguage() const;
    bool isOCRAvailable() const;

signals:
    // 异步处理完成信号
    void processingCompleted(ProcessResult result, const QImage& output);
    
    // 处理进度信号（对于长时间运行的操作）
    void processingProgress(int percentage);
    
    // 错误信号
    void processingError(const QString& errorMessage);

private:
    // 内部实现方法
    void handleError(const QString& errorMessage);
    
    // 验证输入参数
    bool validateInputs(const QImage& input) const;
    
    // Qt滤镜实现辅助方法
    QImage applyGaussianBlur(const QImage& image, int radius);
    QImage applySharpen(const QImage& image, double intensity);
    QImage applyEdgeDetection(const QImage& image, double intensity);
    QImage applySepia(const QImage& image);
    QImage applyNegative(const QImage& image);
    
    // 模板匹配辅助方法
    double calculateTemplateScore(const QImage& source, const QImage& template_, int x, int y);

private:
    // 配置参数
    bool gpuAccelerationEnabled;
    int processingThreads;
    QString ocrLanguage;      // OCR语言设置
    
    // 错误状态
    QString lastErrorMessage;
};

// 便捷的静态工具方法
namespace ImageProcessorUtils {
    // 快速格式转换
    QImage convertFormat(const QImage& image, QImage::Format format);
    
    // 快速缩放
    QImage quickResize(const QImage& image, const QSize& size);
    
    // 快速灰度转换
    QImage toGrayscale(const QImage& image);
    
    // 计算图像哈希（用于快速比较）
    QString calculateImageHash(const QImage& image);
}

#endif // IMAGEPROCESSOR_H