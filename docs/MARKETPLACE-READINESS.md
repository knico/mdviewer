# Notepad++ Plugin List 提交状态

mdviewer 的 x86 插件列表提交位于 [PR #1284](https://github.com/notepad-plus-plus/nppPluginList/pull/1284)，状态为 Open（2026-11-24）。

该 PR 登记的是 v0.1.0 x86 包。源代码仓库中的后续功能和构建产物尚未发布到该列表。更新市场条目前，需要发布对应版本的 ZIP，重新计算 SHA-256，并同步维护插件清单中的版本、下载地址和散列值。

x64 条目应在真实 x64 Notepad++ 宿主验收完成后再提交。当前架构验证范围见 [VALIDATION.md](VALIDATION.md)。

## 发布前核对

- 发布版本号与 DLL 文件版本一致。
- ZIP 根目录直接包含 `mdviewer.dll`、`web` 资源和许可证文件。
- 下载 URL 可直接获取发布 ZIP。
- 清单中的 SHA-256 与实际发布文件相同。
- 作者、项目主页及最低兼容版本符合官方列表 schema。
- x64 发布项具有真实 Notepad++ x64 宿主验证记录。
