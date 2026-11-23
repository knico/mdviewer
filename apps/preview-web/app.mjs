import DOMPurify from 'dompurify';
import mermaid from 'mermaid';
import { createRenderer } from '../../packages/markdown-core/renderer.mjs';
import './style.css';
import 'highlight.js/styles/github-dark.css';
import 'katex/dist/katex.min.css';

const md = createRenderer();
const article = document.querySelector('article');
const title = document.querySelector('#document-title');
const status = document.querySelector('#status');
let revision = 0;
let pending = null;
let rendering = false;
let previousKey = '';
let activeTheme = 'system';
let exportId;
mermaid.initialize({ startOnLoad: false, securityLevel: 'strict', maxTextSize: 50000, suppressErrorRendering: true, htmlLabels: false, flowchart: { htmlLabels: false } });

async function drain() {
  if (rendering) return;
  rendering = true;
  try {
    while (pending) {
      const data = pending;
      exportId = data.exportId;
      pending = null;
      const id = ++revision;
      const sameDocument = data.key === previousKey;
      const scroll = sameDocument ? window.scrollY : 0;
      previousKey = data.key;
      title.textContent = data.title || 'Untitled';
      const fragment = DOMPurify.sanitize(md.render(data.markdown || ''), { RETURN_DOM_FRAGMENT: true, ADD_TAGS: ['annotation'], ADD_ATTR: ['encoding'], FORBID_TAGS: ['style', 'iframe', 'object', 'embed', 'form'] });
      for (const image of fragment.querySelectorAll('img')) {
        image.removeAttribute('src');
        image.removeAttribute('srcset');
        image.alt = (image.alt || '') + ' [image preview unavailable]';
      }
      article.replaceChildren(fragment);
      for (const node of article.querySelectorAll('.mermaid')) {
        if (pending) break;
        try {
          const source = node.textContent.replace(/^\s*%%\{.*?\}%%/gms, '');
          const { svg } = await mermaid.render('diagram-' + id + '-' + Math.random().toString(36).slice(2), source);
          if (!pending && node.isConnected) node.innerHTML = DOMPurify.sanitize(svg, { USE_PROFILES: { svg: true, svgFilters: true } });
        } catch {
          node.classList.add('diagram-error');
          node.setAttribute('aria-label', 'Diagram could not be rendered');
        }
      }
      window.scrollTo(0, scroll);
      status.textContent = data.markdown ? 'Preview updated · offline' : 'Open a Markdown document to begin';
      if (Number.isSafeInteger(data.exportId)) {
        await document.fonts.ready;
        // Hidden/minimized WebViews can suspend animation frames. Layout is
        // performed by PDF export; only await resources, not visible frames.
        await new Promise(resolve => setTimeout(resolve, 0));
        window.chrome?.webview?.postMessage('pdf-ready:' + data.exportId);
      }
    }
  } catch (error) {
    status.textContent = 'Preview failed: ' + error.message;
    if (Number.isSafeInteger(exportId)) window.chrome?.webview?.postMessage('pdf-error:' + exportId);
  } finally { rendering = false; }
}

export function updatePreview(data) {
  if (!data || typeof data.markdown !== 'string') return;
  pending = data;
  void drain();
}
document.querySelector('#theme').addEventListener('click', () => {
  activeTheme = activeTheme === 'dark' ? 'light' : 'dark';
  document.documentElement.dataset.theme = activeTheme;
});
document.addEventListener('click', event => {
  const link = event.target.closest('a');
  if (!link) return;
  event.preventDefault();
  const href = link.getAttribute('href') || '';
  if (href.startsWith('#')) { try { document.getElementById(decodeURIComponent(href.slice(1)))?.scrollIntoView(); } catch {} return; }
  if (href.startsWith('https://') || href.startsWith('http://')) {
    status.textContent = 'External link: ' + href + ' (copy link address to open)';
  }
});
if (window.chrome?.webview) {
  window.chrome.webview.addEventListener('message', event => updatePreview(event.data));
  window.chrome.webview.postMessage('ready');
}
// Browser harness uses the same JSON message boundary as the native host.
window.addEventListener('message', event => {
  if (event.source === window && event.origin === location.origin) updatePreview(event.data);
});
