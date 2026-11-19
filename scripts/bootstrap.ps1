param([switch]$WithNotepad)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
function Fetch([string]$url, [string]$path) {
    New-Item -ItemType Directory -Force (Split-Path $path -Parent) | Out-Null
    & curl.exe -sS -L --fail --retry 2 --max-time 120 $url -o $path
    if ($LASTEXITCODE -ne 0) { throw "Download failed: $url" }
}
$tag = 'v8.9.9.1'
$base = "https://raw.githubusercontent.com/notepad-plus-plus/notepad-plus-plus/$tag"
$headers = @{
    'PluginInterface.h' = 'PowerEditor/src/MISC/PluginsManager/PluginInterface.h'
    'Notepad_plus_msgs.h' = 'PowerEditor/src/MISC/PluginsManager/Notepad_plus_msgs.h'
    'menuCmdID.h' = 'PowerEditor/src/menuCmdID.h'
    'Scintilla.h' = 'scintilla/include/Scintilla.h'
    'Sci_Position.h' = 'scintilla/include/Sci_Position.h'
    'Docking.h' = 'PowerEditor/src/WinControls/DockingWnd/Docking.h'
}
foreach ($name in $headers.Keys) {
    $path = Join-Path $root ".deps/npp/$name"
    if (!(Test-Path $path)) { Fetch "$base/$($headers[$name])" $path }
}
$sdk = '1.0.3650.58'
$sdkDir = Join-Path $root '.deps/webview2'
if (!(Test-Path "$sdkDir/build/native/include/WebView2.h")) {
    $zip = Join-Path $root '.deps/webview2.zip'
    Fetch "https://api.nuget.org/v3-flatcontainer/microsoft.web.webview2/$sdk/microsoft.web.webview2.$sdk.nupkg" $zip
    Expand-Archive -LiteralPath $zip -DestinationPath $sdkDir -Force
}
if ($WithNotepad -and !(Test-Path (Join-Path $root '.deps/notepad/notepad++.exe'))) {
    $zip = Join-Path $root '.deps/notepad.zip'
    Fetch "https://github.com/notepad-plus-plus/notepad-plus-plus/releases/download/$tag/npp.8.9.9.1.portable.x64.zip" $zip
    $hash = (Get-FileHash $zip -Algorithm SHA256).Hash.ToLowerInvariant()
    if ($hash -ne 'cbcefa34677e10fe6dd19802f7d6b3b9860cd6de62afeaae7680e09c51b47779') { throw 'Notepad++ archive SHA256 mismatch' }
    Expand-Archive -LiteralPath $zip -DestinationPath (Join-Path $root '.deps/notepad') -Force
}
Write-Output 'Dependencies ready: Notepad++ v8.9.9.1 API; WebView2 SDK 1.0.3650.58'
