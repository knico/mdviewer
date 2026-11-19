import test from 'node:test';
import assert from 'node:assert/strict';
import { createRenderer } from '../packages/markdown-core/renderer.mjs';
const md = createRenderer();
test('CommonMark and GFM extensions', () => {
  const html = md.render('# 中文标题\n\n| A | B |\n|---|---|\n| 1 | 2 |\n\n- [x] done\n\n~~deleted~~\n\nfootnote[^a]\n\n[^a]: details');
  for (const value of ['<h1>中文标题</h1>', '<table>', 'type="checkbox"', '<s>deleted</s>', 'footnote']) assert.ok(html.includes(value), value);
});
test('HTML disabled and unsafe links rejected', () => {
  const html = md.render('<script>alert(1)</script>\n\n[x](javascript:alert(1))');
  assert.ok(!html.includes('<script>'));
  assert.ok(!html.includes('href="javascript:'));
});
test('math, code highlighting and Mermaid source', () => {
  const html = md.render('$x^2$\n\n```javascript\nconst x = 1;\n```\n\n```mermaid\nflowchart LR\nA --> B\n```');
  assert.ok(html.includes('katex'));
  assert.ok(html.includes('hljs-keyword'));
  assert.ok(html.includes('class="mermaid"'));
});
