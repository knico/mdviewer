import MarkdownIt from 'markdown-it';
import footnote from 'markdown-it-footnote';
import taskLists from 'markdown-it-task-lists';
import { katex } from '@mdit/plugin-katex';
import hljs from 'highlight.js/lib/common';

export function createRenderer() {
  const md = new MarkdownIt({
    html: false, linkify: true, typographer: true,
    highlight(code, lang) {
      if (lang && hljs.getLanguage(lang)) {
        return hljs.highlight(code, { language: lang, ignoreIllegals: true }).value;
      }
      return '';
    },
  }).use(footnote).use(taskLists).use(katex, { throwOnError: false, trust: false, strict: 'warn', maxExpand: 1000, maxSize: 20 });
  const fence = md.renderer.rules.fence;
  md.renderer.rules.fence = (tokens, i, options, env, self) => {
    if (tokens[i].info.trim() === 'mermaid') {
      return '<pre class="mermaid">' + md.utils.escapeHtml(tokens[i].content) + '</pre>\n';
    }
    return fence(tokens, i, options, env, self);
  };
  return md;
}
