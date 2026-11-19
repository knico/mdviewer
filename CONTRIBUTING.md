# 贡献指南

欢迎提交问题、文档改进和代码贡献。请先说明复现环境、编辑器版本、架构和 WebView2 Runtime 版本；涉及渲染行为的改动应同时补充渲染测试或浏览器验证。

## 开发流程

```powershell
npm ci
npm test
npm run build:web
powershell -ExecutionPolicy Bypass -File scripts/build.ps1 -Architecture x86 -SkipWeb
```

跨编辑器功能应优先放入 `packages/markdown-core` 或 `apps/preview-web`；Notepad++ 专属代码放入 `adapters/notepad-plus-plus`。不要把编辑器 API、Windows 窗口句柄或 Scintilla 调用带入共享核心。

## 提交要求

- 不提交 `node_modules`、`.deps`、`build`、`dist` 和本地截图。
- 不在渲染内容中执行动态脚本；保持现有 HTML 清理和链接安全边界。
- 更新用户可见行为时同步更新 README、验证记录和许可证说明。
- Pull Request 应说明已运行的检查以及尚未完成的宿主验收项。
