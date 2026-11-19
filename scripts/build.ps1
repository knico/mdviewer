param([switch]$SkipWeb, [ValidateSet('x64','x86')][string]$Architecture = 'x64')
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
Push-Location $root
try {
    if (!$SkipWeb) {
        & npm.cmd run build:web
        if ($LASTEXITCODE -ne 0) { throw 'Web build failed' }
    }
    if (!(Test-Path '.deps/webview2/build/native/include/WebView2.h')) { throw 'Run scripts/bootstrap.ps1 first' }
    $vswhere = "${env:ProgramFiles(x86)}/Microsoft Visual Studio/Installer/vswhere.exe"
    $vs = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    if (!$vs) { throw 'Visual Studio C++ build tools not found' }
    $output = if ($Architecture -eq 'x86') { 'dist/x86/mdviewer' } else { 'dist/mdviewer' }
    $intermediate = "build/$Architecture"
    New-Item -ItemType Directory -Force $intermediate,$output | Out-Null
    if ($Architecture -eq 'x86') {
        if (!(Test-Path dist/mdviewer/web/index.html)) { throw 'Build web resources first' }
        Copy-Item dist/mdviewer/web $output -Recurse -Force
        Copy-Item dist/mdviewer/DEPENDENCY-LICENSES.txt $output -Force
    }
    $build = Join-Path $root "$intermediate/native.cmd"
    $environment = if ($Architecture -eq 'x86') { 'vcvars32.bat' } else { 'vcvars64.bat' }
    $lines = @(
        '@echo off'
        ('call "' + $vs + '\VC\Auxiliary\Build\' + $environment + '" >nul')
        'if errorlevel 1 exit /b %errorlevel%'
        ('rc /nologo /fo ' + $intermediate + '/version.res adapters/notepad-plus-plus/native/version.rc')
        ('cl /nologo /utf-8 /std:c++17 /EHsc /W4 /O2 /MT /DUNICODE /D_UNICODE /DWIN32_LEAN_AND_MEAN /DNOMINMAX /LD /I .deps/npp /I .deps/webview2/build/native/include adapters/notepad-plus-plus/native/Plugin.cpp /Fo:' + $intermediate + '/Plugin.obj /link ' + $intermediate + '/version.res /OUT:' + $output + '/mdviewer.dll /IMPLIB:' + $intermediate + '/mdviewer.lib /DEF:adapters/notepad-plus-plus/native/mdviewer.def .deps/webview2/build/native/' + $Architecture + '/WebView2LoaderStatic.lib user32.lib ole32.lib shlwapi.lib shell32.lib version.lib advapi32.lib')
    )
    [IO.File]::WriteAllLines($build, $lines, [Text.Encoding]::ASCII)
    & cmd.exe /d /c $build
    if ($LASTEXITCODE -ne 0) { throw 'Native build failed' }
    Copy-Item README.md,THIRD_PARTY.md,LICENSE $output -Force
    Copy-Item .deps/webview2/LICENSE.txt "$output/WEBVIEW2-LICENSE.txt" -Force
    Copy-Item docs/VALIDATION.md "$output/VALIDATION.md" -Force
    Write-Output "Built $output/mdviewer.dll (Windows $Architecture)"
} finally { Pop-Location }
