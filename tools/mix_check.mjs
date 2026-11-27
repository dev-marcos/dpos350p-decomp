// Boots the stock image and the mixed C + assembly image (tools/mixlink.py) in the JS emulator
// and compares what they show. The C code runs a different number of instructions, so the
// images are compared at the same point of the acquisition (after K captured frames) instead
// of after the same instruction count: framebuffer hash and differing pixels at each K.
//   node tools/mix_check.mjs [--image build/mix/app.bin] [--reloc build/mix/reloc.json]
//                            [--frames 100,400,800] [--insns 4000M] [--png build/mix]
import fs from 'node:fs';
import crypto from 'node:crypto';
import zlib from 'node:zlib';
import { Machine } from './sim/web/core/machine.js';

const args = process.argv.slice(2);
const opt = (k, d) => { const i = args.indexOf(k); return i < 0 ? d : args[i + 1]; };
const num = (s) => parseFloat(s) * (s.endsWith('M') ? 1e6 : s.endsWith('K') ? 1e3 : 1);
const image = opt('--image', 'build/mix/app.bin');
const spans = JSON.parse(fs.readFileSync(opt('--reloc', 'build/mix/reloc.json'), 'utf8'));
const reloc = (a) => { for (const [lo, hi, d] of spans) if (a >= lo && a < hi) return (a + d) >>> 0; return a; };
const marks = opt('--frames', '100,400,800').split(',').map(Number);
const limit = num(opt('--insns', '4000M'));
const pngDir = opt('--png', null);

const crcT = new Int32Array(256).map((_, n) => { let c = n; for (let k = 0; k < 8; k++) c = c & 1 ? 0xEDB88320 ^ (c >>> 1) : c >>> 1; return c; });
const crc = (b) => { let c = -1; for (const x of b) c = crcT[(c ^ x) & 0xFF] ^ (c >>> 8); return (c ^ -1) >>> 0; };

function png(fb, path) {
  const raw = Buffer.alloc((fb.w * 3 + 1) * fb.h);
  for (let y = 0; y < fb.h; y++) for (let x = 0; x < fb.w; x++) {
    const v = fb.data[y * fb.w + x], o = y * (fb.w * 3 + 1) + 1 + x * 3;
    raw[o] = (v >> 11) << 3; raw[o + 1] = ((v >> 5) & 63) << 2; raw[o + 2] = (v & 31) << 3;
  }
  const chunk = (t, d) => { const b = Buffer.alloc(12 + d.length); b.writeUInt32BE(d.length); b.write(t, 4); d.copy(b, 8);
    b.writeUInt32BE(crc(b.subarray(4, 8 + d.length)), 8 + d.length); return b; };
  const ih = Buffer.alloc(13); ih.writeUInt32BE(fb.w); ih.writeUInt32BE(fb.h, 4); ih[8] = 8; ih[9] = 2;
  fs.writeFileSync(path, Buffer.concat([Buffer.from([137, 80, 78, 71, 13, 10, 26, 10]), chunk('IHDR', ih),
    chunk('IDAT', zlib.deflateSync(raw)), chunk('IEND', Buffer.alloc(0))]));
}

// RGB565 pixels without the line padding
function packed(fb) {
  const data = new Uint16Array(fb.w * fb.h);
  for (let y = 0; y < fb.h; y++) for (let x = 0; x < fb.w; x++)
    data[y * fb.w + x] = fb.data[y * fb.pitch + 2 * x] | (fb.data[y * fb.pitch + 2 * x + 1] << 8);
  return { data, w: fb.w, h: fb.h };
}

function run(img, opts, tag) {
  const m = new Machine(fs.readFileSync(img), opts);
  const shots = [];
  let k = 0;
  m.boot(limit, 200000, (mm) => {
    while (k < marks.length && mm.dev.csi.frames >= marks[k]) {
      const fb = mm.framebuffer();
      const pic = fb && packed(fb);
      shots.push(pic ? { ...pic, insns: mm.cpu.insns } : null);
      if (pic && pngDir) png(pic, `${pngDir}/${tag}_f${marks[k]}.png`);
      k++;
    }
    return k >= marks.length;
  });
  return { m, shots };
}

const a = run('build/extract/app.bin', {}, 'stock');
const b = run(image, { reloc }, 'mixed');
let ok = !b.m.error && b.shots.length === marks.length;
console.log(`stock: ${a.m.cpu.insns} insns, irqs ${a.m.irqs}, frames ${a.m.dev.csi.frames}${a.m.error ? ', ' + a.m.error : ''}`);
console.log(`mixed: ${b.m.cpu.insns} insns, irqs ${b.m.irqs}, frames ${b.m.dev.csi.frames}${b.m.error ? ', ' + b.m.error : ''}`);
for (let i = 0; i < marks.length; i++) {
  const x = a.shots[i], y = b.shots[i];
  if (!x || !y) { console.log(`frame ${marks[i]}: ${!x ? 'stock' : 'mixed'} has no picture`); ok = false; continue; }
  let diff = 0;
  for (let p = 0; p < x.data.length; p++) diff += x.data[p] !== y.data[p];
  const h = (s) => crypto.createHash('sha256').update(Buffer.from(s.data.buffer)).digest('hex').slice(0, 16);
  console.log(`frame ${String(marks[i]).padStart(4)}: stock ${h(x)} @${(x.insns / 1e6).toFixed(0)}M  mixed ${h(y)} @${(y.insns / 1e6).toFixed(0)}M  ` +
              `${diff ? diff + ' pixels differ' : 'same picture'}`);
  ok &&= diff === 0;
}
console.log(ok ? 'mixed image shows the same as the stock one' : 'mixed image DIFFERS');
process.exit(ok ? 0 : 1);
