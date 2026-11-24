# 安装与卸载

## 安装

1. 在 Notepad++ 中打开 **? → Debug Info**，确认应用架构。
2. 下载与 Notepad++ 架构相同的 mdviewer ZIP，并解压。
3. 将解压内容复制到 Notepad++ 安装目录下的 `plugins/mdviewer`。安装后目录中应包含 `mdviewer.dll`、`web` 目录和许可证文件。
4. 确认已安装 Microsoft Edge WebView2 Runtime。
5. 重启 Notepad++，从 **插件 → mdviewer → Preview** 打开预览。

插件 DLL 和 `web` 资源目录必须保持并列。缺少资源时，插件无法加载预览页面。

## 使用命令

- **Preview**：打开停靠预览面板。
- **Refresh**：重新读取当前文档并刷新预览。
- **Floating preview**：打开可移动和缩放的独立预览窗口。该窗口可以与停靠面板同时显示。
- **Export PDF...**：选择目标路径并直接生成 PDF 文件。
- **About**：显示插件版本和支持的 Markdown 功能。

浮动窗口会保留到关闭 Notepad++ 为止。关闭窗口会隐藏它；再次执行 **Floating preview** 可重新显示。

## 添加到编辑区右键菜单

1. 在 Notepad++ 中打开 **设置 → 编辑右键菜单**，确认当前使用的配置目录。
2. 关闭 Notepad++。
3. 在 PowerShell 中运行发行包内的安装脚本，并将配置目录替换为实际路径：

   ```powershell
   powershell -ExecutionPolicy Bypass -File .\install-context-menu.ps1 -ConfigDirectory 'C:\path\to\Notepad++\配置目录'
   ```

脚本会备份 `contextMenu.xml`，添加 mdviewer 命令并保留其他菜单项。重复运行会更新 mdviewer 菜单项，不会重复追加。重启 Notepad++ 后生效。若需恢复，关闭 Notepad++ 并将脚本生成的备份文件还原为 `contextMenu.xml`。

## 用户数据

WebView2 用户数据存储在 `%LOCALAPPDATA%\mdviewer\WebView2`。卸载插件不会自动删除此目录。

## 卸载

1. 关闭所有 Notepad++ 窗口。
2. 删除 Notepad++ 安装目录下的 `plugins/mdviewer` 文件夹。
3. 如需清除 WebView2 用户数据，删除 `%LOCALAPPDATA%\mdviewer\WebView2`。
4. 如已修改编辑区右键菜单，可用安装脚本生成的备份恢复原配置。
