# 验证状态

## 自动化检查

- Node.js 渲染测试：CommonMark/GFM、危险 HTML 与链接过滤、代码高亮、数学公式及 Mermaid。
- Chromium 浏览器检查：渲染结果、主题、窄屏布局、消息更新和不安全内容拦截。
- Windows 原生构建：x86 和 x64 MSVC 构建。
- 原生模块检查：DLL 加载、插件导出、Unicode 标识和架构信息。
- 安装包检查：DLL 架构、文件版本、ZIP 根目录结构和离线预览资源。

## 宿主验证

| 宿主 | 架构 | 结果 |
| --- | --- | --- |
| Notepad++ 7.7 | x86 | 在隔离副本中验证插件加载、中文文档、数学公式、Mermaid、编辑后刷新及正常退出。 |
| x64 原生构建与 DLL 检查 | x64 | 构建和原生 DLL 检查已完成；真实 Notepad++ x64 宿主集成验证尚未完成。 |

x86 结果不代表所有 Notepad++ 版本均已验证。x64 使用 Notepad++ 8.9.9.1 接口头文件构建；DLL 检查不代表真实宿主集成通过。

## 当前限制

- x64 宿主中的停靠、浮动、文档切换和关闭流程仍需实际验证。
- 本地图片、相对文件链接、源文档同步滚动、HTML 导出和 ARM64 尚未实现。
- PDF 导出依赖已实现 `PrintToPdf` 接口的 WebView2 Runtime。

## 复现检查

```powershell
npm ci
npm test
npm run build:web
powershell -ExecutionPolicy Bypass -File scripts/build.ps1 -Architecture x64 -SkipWeb
powershell -ExecutionPolicy Bypass -File scripts/build.ps1 -Architecture x86 -SkipWeb
powershell -ExecutionPolicy Bypass -File tests/native-smoke.ps1
npm run test:browser
powershell -ExecutionPolicy Bypass -File tests/host-smoke.ps1
```

宿主检查需要先运行 `scripts/bootstrap.ps1 -WithNotepad`。自动化检查结果不替代各架构的真实用户环境验证。
