# Notepad++ Plugin Admin 发布准备

## 当前结论

插件已达到生成候选发布包的阶段，尚未达到可提交 Plugin Admin 的阶段。仓库列表要求每个架构在对应 JSON 清单中登记，`id` 必须是发布 ZIP 的 SHA256，`version` 必须与 DLL 版本资源一致，`repository` 必须是可直接下载的 ZIP 地址；列表改动通过 `nppPluginList` 的 Pull Request 提交。

## 本地产物

| 架构 | ZIP | SHA256 | DLL 版本 | 验证 |
| --- | --- | --- | --- | --- |
| x86 | `dist/releases/mdviewer-0.1.0.0-x86.zip` | `becd6c9f5115d015e70970367754c9d03688b347649df5f59ca1a1dbe51fe3db` | 0.1.0.0 | 根目录含 `mdviewer.dll` 和 `web/index.html`；Notepad++ 7.7 x86 隔离副本验证通过 |
| x64 | `dist/releases/mdviewer-0.1.0.0-x64.zip` | `dbef2a5f2db440d304dcadd27705df6e5082ba534140f93a882761c8f973e735` | 0.1.0.0 | 包结构和 PE 架构验证通过；真实 x64 Notepad++ 宿主验收仍未完成 |

两份 ZIP 都包含离线渲染资源、第三方许可证和说明文件。WebView2 Runtime 是运行前提，包中不包含该运行时。

重建后运行 `scripts/package.ps1 -Architecture x86` 或 `x64`。脚本会核对 PE 架构、DLL FileVersion、离线资源、ZIP 根目录布局并输出 SHA256。任何 DLL 或 ZIP 变化都要求重新计算清单 `id`。

## 提交前仍需补齐

1. 确认公开显示的作者名称。
2. 提供公开源码/项目主页及可直接下载的 Release ZIP URL。当前工作目录没有 Git 元数据，也没有已配置的公开发布仓库，因此不能生成真实 marketplace `repository` 地址。
3. 建立并运行 `nppPluginList` 的本地列表校验，再仅修改对应架构的 `src/pl.x86.json`（以及 x64 验收完成后可选的 `src/pl.x64.json`），提交 Pull Request。
4. 完成真实 x64 Notepad++ 宿主验收后，再发布并登记 x64 包；当前证据不足以宣称其已通过宿主测试。

插件列表已有若干 Markdown 预览插件。提交描述应突出 mdviewer 的离线资源、KaTeX、Mermaid、任务列表/脚注及经验证的旧版 x86 兼容性，不要声称“支持最新 Markdown”而不列明具体语法范围。

## 清单字段草案（占位，不可直接提交）

```json
{
  "folder-name": "mdviewer",
  "display-name": "mdviewer",
  "version": "0.1.0.0",
  "id": "<对应架构发布 ZIP 的 SHA256>",
  "repository": "<直接下载 ZIP 的公开 URL>",
  "description": "Offline WebView2 Markdown preview with tables, task lists, footnotes, syntax highlighting, KaTeX and Mermaid. Requires Microsoft Edge WebView2 Runtime.",
  "author": "<作者名称>",
  "homepage": "<公开项目主页>",
  "npp-compatible-versions": "[7.7,]"
}
```

此草案按当前列表字段约定整理；最终字段、兼容性范围及下载地址应在提交时再次对照官方 schema 和审核说明。
