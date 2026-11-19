import { chromium } from 'playwright';
import { createServer } from 'node:http';
import { readFile, mkdir, readdir } from 'node:fs/promises';
import { resolve, extname, sep } from 'node:path';
import assert from 'node:assert/strict';
const root = resolve('dist/mdviewer/web');
const server = createServer(async (req, res) => {
  try {
    const file = resolve(root, '.' + new URL(req.url, 'http://localhost').pathname);
    if (!file.startsWith(root + sep)) { res.writeHead(403).end(); return; }
    const types = { '.html': 'text/html', '.js': 'text/javascript', '.css': 'text/css', '.woff2': 'font/woff2', '.woff': 'font/woff', '.ttf': 'font/ttf' };
    res.setHeader('Content-Type', types[extname(file)] || 'application/octet-stream');
    res.end(await readFile(file));
  } catch { res.writeHead(404).end(); }
});
await new Promise(r => server.listen(0, '127.0.0.1', r));
let browser;
try {
  let executablePath = process.env.MDVIEWER_CHROMIUM;
  if (!executablePath) {
    const cache = process.env.LOCALAPPDATA + '/ms-playwright';
    const versions = (await readdir(cache)).filter(n => /^chromium-\d+$/.test(n)).sort().reverse();
    if (versions.length) executablePath = cache + '/' + versions[0] + '/chrome-win64/chrome.exe';
  }
  browser = await chromium.launch({ executablePath, headless: true });
  const page = await browser.newPage({ viewport: { width: 900, height: 1100 } });
  const errors = [];
  page.on('pageerror', e => errors.push(e.message));
  await page.goto('http://127.0.0.1:' + server.address().port + '/index.html');
  const markdown = await readFile('examples/showcase.md', 'utf8');
  await page.evaluate(markdown => window.postMessage({ markdown, title: 'showcase.md', key: '1' }, location.origin), markdown);
  await page.locator('article .mermaid svg').waitFor({ timeout: 30000 });
  assert.ok((await page.locator('article .mermaid svg').textContent()).includes('编辑 Markdown'), 'Mermaid labels survive sanitization');
  assert.equal(await page.locator('article table').count(), 1);
  assert.ok(await page.locator('article .katex').count());
  assert.ok(await page.locator('article .hljs-keyword').count());
  await mkdir('artifacts', { recursive: true });
  await page.screenshot({ path: 'artifacts/preview-light.png', fullPage: true });
  await page.locator('#theme').click();
  await page.screenshot({ path: 'artifacts/preview-dark.png', fullPage: true });
  await page.setViewportSize({ width: 360, height: 800 });
  assert.ok(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth));
  await page.evaluate(() => window.postMessage({ markdown: '# Latest 中文\n\n<script>window.pwned=1</script>\n\n[x](javascript:alert(1))\n\n![remote](https://example.com/track.png)', title: 'new.md', key: '2' }, location.origin));
  await page.waitForFunction(() => document.querySelector('article h1')?.textContent === 'Latest 中文');
  assert.equal(await page.evaluate(() => window.pwned), undefined);
  assert.equal(await page.locator('article script, article [href^="javascript:"], article img[src^="https://example"]').count(), 0);
  assert.deepEqual(errors, []);
  console.log('Browser passed: Markdown, table, tasks, math, Mermaid, highlight, theme, narrow layout, update and script/link rejection.');
} finally { if (browser) await browser.close(); server.close(); }
