param([ValidateSet('x64','x86')][string]$Architecture='x64', [string]$HostSource='')
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$source = Join-Path $root '.deps/notepad'
if ($HostSource) { $source = $HostSource }
elseif (!(Test-Path (Join-Path $source 'notepad++.exe'))) { $source = 'D:/tools/Notepad++' }
$exe = Join-Path $source 'notepad++.exe'
$bytes = [IO.File]::ReadAllBytes($exe)
$pe = [BitConverter]::ToInt32($bytes,60)
$expected = if($Architecture -eq 'x86'){0x14c}else{0x8664}
if ([BitConverter]::ToUInt16($bytes,$pe+4) -ne $expected) { throw 'Host architecture does not match plugin' }
$target = Join-Path $root ".deps/host-smoke-$Architecture"
New-Item -ItemType Directory -Force $target | Out-Null
Copy-Item $exe $target -Force
if (Test-Path (Join-Path $source 'SciLexer.dll')) { Copy-Item (Join-Path $source 'SciLexer.dll') $target -Force }
foreach($name in @('langs.xml','langs.model.xml','stylers.xml','stylers.model.xml','contextMenu.xml','functionList.xml','shortcuts.xml')) {
 if(Test-Path (Join-Path $source $name)){Copy-Item (Join-Path $source $name) $target -Force}
}
[IO.File]::WriteAllText((Join-Path $target 'doLocalConf.xml'),'')
New-Item -ItemType Directory -Force (Join-Path $target 'plugins') | Out-Null
$package = if($Architecture -eq 'x86'){'dist/x86/mdviewer'}else{'dist/mdviewer'}
Copy-Item (Join-Path $root $package) (Join-Path $target 'plugins') -Recurse -Force
Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public static class HostUI {
 [DllImport("user32")] public static extern IntPtr GetMenu(IntPtr h);
 [DllImport("user32")] public static extern int GetMenuItemCount(IntPtr h);
 [DllImport("user32")] public static extern IntPtr GetSubMenu(IntPtr h,int p);
 [DllImport("user32")] public static extern uint GetMenuItemID(IntPtr h,int p);
 [DllImport("user32",CharSet=CharSet.Unicode)] public static extern int GetMenuString(IntPtr h,uint i,StringBuilder s,int n,uint f);
 [DllImport("user32")] public static extern IntPtr SendMessage(IntPtr h,uint m,IntPtr w,IntPtr l);
 [DllImport("user32",CharSet=CharSet.Unicode)] public static extern IntPtr FindWindowEx(IntPtr parent,IntPtr after,string cls,string title);
 public delegate bool EnumProc(IntPtr h,IntPtr data);
 [DllImport("user32")] public static extern bool EnumChildWindows(IntPtr parent,EnumProc callback,IntPtr data);
 [DllImport("user32",CharSet=CharSet.Unicode)] public static extern int GetClassName(IntPtr h,StringBuilder name,int size);
 [DllImport("user32")] public static extern bool IsWindowVisible(IntPtr h);
 public static IntPtr Editor(IntPtr parent) { IntPtr found=IntPtr.Zero; EnumChildWindows(parent,(h,d)=>{var s=new StringBuilder(256);GetClassName(h,s,256);if(s.ToString()=="Scintilla" && IsWindowVisible(h)){found=h;return false;}return true;},IntPtr.Zero);return found; }
 [DllImport("user32",CharSet=CharSet.Ansi,EntryPoint="SendMessageA")] public static extern IntPtr SendText(IntPtr h,uint m,IntPtr w,string text);
 public static void SetText(IntPtr h,string text) { SendMessage(h,2013,IntPtr.Zero,IntPtr.Zero); foreach(char c in text){SendMessage(h,258,(IntPtr)c,IntPtr.Zero);} }
}
'@
function Find-Preview([IntPtr]$menu,[bool]$inside=$false) {
 for($i=0;$i -lt [HostUI]::GetMenuItemCount($menu);$i++) {
  $s = New-Object Text.StringBuilder 256
  [HostUI]::GetMenuString($menu,$i,$s,256,0x400) | Out-Null
  $label=$s.ToString().Replace('&','')
  if($inside -and $label -eq 'Preview') { return [HostUI]::GetMenuItemID($menu,$i) }
  $sub = [HostUI]::GetSubMenu($menu,$i)
  if($sub -ne [IntPtr]::Zero) { $found=Find-Preview $sub ($inside -or $label -eq 'mdviewer'); if($found){return $found} }
 }
 return 0
}
$env:WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS='--remote-debugging-port=19247'
$sample=Join-Path $root 'examples/showcase.md'
$p=Start-Process -FilePath (Join-Path $target 'notepad++.exe') -ArgumentList ('-multiInst -nosession "' + $sample + '"') -PassThru -WindowStyle Hidden
try {
 $deadline=[DateTime]::UtcNow.AddSeconds(20)
 do { Start-Sleep -Milliseconds 200; $p.Refresh() } while(!$p.HasExited -and $p.MainWindowHandle -eq [IntPtr]::Zero -and [DateTime]::UtcNow -lt $deadline)
 if($p.HasExited -or $p.MainWindowHandle -eq [IntPtr]::Zero){throw 'Host did not start'}
 do { $p.Refresh(); $cmd=Find-Preview ([HostUI]::GetMenu($p.MainWindowHandle)); if(!$cmd){Start-Sleep -Milliseconds 300} } while(!$cmd -and [DateTime]::UtcNow -lt $deadline)
 if(!$cmd){throw 'mdviewer menu missing'}
 [HostUI]::SendMessage($p.MainWindowHandle,0x111,[IntPtr]$cmd,[IntPtr]::Zero) | Out-Null
 & node.exe tests/native-webview.mjs initial
 if($LASTEXITCODE -ne 0){throw 'Embedded WebView2 check failed'}
 $editor=[HostUI]::Editor($p.MainWindowHandle)
 if($editor -eq [IntPtr]::Zero){throw 'Editor missing'}
 [HostUI]::SetText($editor,"# Live edit verified" + [Environment]::NewLine + [Environment]::NewLine + "Native Scintilla update.")
 Start-Sleep -Milliseconds 500
 $p.Refresh()
 Write-Output "Host after edit: exited=$($p.HasExited)"
 if($p.HasExited){throw "Host crashed after edit: $($p.ExitCode)"}
 & node.exe tests/native-webview.mjs edited
 if($LASTEXITCODE -ne 0){throw 'Live edit preview failed'}
 [HostUI]::SendMessage($editor,2014,[IntPtr]::Zero,[IntPtr]::Zero) | Out-Null
 [HostUI]::SendMessage($p.MainWindowHandle,0x10,[IntPtr]::Zero,[IntPtr]::Zero) | Out-Null
 if(!$p.WaitForExit(10000)){throw 'Host did not exit'}
 if($p.ExitCode -ne 0){throw 'Host exited with error'}
 Write-Output 'Isolated Notepad++ passed: plugin menu, embedded Markdown/Mermaid/math, live edit and clean shutdown.'
} finally { if(!$p.HasExited){Stop-Process -Id $p.Id -Force}; Remove-Item Env:WEBVIEW2_ADDITIONAL_BROWSER_ARGUMENTS -ErrorAction SilentlyContinue }
