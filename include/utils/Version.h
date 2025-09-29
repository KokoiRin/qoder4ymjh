#ifndef VERSION_H
#define VERSION_H

#include <QString>

/**
 * Version - 版本信息管理
 * 
 * 统一管理项目版本信息，支持语义化版本控制
 * 版本格式：主版本.次版本.修订版本-预发布标识+构建信息
 * 例如：2.1.0-beta.1+20241227.001
 */
class Version
{
public:
    // 版本号常量
    static constexpr int MAJOR_VERSION = 2;     // 主版本号（重大功能变更）
    static constexpr int MINOR_VERSION = 1;     // 次版本号（功能增加）
    static constexpr int PATCH_VERSION = 0;     // 修订版本号（bug修复）
    
    // 预发布标识
    static const QString PRE_RELEASE;           // 预发布版本标识
    
    // 构建信息
    static const QString BUILD_DATE;            // 构建日期
    static const QString BUILD_NUMBER;          // 构建编号
    static const QString GIT_COMMIT;            // Git提交ID（如果可用）
    
    // 项目信息
    static const QString PROJECT_NAME;          // 项目名称
    static const QString PROJECT_DESCRIPTION;   // 项目描述
    static const QString COMPANY_NAME;          // 公司/组织名称
    static const QString COPYRIGHT;             // 版权信息
    
    // 版本字符串生成
    static QString getVersionString();          // 获取完整版本字符串
    static QString getShortVersionString();     // 获取简短版本字符串
    static QString getBuildInfo();              // 获取构建信息
    static QString getFullInfo();               // 获取完整信息
    
    // 版本比较
    static int compareVersion(int major, int minor, int patch);
    static bool isNewerThan(int major, int minor, int patch);
    static bool isOlderThan(int major, int minor, int patch);
    
    // 特性检查
    static bool hasFeature(const QString& featureName);
    static QStringList getSupportedFeatures();
    
private:
    Version() = default; // 静态类，不允许实例化
};

// 便捷的版本宏定义
#define VERSION_STRING Version::getVersionString()
#define SHORT_VERSION Version::getShortVersionString()
#define BUILD_INFO Version::getBuildInfo()

#endif // VERSION_H