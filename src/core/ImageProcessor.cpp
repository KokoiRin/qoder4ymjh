#include "core/ImageProcessor.h"
#include <QDebug>
#include <QImage>
#include <QColor>
#include <QDir>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QTransform>
#include <QPainter>
#include <QPixmap>
#include <QImageReader>
#include <QImageWriter>
#include <algorithm>
#include <future>
#include <thread>

// Tesseract OCR头文件
#ifdef TESSERACT_ENABLED
#ifdef _WIN32
// Windows平台Tesseract集成
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#pragma comment(lib, "tesseract50.lib")
#pragma comment(lib, "leptonica-1.82.0.lib")
#endif
#endif

ImageProcessor::ImageProcessor(QObject *parent)
    : QObject(parent)
    , gpuAccelerationEnabled(false)
    , processingThreads(std::thread::hardware_concurrency())
    , ocrLanguage("chi_sim")
{
}

ImageProcessor::~ImageProcessor()
{
}

// ========== 基础图像操作 ==========

ImageProcessor::ProcessResult ImageProcessor::resizeImage(const QImage& input, QImage& output, 
                                                         int width, int height, 
                                                         ScaleAlgorithm algorithm)
{
    if (!validateInputs(input) || width <= 0 || height <= 0) {
        return ProcessResult::InvalidInput;
    }

    Qt::TransformationMode mode;
    
    switch (algorithm) {
        case ScaleAlgorithm::Nearest:
            mode = Qt::FastTransformation;
            break;
        case ScaleAlgorithm::Linear:
        case ScaleAlgorithm::Cubic:
        case ScaleAlgorithm::Lanczos:
        default:
            mode = Qt::SmoothTransformation;
            break;
    }
    
    output = input.scaled(width, height, Qt::IgnoreAspectRatio, mode);
    return ProcessResult::Success;
}

ImageProcessor::ProcessResult ImageProcessor::rotateImage(const QImage& input, QImage& output, 
                                                        double angle, bool autoResize)
{
    if (!validateInputs(input)) {
        return ProcessResult::InvalidInput;
    }

    QTransform transform;
    transform.rotate(angle);
    
    if (autoResize) {
        output = input.transformed(transform, Qt::SmoothTransformation);
    } else {
        // 保持原始尺寸
        QRect bounds = transform.mapRect(input.rect());
        QImage temp = input.transformed(transform, Qt::SmoothTransformation);
        
        // 裁剪到原始尺寸
        int x = (temp.width() - input.width()) / 2;
        int y = (temp.height() - input.height()) / 2;
        output = temp.copy(x, y, input.width(), input.height());
    }
    
    return ProcessResult::Success;
}

ImageProcessor::ProcessResult ImageProcessor::cropImage(const QImage& input, QImage& output,
                                                      int x, int y, int width, int height)
{
    if (!validateInputs(input) || x < 0 || y < 0 || width <= 0 || height <= 0 ||
        x + width > input.width() || y + height > input.height()) {
        return ProcessResult::InvalidInput;
    }

    output = input.copy(x, y, width, height);
    return ProcessResult::Success;
}

// ========== 图像增强和滤镜 ==========

ImageProcessor::ProcessResult ImageProcessor::applyFilter(const QImage& input, QImage& output, 
                                                         FilterType filter, double intensity)
{
    if (!validateInputs(input) || intensity < 0.0 || intensity > 2.0) {
        return ProcessResult::InvalidInput;
    }

    if (filter == FilterType::None) {
        output = input;
        return ProcessResult::Success;
    }

    switch (filter) {
        case FilterType::Blur: {
            // 使用Qt的模糊效果
            output = applyGaussianBlur(input, static_cast<int>(intensity * 10));
            break;
        }
        case FilterType::Sharpen: {
            // 使用Qt的锐化效果
            output = applySharpen(input, intensity);
            break;
        }
        case FilterType::EdgeDetection: {
            // 简单的边缘检测
            output = applyEdgeDetection(input, intensity);
            break;
        }
        case FilterType::Grayscale: {
            output = input.convertToFormat(QImage::Format_Grayscale8);
            break;
        }
        case FilterType::Sepia: {
            output = applySepia(input);
            break;
        }
        case FilterType::Negative: {
            output = applyNegative(input);
            break;
        }
        default:
            output = input;
            break;
    }
    
    return ProcessResult::Success;
}

// ========== OCR文字识别功能 ==========

// Qt实现的滤镜辅助方法
QImage ImageProcessor::applyGaussianBlur(const QImage& image, int radius)
{
    if (radius <= 0) return image;
    
    QImage result = image;
    QPainter painter(&result);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    
    // 简单的模糊实现：通过多次缩放来模拟模糊效果
    for (int i = 0; i < radius / 2; ++i) {
        QImage temp = result.scaled(result.size() * 0.9, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        result = temp.scaled(result.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    
    return result;
}

QImage ImageProcessor::applySharpen(const QImage& image, double intensity)
{
    QImage result = image.copy();
    
    // 简单的锐化实现：增强对比度
    for (int y = 1; y < result.height() - 1; ++y) {
        for (int x = 1; x < result.width() - 1; ++x) {
            QColor center = result.pixelColor(x, y);
            QColor left = result.pixelColor(x-1, y);
            QColor right = result.pixelColor(x+1, y);
            QColor top = result.pixelColor(x, y-1);
            QColor bottom = result.pixelColor(x, y+1);
            
            int r = qBound(0, static_cast<int>(center.red() + intensity * 
                (4 * center.red() - left.red() - right.red() - top.red() - bottom.red())), 255);
            int g = qBound(0, static_cast<int>(center.green() + intensity * 
                (4 * center.green() - left.green() - right.green() - top.green() - bottom.green())), 255);
            int b = qBound(0, static_cast<int>(center.blue() + intensity * 
                (4 * center.blue() - left.blue() - right.blue() - top.blue() - bottom.blue())), 255);
            
            result.setPixelColor(x, y, QColor(r, g, b, center.alpha()));
        }
    }
    
    return result;
}

QImage ImageProcessor::applyEdgeDetection(const QImage& image, double intensity)
{
    QImage gray = image.convertToFormat(QImage::Format_Grayscale8);
    QImage result(gray.size(), QImage::Format_RGB32);
    result.fill(Qt::black);
    
    // 简单的Sobel边缘检测
    for (int y = 1; y < gray.height() - 1; ++y) {
        for (int x = 1; x < gray.width() - 1; ++x) {
            int gx = -1 * qGray(gray.pixel(x-1, y-1)) + 1 * qGray(gray.pixel(x+1, y-1)) +
                     -2 * qGray(gray.pixel(x-1, y)) + 2 * qGray(gray.pixel(x+1, y)) +
                     -1 * qGray(gray.pixel(x-1, y+1)) + 1 * qGray(gray.pixel(x+1, y+1));
            
            int gy = -1 * qGray(gray.pixel(x-1, y-1)) - 2 * qGray(gray.pixel(x, y-1)) - 1 * qGray(gray.pixel(x+1, y-1)) +
                     1 * qGray(gray.pixel(x-1, y+1)) + 2 * qGray(gray.pixel(x, y+1)) + 1 * qGray(gray.pixel(x+1, y+1));
            
            int magnitude = qBound(0, static_cast<int>(sqrt(gx*gx + gy*gy) * intensity), 255);
            result.setPixel(x, y, qRgb(magnitude, magnitude, magnitude));
        }
    }
    
    return result;
}

QImage ImageProcessor::applySepia(const QImage& image)
{
    QImage result = image.copy();
    
    for (int y = 0; y < result.height(); ++y) {
        for (int x = 0; x < result.width(); ++x) {
            QColor pixel = result.pixelColor(x, y);
            
            int r = qBound(0, static_cast<int>(0.393 * pixel.red() + 0.769 * pixel.green() + 0.189 * pixel.blue()), 255);
            int g = qBound(0, static_cast<int>(0.349 * pixel.red() + 0.686 * pixel.green() + 0.168 * pixel.blue()), 255);
            int b = qBound(0, static_cast<int>(0.272 * pixel.red() + 0.534 * pixel.green() + 0.131 * pixel.blue()), 255);
            
            result.setPixelColor(x, y, QColor(r, g, b, pixel.alpha()));
        }
    }
    
    return result;
}

QImage ImageProcessor::applyNegative(const QImage& image)
{
    QImage result = image.copy();
    
    for (int y = 0; y < result.height(); ++y) {
        for (int x = 0; x < result.width(); ++x) {
            QColor pixel = result.pixelColor(x, y);
            result.setPixelColor(x, y, QColor(255 - pixel.red(), 255 - pixel.green(), 255 - pixel.blue(), pixel.alpha()));
        }
    }
    
    return result;
}

ImageProcessor::ProcessResult ImageProcessor::recognizeText(const QImage& input, QString& recognizedText, const QString& language)
{
    if (!validateInputs(input)) {
        return ProcessResult::InvalidInput;
    }

#ifdef TESSERACT_ENABLED
#ifdef _WIN32
    try {
        // 初始化Tesseract
        tesseract::TessBaseAPI api;
        if (api.Init(nullptr, language.toLocal8Bit().constData()) != 0) {
            handleError("Failed to initialize Tesseract");
            return ProcessResult::ProcessingError;
        }

        // 预处理图像以提高OCR精度
        QImage processedImage;
        if (preprocessForOCR(input, processedImage) != ProcessResult::Success) {
            processedImage = input;
        }

        // 转换为适合Tesseract的格式
        QImage grayImage = processedImage.convertToFormat(QImage::Format_Grayscale8);
        
        // 设置图像数据
        api.SetImage(grayImage.constBits(), grayImage.width(), grayImage.height(), 
                    1, grayImage.bytesPerLine());

        // 执行OCR
        char* text = api.GetUTF8Text();
        if (text) {
            recognizedText = QString::fromUtf8(text);
            delete[] text;
        } else {
            recognizedText.clear();
        }

        return ProcessResult::Success;
    } catch (const std::exception& e) {
        handleError(QString("OCR error: %1").arg(e.what()));
        return ProcessResult::ProcessingError;
    }
#else
    Q_UNUSED(input)
    Q_UNUSED(recognizedText)
    Q_UNUSED(language)
    handleError("OCR not supported on this platform");
    return ProcessResult::NotSupported;
#endif
#else
    Q_UNUSED(input)
    Q_UNUSED(recognizedText)
    Q_UNUSED(language)
    handleError("OCR not available - Tesseract not found");
    return ProcessResult::NotSupported;
#endif
}

bool ImageProcessor::searchText(const QImage& input, const QString& searchString, QRect& foundRect, double& confidence)
{
    QString recognizedText;
    if (recognizeText(input, recognizedText) != ProcessResult::Success) {
        return false;
    }

    // 简单的文本搜索实现
    int index = recognizedText.indexOf(searchString, 0, Qt::CaseInsensitive);
    if (index != -1) {
        // 这里应该根据OCR结果的详细信息来确定文本位置
        // 简化实现，返回整个图像区域
        foundRect = QRect(0, 0, input.width(), input.height());
        confidence = 0.8; // 假设的置信度
        return true;
    }

    return false;
}

ImageProcessor::ProcessResult ImageProcessor::preprocessForOCR(const QImage& input, QImage& output)
{
    if (!validateInputs(input)) {
        return ProcessResult::InvalidInput;
    }

    // 转换为灰度
    QImage grayImage = input.convertToFormat(QImage::Format_Grayscale8);
    
    // 应用滤镜提高文字清晰度
    ProcessResult result = applyFilter(grayImage, output, FilterType::Sharpen, 0.5);
    
    return result;
}

// ========== 模板匹配 ==========

ImageProcessor::ProcessResult ImageProcessor::templateMatch(const QImage& source, const QImage& template_,
                                                           QPoint& bestMatch, double& confidence)
{
    if (!validateInputs(source) || !validateInputs(template_)) {
        return ProcessResult::InvalidInput;
    }

    // Qt实现的简单模板匹配
    bestMatch = QPoint(0, 0);
    confidence = 0.0;
    double bestScore = -1.0;
    
    // 遍历源图像的每个可能位置
    for (int y = 0; y <= source.height() - template_.height(); ++y) {
        for (int x = 0; x <= source.width() - template_.width(); ++x) {
            double score = calculateTemplateScore(source, template_, x, y);
            if (score > bestScore) {
                bestScore = score;
                bestMatch = QPoint(x, y);
            }
        }
    }
    
    confidence = bestScore;
    return ProcessResult::Success;
}

// ========== 异步处理 ==========

// ========== 模板匹配辅助方法 ==========

double ImageProcessor::calculateTemplateScore(const QImage& source, const QImage& template_, int x, int y)
{
    if (x + template_.width() > source.width() || y + template_.height() > source.height()) {
        return -1.0;
    }
    
    double score = 0.0;
    int totalPixels = template_.width() * template_.height();
    
    for (int ty = 0; ty < template_.height(); ++ty) {
        for (int tx = 0; tx < template_.width(); ++tx) {
            QColor sourcePixel = source.pixelColor(x + tx, y + ty);
            QColor templatePixel = template_.pixelColor(tx, ty);
            
            // 计算颜色相似度
            double rDiff = qAbs(sourcePixel.red() - templatePixel.red()) / 255.0;
            double gDiff = qAbs(sourcePixel.green() - templatePixel.green()) / 255.0;
            double bDiff = qAbs(sourcePixel.blue() - templatePixel.blue()) / 255.0;
            
            double pixelScore = 1.0 - (rDiff + gDiff + bDiff) / 3.0;
            score += pixelScore;
        }
    }
    
    return score / totalPixels;
}

void ImageProcessor::resizeImageAsync(const QImage& input, int width, int height,
                                     ScaleAlgorithm algorithm, ProcessCallback callback)
{
    // 存储 future 对象以避免 nodiscard 警告
    auto future = std::async(std::launch::async, [this, input, width, height, algorithm, callback]() {
        QImage output;
        ProcessResult result = resizeImage(input, output, width, height, algorithm);
        callback(result, output);
    });
    // future 对象会在作用域结束时自动析构
    (void)future; // 明确表示我们不使用返回值
}

void ImageProcessor::applyFilterAsync(const QImage& input, FilterType filter, 
                                     double intensity, ProcessCallback callback)
{
    // 存储 future 对象以避免 nodiscard 警告
    auto future = std::async(std::launch::async, [this, input, filter, intensity, callback]() {
        QImage output;
        ProcessResult result = applyFilter(input, output, filter, intensity);
        callback(result, output);
    });
    // future 对象会在作用域结束时自动析构
    (void)future; // 明确表示我们不使用返回值
}

// ========== 实用功能 ==========

bool ImageProcessor::isValidImage(const QImage& image)
{
    return !image.isNull() && image.width() > 0 && image.height() > 0;
}

QString ImageProcessor::getImageInfo(const QImage& image)
{
    if (!isValidImage(image)) {
        return "Invalid image";
    }

    return QString("Size: %1x%2, Format: %3, Depth: %4 bits")
           .arg(image.width())
           .arg(image.height())
           .arg(static_cast<int>(image.format()))
           .arg(image.depth());
}

size_t ImageProcessor::calculateImageMemorySize(const QImage& image)
{
    if (!isValidImage(image)) {
        return 0;
    }
    
    return image.sizeInBytes();
}

// ========== 配置方法 ==========

void ImageProcessor::setGpuAcceleration(bool enable)
{
    gpuAccelerationEnabled = enable;
}

bool ImageProcessor::isGpuAccelerationEnabled() const
{
    return gpuAccelerationEnabled;
}

void ImageProcessor::setProcessingThreads(int threadCount)
{
    if (threadCount > 0) {
        processingThreads = threadCount;
    }
}

int ImageProcessor::getProcessingThreads() const
{
    return processingThreads;
}

void ImageProcessor::setOCRLanguage(const QString& language)
{
    ocrLanguage = language;
}

QString ImageProcessor::getOCRLanguage() const
{
    return ocrLanguage;
}

bool ImageProcessor::isOCRAvailable() const
{
#ifdef TESSERACT_ENABLED
#ifdef _WIN32
    return true; // 简化检查
#else
    return false;
#endif
#else
    return false;
#endif
}

// ========== 私有方法 ==========

void ImageProcessor::handleError(const QString& errorMessage)
{
    lastErrorMessage = errorMessage;
    emit processingError(errorMessage);
    qWarning() << "ImageProcessor Error:" << errorMessage;
}

bool ImageProcessor::validateInputs(const QImage& input) const
{
    return isValidImage(input);
}

// ========== 工具命名空间实现 ==========

namespace ImageProcessorUtils {

QImage convertFormat(const QImage& image, QImage::Format format)
{
    return image.convertToFormat(format);
}

QImage quickResize(const QImage& image, const QSize& size)
{
    return image.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QImage toGrayscale(const QImage& image)
{
    return image.convertToFormat(QImage::Format_Grayscale8);
}

QString calculateImageHash(const QImage& image)
{
    // 简化的图像哈希计算
    QCryptographicHash hash(QCryptographicHash::Md5);
    
    QImage scaledImage = image.scaled(8, 8, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    QImage grayImage = scaledImage.convertToFormat(QImage::Format_Grayscale8);
    
    hash.addData(QByteArrayView(reinterpret_cast<const char*>(grayImage.constBits()), grayImage.sizeInBytes()));
    
    return hash.result().toHex();
}

}