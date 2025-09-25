# GitHub 分支保护设置指南

由于GitHub API需要特殊权限，请在GitHub网页上手动配置分支保护规则：

## 📝 分支保护配置步骤

### 1. 访问仓库设置
1. 打开 https://github.com/KokoiRin/qoder4ymjh
2. 点击 "Settings" 标签页
3. 在左侧菜单中选择 "Branches"

### 2. 添加分支保护规则
点击 "Add rule" 按钮，然后配置：

#### 基本设置
- **Branch name pattern**: `main`
- **Restrict pushes that create files over 100 MB**: ✅ (推荐)

#### Pull Request 设置
- **Require a pull request before merging**: ✅
  - **Require approvals**: ✅ (设置为 1)
  - **Dismiss stale PR approvals when new commits are pushed**: ✅
  - **Require review from code owners**: ✅ (如果有 CODEOWNERS 文件)
  - **Restrict push access to users that can push changes**: ✅

#### 状态检查设置
- **Require status checks to pass before merging**: ✅
  - **Require branches to be up to date before merging**: ✅
  - 添加状态检查: `build` (CI工作流的job名称)

#### 其他限制
- **Require signed commits**: ⚠️ (可选，增强安全性)
- **Require linear history**: ✅ (推荐，保持清晰的历史)
- **Include administrators**: ✅ (管理员也需要遵循规则)
- **Allow force pushes**: ❌
- **Allow deletions**: ❌

### 3. 为 develop 分支设置保护 (可选)
重复上述步骤，Branch name pattern 设置为 `develop`

## 🔒 CODEOWNERS 文件 (可选)

创建 `.github/CODEOWNERS` 文件来指定代码审查者：

```
# 全局代码所有者
* @KokoiRin

# 核心模块需要更严格的审查
/src/core/ @KokoiRin
/include/core/ @KokoiRin

# 构建配置文件
CMakeLists.txt @KokoiRin
*.ps1 @KokoiRin
*.bat @KokoiRin

# 文档文件
*.md @KokoiRin
```

## ✅ 验证配置

配置完成后，测试流程：

1. 创建新的功能分支：
   ```bash
   git checkout develop
   git checkout -b feature/test-protection
   echo "test" > test.txt
   git add test.txt
   git commit -m "test: 测试分支保护"
   git push origin feature/test-protection
   ```

2. 在GitHub上创建PR到main分支
3. 验证无法直接合并（需要审查）
4. 进行审查并批准
5. 验证可以合并

## 🎯 工作流程

设置完成后，开发流程将是：

1. **开发者**: 从 `develop` 创建功能分支
2. **开发者**: 推送代码并创建PR到 `main`
3. **审查者**: 在GitHub网页上审查代码
4. **审查者**: 批准或请求修改
5. **开发者**: 根据反馈修改（如需要）
6. **审查者**: 最终批准并合并

这样确保所有代码都经过您的审查才能进入主分支！