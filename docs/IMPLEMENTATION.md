# Architecture

mdviewer separates Markdown rendering from the editor integration.

- `packages/markdown-core` owns Markdown parsing and sanitization.
- `apps/preview-web` renders the preview UI and handles messages from the host.
- `adapters/notepad-plus-plus/native` integrates with Notepad++ and Scintilla, hosts WebView2, and synchronizes the active document.

The host sends document content to WebView2 using JSON messages. Preview assets are served from the plugin's local `web` directory through a fixed virtual host. The adapter restricts navigation, downloads, permissions, pop-up windows, and host objects. Raw HTML is disabled and rendered output is sanitized.

The supported syntax is CommonMark with tables, task lists, strikethrough, fenced-code highlighting, footnotes, math and Mermaid diagrams. There is no single extensible Markdown specification that defines a universal “latest” feature set.

New editor integrations belong in `adapters/<editor>`. They should reuse the shared renderer and preview protocol without bringing editor APIs, native window handles, or Scintilla calls into the shared packages.
