# mdviewer

mdviewer 是 Notepad++ 的离线 Markdown 预览插件。它将 Markdown 渲染核心与编辑器适配层分开，当前提供 Windows x86 和 x64 原生适配器。

## 功能

- 在 Notepad++ 停靠面板或独立浮动窗口中实时预览当前文档。
- 支持 CommonMark、表格、任务列表、删除线、脚注、代码高亮、KaTeX 数学公式和 Mermaid 图表。
- 使用随插件分发的前端资源，无需 CDN 或网络连接。
- 可将当前文档导出为 PDF 文件。
- 对原始 HTML、外部导航、下载、弹窗和浏览器权限采取限制策略。
- 预览文档大小上限为 4 MiB。

## 系统要求与兼容性

- Windows 10 或 Windows 11。
- Notepad++ x86 或 x64；插件架构必须与 Notepad++ 架构一致。
- Microsoft Edge WebView2 Runtime。
- x86 已在 Notepad++ 7.7 隔离环境中验证。x64 构建已完成，真实 x64 Notepad++ 宿主验收尚未完成；验证范围见 [docs/VALIDATION.md](docs/VALIDATION.md)。

## 安装

从 [GitHub Releases](https://github.com/knico/mdviewer/releases) 下载与 Notepad++ 架构匹配的 ZIP。解压到 Notepad++ 安装目录下的 `plugins/mdviewer`，确认 `mdviewer.dll` 与 `web` 目录位于同一目录，然后重启 Notepad++。在 **插件 → mdviewer → Preview** 打开预览。

各发行包包含的功能以对应版本的 Release Notes 为准。

完整安装与卸载步骤见 [docs/INSTALLATION.md](docs/INSTALLATION.md)。

### 编辑区右键菜单

Notepad++ 的编辑区上下文菜单由用户配置文件控制。仓库和发行包中的 `install-context-menu.ps1` 可以备份并更新该配置，将插件命令放入 **mdviewer** 子菜单。操作步骤见安装指南。

## 项目结构

- `packages/markdown-core`：与编辑器无关的 Markdown 渲染逻辑。
- `apps/preview-web`：预览界面及 WebView 消息协议。
- `adapters/notepad-plus-plus/native`：Notepad++、Scintilla 和 WebView2 原生适配层。
- `tests`：渲染、浏览器、原生模块和宿主集成检查。

## 从源码构建

构建环境需要 Windows、Visual Studio 2022 C++ 桌面开发工具、Node.js 22 或更高版本，以及 WebView2 SDK。

```powershell
npm ci
powershell -ExecutionPolicy Bypass -File scripts/bootstrap.ps1
npm test
npm run build:web
powershell -ExecutionPolicy Bypass -File scripts/build.ps1 -Architecture x64
powershell -ExecutionPolicy Bypass -File scripts/build.ps1 -Architecture x86 -SkipWeb
npm run test:browser
powershell -ExecutionPolicy Bypass -File scripts/package.ps1 -Architecture x64
powershell -ExecutionPolicy Bypass -File scripts/package.ps1 -Architecture x86
```

`bootstrap.ps1` 获取并校验 Notepad++ 接口头文件和 WebView2 SDK。可使用 `-WithNotepad` 下载便携式 Notepad++ 测试宿主。
发行步骤见 [docs/RELEASE.md](docs/RELEASE.md)。

## 扩展到其他编辑器

新增编辑器适配器放在 `adapters/<editor>`，并复用 Markdown 核心及预览消息协议。编辑器 API、文档读取和宿主窗口管理应留在各自适配器中。

## 开源许可

mdviewer 以 GPL-3.0-or-later 发布。第三方组件及其许可证见 [THIRD_PARTY.md](THIRD_PARTY.md) 和发行包中的许可证文件。
