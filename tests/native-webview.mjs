import { chromium } from 'playwright';
import assert from 'node:assert/strict';
let browser;
const deadline = Date.now() + 25000;
while (!browser && Date.now() < deadline) {
  try { browser = await chromium.connectOverCDP('http://127.0.0.1:19247'); }
  catch { await new Promise(r => setTimeout(r, 300)); }
}
assert.ok(browser, 'Isolated WebView2 started');
try {
  let page;
  while (!page && Date.now() < deadline) {
    page = browser.contexts().flatMap(c => c.pages()).find(p => p.url().startsWith('https://mdviewer.local/'));
    if (!page) await new Promise(r => setTimeout(r, 200));
  }
  assert.ok(page, 'Embedded preview page');
  if(process.argv[2] === 'edited') {
    await page.waitForFunction(() => document.querySelector('article h1')?.textContent === 'Live edit verified');
  } else {
    try { await page.locator('article .mermaid svg').waitFor({timeout:25000}); }
    catch(error) { console.log(await page.locator('body').innerText()); await page.screenshot({path:'artifacts/native-failure.png'}); throw error; }
    assert.ok((await page.locator('article h1').textContent()).includes('Markdown'));
    assert.ok(await page.locator('article .katex').count());
    assert.ok((await page.locator('article .mermaid svg').textContent()).includes('编辑 Markdown'));
  }
  await page.screenshot({ path: 'artifacts/native-webview.png', fullPage: true });
  console.log('Embedded WebView2 native check passed: ' + process.argv[2]);
} catch (error) { console.error(error); process.exit(1); }
// End this CDP client without sending Browser.close to the embedded host.
process.exit(0);
