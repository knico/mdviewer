param([string]$PluginPath = 'dist/mdviewer/mdviewer.dll')
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class DllSmoke {
 [DllImport("kernel32", CharSet=CharSet.Unicode, SetLastError=true)] public static extern IntPtr LoadLibrary(string path);
 [DllImport("kernel32", CharSet=CharSet.Ansi)] public static extern IntPtr GetProcAddress(IntPtr dll,string name);
 [DllImport("kernel32")] public static extern bool FreeLibrary(IntPtr dll);
 [UnmanagedFunctionPointer(CallingConvention.Cdecl)] public delegate IntPtr Name();
 [UnmanagedFunctionPointer(CallingConvention.Cdecl)] public delegate int Unicode();
 [UnmanagedFunctionPointer(CallingConvention.Cdecl)] public delegate IntPtr Functions(out int count);
}
'@
$dll = [DllSmoke]::LoadLibrary((Join-Path $root $PluginPath))
if ($dll -eq [IntPtr]::Zero) { throw "LoadLibrary failed: $([Runtime.InteropServices.Marshal]::GetLastWin32Error())" }
try {
    foreach ($name in @('setInfo','getName','getFuncsArray','beNotified','messageProc','isUnicode')) {
        if ([DllSmoke]::GetProcAddress($dll,$name) -eq [IntPtr]::Zero) { throw "Missing export $name" }
    }
    $getName = [Runtime.InteropServices.Marshal]::GetDelegateForFunctionPointer([DllSmoke]::GetProcAddress($dll,'getName'),[type][DllSmoke+Name])
    if ([Runtime.InteropServices.Marshal]::PtrToStringUni($getName.Invoke()) -ne 'mdviewer') { throw 'Invalid plugin name' }
    $unicode = [Runtime.InteropServices.Marshal]::GetDelegateForFunctionPointer([DllSmoke]::GetProcAddress($dll,'isUnicode'),[type][DllSmoke+Unicode])
    if ($unicode.Invoke() -ne 1) { throw 'Unicode export invalid' }
    $functions = [Runtime.InteropServices.Marshal]::GetDelegateForFunctionPointer([DllSmoke]::GetProcAddress($dll,'getFuncsArray'),[type][DllSmoke+Functions])
    $count = 0
    $items = $functions.Invoke([ref]$count)
    if ($items -eq [IntPtr]::Zero -or $count -ne 3) { throw 'Invalid command exports' }
    Write-Output 'Native DLL smoke passed: load, six exports, Unicode, plugin name and three commands.'
} finally { [DllSmoke]::FreeLibrary($dll) | Out-Null }
