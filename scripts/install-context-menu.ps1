param([Parameter(Mandatory=$true)][string]$ConfigDirectory)
$ErrorActionPreference = 'Stop'
# Run while Notepad++ is closed. This only merges our own plugin commands.
$path = Join-Path $ConfigDirectory 'contextMenu.xml'
if (!(Test-Path -LiteralPath $path)) { throw 'contextMenu.xml not found. Open Settings > Edit Popup ContextMenu, save it and close Notepad++ first.' }
$document = New-Object System.Xml.XmlDocument
$document.PreserveWhitespace = $true
$document.Load($path)
$menu = $document.SelectSingleNode('/NotepadPlus/ScintillaContextMenu')
if (!$menu) { throw 'Not a supported Notepad++ context menu configuration' }
$names = @('Preview','Floating preview','Refresh','Export PDF...')
$missing = @($names | Where-Object { !$menu.SelectSingleNode("Item[@PluginEntryName='mdviewer' and @PluginCommandItemName='$_']") })
$rename = @($menu.SelectNodes("Item[@PluginEntryName='mdviewer']") | Where-Object { $_.GetAttribute('FolderName') -ne 'mdviewer' })
if (!$missing.Count -and !$rename.Count) { Write-Output 'mdviewer context commands already installed'; exit 0 }
$backup = $path + '.mdviewer-' + (Get-Date -Format 'yyyyMMdd-HHmmss-fff') + '.bak'
Copy-Item -LiteralPath $path -Destination $backup -ErrorAction Stop
foreach ($item in $rename) { $item.SetAttribute('FolderName','mdviewer') }
foreach ($name in $missing) {
    $item = $document.CreateElement('Item')
    $item.SetAttribute('FolderName','mdviewer')
    $item.SetAttribute('PluginEntryName','mdviewer')
    $item.SetAttribute('PluginCommandItemName',$name)
    $menu.AppendChild($item) | Out-Null
}
$settings = New-Object System.Xml.XmlWriterSettings
$settings.Encoding = New-Object System.Text.UTF8Encoding($false)
$writer = [System.Xml.XmlWriter]::Create($path, $settings)
try { $document.Save($writer) } finally { $writer.Dispose() }
Write-Output "Installed four commands under mdviewer. Backup: $backup. Restart Notepad++."
