# mdviewer

mdviewer 是一个可扩展的 Markdown 实时预览项目。目前提供 Notepad++ 的 Windows x64 / x86 适配器；渲染核心和 Web 预览应用已与宿主适配层分离，未来可以复用到其他编辑器。

## 工程结构

- `packages/markdown-core`：Markdown 渲染核心，编辑器无关。
- `apps/preview-web`：WebView 中运行的预览应用和页面资源。
- `adapters/notepad-plus-plus/native`：Notepad++ 原生插件适配器（C++、WebView2、停靠面板和 Scintilla 文档同步）。
- `tests`：渲染、浏览器和 Windows 原生宿主测试。

2026-11-18 补充：32 位版位于 dist/x86/mdviewer；使用 scripts/build.ps1 -Architecture x86 构建。已在 Notepad++ 7.7 x86 的隔离副本中验证插件菜单、中文、公式、Mermaid、实时编辑与正常退出。下文早期的“32 位未实现”说明已被此记录取代。

## 功能

- 右侧可停靠预览面板；编辑防抖刷新、切换文档和保存时刷新。
- CommonMark、表格、删除线、任务列表、脚注、代码高亮、KaTeX 公式、Mermaid 图。
- 本地打包渲染资源；明暗主题切换；不依赖 CDN。
- 默认禁用原始 HTML、图片、下载、弹窗和浏览器权限；文档上限 4 MiB。

“最新 Markdown”没有统一扩展标准；本项目以上述功能清单为准。当前未实现本地图片、相对文件跳转、同步滚动、HTML 导出和 ARM64 构建。

## 构建

需要 Windows 10/11、Visual Studio 2022 C++ 桌面开发工具、Node.js 22+ 和 Microsoft Edge WebView2 Runtime。

```powershell
npm ci --registry=https://registry.npmjs.org
powershell -ExecutionPolicy Bypass -File scripts/bootstrap.ps1
npm test
powershell -ExecutionPolicy Bypass -File scripts/build.ps1
npm run test:browser
powershell -ExecutionPolicy Bypass -File scripts/package.ps1 -Architecture x86
```

初次开发安装依赖使用 package.json/package-lock.json 固定版本。bootstrap 固定 Notepad++ v8.9.9.1 接口、WebView2 SDK 1.0.3650.58；可加 `-WithNotepad` 下载校验过 SHA256 的便携测试宿主。

## 扩展到其他编辑器

新增编辑器适配器时，复用 `packages/markdown-core` 的渲染接口和 `apps/preview-web` 的消息协议，另建对应的 `adapters/<editor>` 目录。适配器只负责读取当前文档、发送 Markdown 和接收预览状态；不要复制渲染逻辑。不同编辑器的安装包、运行时要求和测试应分别记录。

## 安装

关闭 Notepad++，将对应架构目录的内容复制到其 `plugins/mdviewer`：x86 使用 `dist/x86/mdviewer`，x64 使用 `dist/mdviewer`。保持 `plugins/mdviewer/mdviewer.dll` 与旁边的 `web` 和许可证文件。重启后选择 **插件 → mdviewer → Preview**。不要只复制 DLL。可分发 ZIP 通过 `scripts/package.ps1 -Architecture x86|x64` 生成到 `dist/releases`。

WebView2 用户数据放在 `%LOCALAPPDATA%/mdviewer/WebView2`。插件缺失资源或运行时时在面板提示错误。预览链接不会自动打开程序，可复制链接后自行打开。

## 验证记录

截至 2026-11-18：工程和源码已建立；实际构建与测试状态见 docs/VALIDATION.md。尚未通过的验收项不能视为已交付可用版本。

Notepad++ Plugin Admin 上架准备状态见 [docs/MARKETPLACE-READINESS.md](docs/MARKETPLACE-READINESS.md)。发布前需要公开 ZIP 下载地址和作者/项目主页信息。
