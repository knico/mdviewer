import { build } from 'esbuild';
import { mkdir, cp, readFile, writeFile } from 'node:fs/promises';
const out = 'dist/mdviewer/web';
await mkdir(out, { recursive: true });
await build({ entryPoints: ['apps/preview-web/app.mjs'], bundle: true, format: 'esm', target: ['chrome120'], outdir: out, entryNames: 'app', assetNames: 'assets/[name]-[hash]', loader: { '.woff': 'file', '.woff2': 'file', '.ttf': 'file' }, minify: true, legalComments: 'external' });
await cp('apps/preview-web/index.html', out + '/index.html');
// Include complete dependency licenses in the installed package.
const lock = JSON.parse(await readFile('package-lock.json', 'utf8'));
let notices = '# Bundled dependency licenses\n\n';
for (const [path, metadata] of Object.entries(lock.packages)) {
  if (!path || metadata.dev) continue;
  let license = '';
  for (const name of ['LICENSE', 'LICENSE.md', 'LICENSE.txt', 'license', 'license.md', 'COPYING']) {
    try { license = await readFile(path + '/' + name, 'utf8'); break; } catch {}
  }
  if (license) notices += '## ' + path + ' @ ' + metadata.version + '\n\n' + license + '\n\n';
}
await writeFile('dist/mdviewer/DEPENDENCY-LICENSES.txt', notices);
console.log('Offline preview assets built.');
