# Third-party components

- Notepad++ interface headers: official notepad-plus-plus/notepad-plus-plus v8.9.9.1, GPL-3.0-or-later (Scintilla headers retain their original license).
- Microsoft.Web.WebView2 SDK 1.0.3650.58: Microsoft license, downloaded to .deps; static loader linked into plugin. Include the SDK LICENSE.txt when packaging. WebView2 Runtime is installed separately.
- Markdown rendering: markdown-it, markdown-it-footnote, markdown-it-task-lists, @mdit/plugin-katex, KaTeX, DOMPurify, highlight.js and Mermaid. Exact versions and integrity hashes are recorded in package-lock.json.
- scripts/build-web.mjs collects installed dependency license texts into DEPENDENCY-LICENSES.txt.

Official upstream sources:
https://github.com/notepad-plus-plus/notepad-plus-plus
https://www.nuget.org/packages/Microsoft.Web.WebView2
https://github.com/markdown-it/markdown-it
https://github.com/cure53/DOMPurify
https://github.com/mermaid-js/mermaid
https://github.com/KaTeX/KaTeX
