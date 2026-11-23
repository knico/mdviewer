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
  if(process.argv[2] === 'arm-export') {
    await page.evaluate(()=>{window.exportMessages=[];window.chrome.webview.addEventListener('message',e=>window.exportMessages.push(e.data));});
  } else if(process.argv[2] === 'diagnostic') {
    console.log(await page.locator('body').innerText());
    console.log(await page.evaluate(()=>({visibility:document.visibilityState,fonts:document.fonts.status})));
    console.log(await page.evaluate(()=>window.exportMessages));
  } else if(process.argv[2] === 'floating') {
    await page.waitForFunction(() => document.querySelector('article h1')?.textContent === 'Live edit verified');
    const deadline = Date.now() + 20000;
    while (browser.contexts().flatMap(c => c.pages()).filter(p => p.url().startsWith('https://mdviewer.local/')).length < 2 && Date.now() < deadline) await new Promise(r => setTimeout(r, 200));
    const previews=browser.contexts().flatMap(c=>c.pages()).filter(p=>p.url().startsWith('https://mdviewer.local/'));
    assert.equal(previews.length,2,'Docked and independent floating preview');
    for(const preview of previews) await preview.waitForFunction(() => document.querySelector('article h1')?.textContent === 'Live edit verified');
  } else if(process.argv[2] === 'edited') {
    await page.waitForFunction(() => document.querySelector('article h1')?.textContent === 'Live edit verified');
  } else {
    try { await page.locator('article .mermaid svg').waitFor({timeout:25000}); }
    catch(error) { console.log(await page.locator('body').innerText()); await page.screenshot({path:'artifacts/native-failure.png'}); throw error; }
    assert.ok((await page.locator('article h1').textContent()).includes('Markdown'));
    assert.ok(await page.locator('article .katex').count());
    assert.ok((await page.locator('article .mermaid svg').textContent()).includes('编辑 Markdown'));
  }
  if(!['arm-export','diagnostic'].includes(process.argv[2])) await page.screenshot({ path: 'artifacts/native-webview.png', fullPage: true });
  console.log('Embedded WebView2 native check passed: ' + process.argv[2]);
} catch (error) { console.error(error); process.exit(1); }
// End this CDP client without sending Browser.close to the embedded host.
process.exit(0);
