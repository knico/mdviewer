# Validation — 2026-11-18

## Development update — 2026-11-23

- Added independent floating preview and direct PDF export, preserving Preview/Refresh/About command indices.
- Isolated Notepad++ 7.7 x86 passed docked rendering, live editing, two simultaneous preview WebViews, floating show/close, native Save PDF dialog, valid PDF file creation and clean shutdown.
- Initial export automation failures came from IntPtr zero checks, localized dialog titles and nested filename controls; the corrected harness passed. A separate test WebView profile avoids existing-user profile collisions.
- Context menu setup is opt-in: four commands under Plugin commands with configuration backup and duplicate prevention.
- Published v0.1.0 ZIP files and marketplace hashes remain unchanged. New features are development candidates; x64 runtime acceptance remains open.

Marketplace packaging was rechecked on 2026-11-19: x86 and x64 packages have DLL FileVersion 0.1.0.0, correct PE machine type, and the plugin DLL plus offline web assets at the ZIP root. Their hashes and remaining listing gates are recorded in `docs/MARKETPLACE-READINESS.md`. This package check does not close real x64 host acceptance.

## x86 compatibility update (supersedes earlier x86 limitations)

- Built dist/x86/mdviewer/mdviewer.dll with x86 MSVC and x86 WebView2 static loader; 32-bit PowerShell LoadLibrary and all six exports passed.
- Notepad++ 7.7 x86 isolated copy passed actual plugin menu loading, embedded Chinese Markdown, KaTeX, Mermaid labels, real WM_CHAR editing notifications, automatic preview refresh and clean host exit (code 0).
- Fixed native JSON control-character escaping discovered by embedded-host testing. Both architecture builds use the corrected source.
- Fixed test-host startup by copying required langs/stylers resources; missing langs.xml was a test setup defect.
- Initial test code used a process-local Scintilla text pointer across processes; corrected to standard character messages. This test failure did not establish a plugin defect.
- x64 real-host acceptance remains open. Tab switching and docking restoration require separate manual validation.

## Passed

- Windows x64 release DLL compiled with Visual Studio 2022 MSVC 14.44, Windows SDK 10.0.26100.0, WebView2 SDK 1.0.3650.58 and Notepad++ v8.9.9.1 interface headers.
- Native LoadLibrary smoke: DLL loads, all six plugin exports resolve, name is mdviewer, Unicode enabled, three menu commands returned.
- Node rendering tests: 3/3 passed (CommonMark/GFM, HTML/script-link rejection, code highlighting/math/Mermaid source).
- Chromium browser checks passed: rendered Markdown, tables, math, Mermaid, code highlighting, light/dark theme, 360px layout, message updates and rejection of document scripts/unsafe links/remote image sources.
- Visual review of preview-light.png identified missing Mermaid labels; top-level htmlLabels=false fixed the issue; a label-content assertion and browser rerun passed.
- npm installation audit reported 0 vulnerabilities at dependency resolution time. This is not a complete security audit.

## Open acceptance gates

- Real Notepad++ x64 loading, docking/floating, live Scintilla edits, tab switches, close/reopen and clean shutdown are **not yet verified**.
- The existing D:/tools/Notepad++ host is version 7.7, PE machine 0x14C (x86). An isolated copy rejected the x64 plugin (no plugin menu); that result does not demonstrate an x64 plugin defect or successful host integration.
- Download of the official Notepad++ 8.9.9.1 x64 portable archive failed because release-assets.githubusercontent.com could not be resolved. bootstrap.ps1 retains the official archive hash and is ready to retry.
- Local and remote images, relative file links, synchronous source scrolling, export, x86 and ARM64 are outside v0.1 scope.

## Commands

1. npm ci --registry=https://registry.npmjs.org
2. powershell -ExecutionPolicy Bypass -File scripts/bootstrap.ps1
3. npm test
4. powershell -ExecutionPolicy Bypass -File scripts/build.ps1
5. powershell -ExecutionPolicy Bypass -File tests/native-smoke.ps1
6. npm run test:browser
7. powershell -ExecutionPolicy Bypass -File scripts/bootstrap.ps1 -WithNotepad
8. powershell -ExecutionPolicy Bypass -File tests/host-smoke.ps1

Steps 7–8 remain open. The host smoke currently checks initial embedded preview and clean shutdown; live edits, switching tabs and docking restoration also require manual host validation.

## Artifacts

- dist/mdviewer/mdviewer.dll and adjacent web/ folder: installation candidate, not fully host-accepted release.
- artifacts/preview-light.png and artifacts/preview-dark.png: real browser screenshots.
- package-lock.json: exact dependency versions and integrity metadata.
