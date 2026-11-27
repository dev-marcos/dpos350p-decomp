// Boots the stock image and the relinked one (tools/relink_test.py) in the JS emulator and
// compares them: same instruction count, interrupts, captured frames, final PC (mapped back to
// the stock address) and framebuffer. Run from the repository root after relink_test.py:
//   node tools/relink_check.mjs [--insns 1500M]
import fs from 'node:fs';
import crypto from 'node:crypto';
import { Machine } from './sim/web/core/machine.js';

const args = process.argv.slice(2);
const opt = (k, d) => { const i = args.indexOf(k); return i < 0 ? d : args[i + 1]; };
const nStr = opt('--insns', '1500M');
const n = parseFloat(nStr) * (nStr.endsWith('M') ? 1e6 : nStr.endsWith('K') ? 1e3 : 1);

const spans = JSON.parse(fs.readFileSync('build/relink/reloc.json', 'utf8'));
const reloc = (a) => { for (const [lo, hi, d] of spans) if (a >= lo && a < hi) return (a + d) >>> 0; return a; };
const back = (a) => { for (const [lo, hi, d] of spans) if (a >= lo + d && a < hi + d) return (a - d) >>> 0; return a; };

function run(image, opts) {
  const m = new Machine(fs.readFileSync(image), opts);
  m.boot(n);
  const fb = m.framebuffer();
  return {
    insns: m.cpu.insns, irqs: m.irqs, frames: m.dev.csi.frames, error: m.error,
    pc: opts.reloc ? back(m.cpu.pc >>> 0) : m.cpu.pc >>> 0,
    screen: fb ? crypto.createHash('sha256').update(fb.data).digest('hex').slice(0, 16) : null,
  };
}

const a = run('build/extract/app.bin', {});
const b = run('build/relink/app.bin', { reloc });
let ok = true;
for (const k of Object.keys(a)) {
  const same = a[k] === b[k];
  ok &&= same;
  const show = (v) => (k === 'pc' ? `0x${v.toString(16)}` : String(v));
  console.log(`${same ? 'same' : 'DIFF'}  ${k.padEnd(7)} stock ${show(a[k])}  relinked ${show(b[k])}`);
}
console.log(ok ? 'relinked image behaves like the stock one' : 'relinked image DIFFERS');
process.exit(ok ? 0 : 1);
