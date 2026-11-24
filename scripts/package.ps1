param([ValidateSet('x86','x64')][string]$Architecture='x86')
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$manifest=Get-Content -LiteralPath (Join-Path $root 'package.json') -Raw | ConvertFrom-Json
if ($manifest.version -notmatch '^\d+\.\d+\.\d+$') { throw "Unsupported package version '$($manifest.version)'" }
$packageVersion=$manifest.version
$dllVersion="$packageVersion.0"
$source=if($Architecture -eq 'x86'){'dist/x86/mdviewer'}else{'dist/mdviewer'}
$dll=Join-Path $root "$source/mdviewer.dll"
if(!(Test-Path $dll)){throw "Build $Architecture first"}
$bin=[IO.File]::ReadAllBytes($dll)
$machine=[BitConverter]::ToUInt16($bin,([BitConverter]::ToInt32($bin,60)+4))
$expected=if($Architecture -eq 'x86'){0x14c}else{0x8664}
if($machine -ne $expected){throw 'DLL architecture mismatch'}
if($bin.Length -lt 1){throw 'Empty DLL'}
$fileVersion=(Get-Item $dll).VersionInfo.FileVersion
if($fileVersion -ne $dllVersion){throw "DLL FileVersion '$fileVersion' does not match package version '$dllVersion'"}
$requiredFiles=@(
 'web/index.html'
 'LICENSE'
 'THIRD_PARTY.md'
 'INSTALLATION.md'
 'WEBVIEW2-LICENSE.txt'
 'DEPENDENCY-LICENSES.txt'
)
foreach($file in $requiredFiles){
 if(!(Test-Path (Join-Path $root "$source/$file"))){throw "Required package file missing: $file"}
}
$release=Join-Path $root 'dist/releases'
New-Item -ItemType Directory -Force $release | Out-Null
$zip=Join-Path $release "mdviewer-$fileVersion-$Architecture.zip"
if(Test-Path $zip){Remove-Item $zip -Force}
Compress-Archive -Path (Join-Path $root "$source/*") -DestinationPath $zip -CompressionLevel Optimal
$archive=[IO.Compression.ZipFile]::OpenRead($zip)
try {
 $names=@($archive.Entries | ForEach-Object {$_.FullName.Replace('\','/')})
 if($names -notcontains 'mdviewer.dll'){throw 'Plugin DLL must be at ZIP root'}
 foreach($file in $requiredFiles){if($names -notcontains $file){throw "Required package file missing from ZIP: $file"}}
 if($names -contains 'mdviewer/mdviewer.dll'){throw 'Unexpected nested plugin folder'}
} finally {$archive.Dispose()}
$sha=(Get-FileHash $zip -Algorithm SHA256).Hash.ToLowerInvariant()
[pscustomobject]@{Architecture=$Architecture;Version=$fileVersion;Zip=$zip;SHA256=$sha;Size=(Get-Item $zip).Length;Entries=$names.Count} | Format-List
