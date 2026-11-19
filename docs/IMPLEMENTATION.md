# mdviewer implementation

Scope: Windows x64 Notepad++ plugin, dockable WebView2 preview, local bundled rendering assets, debounced edits and buffer switches.

Markdown target: CommonMark plus tables, task lists, strikethrough, fenced code highlighting, footnotes, math and Mermaid diagrams. There is no universal "latest Markdown" standard; supported extensions are explicitly listed and tested.

Architecture: the editor-independent Markdown core lives in `packages/markdown-core`; the Web preview app lives in `apps/preview-web`; and the current Notepad++ adapter lives in `adapters/notepad-plus-plus/native`. The native adapter reads the active Scintilla buffer and a fixed local HTTPS virtual host serves the preview application. Markdown travels through WebView2 JSON messages, never generated script. Rendering dependencies ship locally. Raw HTML is disabled and generated output is sanitized.

Acceptance gates: JavaScript rendering tests; Windows x64 native compilation; browser visual/security checks; real Notepad++ plugin loading, docking, edits, tab switches and shutdown. Record completed gates and remaining limits in README.md.
