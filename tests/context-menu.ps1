$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
$fixture=Join-Path $root 'build/context-menu-test'
New-Item -ItemType Directory -Force $fixture | Out-Null
[IO.File]::WriteAllText((Join-Path $fixture 'contextMenu.xml'),'<NotepadPlus><ScintillaContextMenu><Item MenuEntryName="Edit" MenuItemName="Copy" /></ScintillaContextMenu></NotepadPlus>')
& (Join-Path $root 'scripts/install-context-menu.ps1') -ConfigDirectory $fixture
& (Join-Path $root 'scripts/install-context-menu.ps1') -ConfigDirectory $fixture
[xml]$xml=Get-Content (Join-Path $fixture 'contextMenu.xml') -Raw
if(@($xml.SelectNodes('//Item[@PluginEntryName="mdviewer"]')).Count -ne 4){throw 'Expected four unique commands'}
if(!$xml.SelectSingleNode('//Item[@MenuItemName="Copy"]')){throw 'Existing user command lost'}
if(@(Get-ChildItem $fixture -Filter '*.bak').Count -lt 1){throw 'Configuration backup missing'}
Write-Output 'Context menu merge passed: four commands, preserved user entry, backup and idempotence.'
