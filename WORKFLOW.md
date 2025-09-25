# Git 工作流指南

## 🌟 简化的代码审查工作流程

### 1. 创建功能分支
```bash
# 从main分支创建新的功能分支
git checkout main
git pull origin main
git checkout -b feature/your-feature-name

# 或者创建修复分支
git checkout -b bugfix/issue-description
```

### 2. 开发和提交
```bash
# 进行开发工作
# 编辑代码...

# 添加和提交变更
git add .
git commit -m "feat: 添加新功能描述"

# 或者使用常规提交格式
git commit -m "fix: 修复具体问题描述"
```

### 3. 推送分支并创建Pull Request
```bash
# 推送功能分支到远程仓库
git push origin feature/your-feature-name
```

然后在GitHub网页上：
1. 访问 https://github.com/KokoiRin/qoder4ymjh
2. 点击 "Compare & pull request" 按钮
3. 填写PR描述（使用提供的模板）
4. 创建Pull Request

### 4. 代码审查流程
- **提交者**: 创建PR后等待审查
- **审查者**: 在GitHub网页上进行代码审查
  - 查看代码变更
  - 添加评论和建议
  - 批准或请求修改
- **提交者**: 根据反馈修改代码并推送更新
- **审查者**: 再次审查并最终批准

### 5. 合并代码
审查通过后，在GitHub网页上：
1. 点击 "Merge pull request"
2. 选择合并方式（推荐 "Squash and merge"）
3. 删除功能分支

## 📋 提交信息规范

使用常规提交格式：
```
<类型>[可选作用域]: <描述>

[可选正文]

[可选脚注]
```

### 类型
- `feat`: 新功能
- `fix`: Bug修复
- `docs`: 文档更改
- `style`: 代码格式（不影响代码运行的变更）
- `refactor`: 重构（既不是新增功能，也不是修复bug的代码变更）
- `test`: 添加测试
- `chore`: 构建过程或辅助工具的变更

### 示例
```bash
git commit -m "feat(ui): 添加颜色拾取器功能"
git commit -m "fix(core): 修复窗口绑定时的内存泄漏"
git commit -m "docs: 更新构建说明文档"
```

## 🔒 分支保护规则

主分支 (`main`) 受到以下保护：
- 禁止直接推送（包括管理员）
- 必须通过Pull Request合并
- 需要至少1个审查者批准
- 必须通过CI检查
- 合并前需要分支是最新的

## 🎯 最佳实践

1. **小而频繁的提交**: 每个PR保持较小的变更范围
2. **清晰的描述**: 提供清晰的提交信息和PR描述
3. **及时响应**: 快速响应审查反馈
4. **持续集成**: 确保CI通过再请求审查
5. **测试覆盖**: 为新功能添加适当的测试

## 🚀 快速开始

```bash
# 1. 创建功能分支
git checkout main
git pull origin main
git checkout -b feature/new-feature

# 2. 开发和提交
# ... 编写代码 ...
git add .
git commit -m "feat: 添加新功能"

# 3. 推送并创建PR
git push origin feature/new-feature
# 在GitHub上创建PR

# 4. 等待审查和合并
```

现在所有代码变更，包括管理员的变更，都需要通过PR和代码审查流程！