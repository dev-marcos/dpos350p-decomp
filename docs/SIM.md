# Whole-system simulator

The simulator lives in its own repository, https://github.com/dev-marcos/dpos350p-emu, included here as the
git submodule `tools/sim` (`git submodule update --init` after cloning).

## Browser emulator (no server)

The emulator runs entirely in the browser: an ARMv7 core in JavaScript in a Web Worker
(`tools/sim/web/core/`). It is published on GitHub Pages from the dpos350p-emu repository.
Locally: `node tools/sim/tests/serve.mjs`, then open http://localhost:8351. The firmware, the SD
image and saved states stay in the browser (IndexedDB). The CPU core is checked against Unicorn
instruction by instruction: `tools/sim/tests/gen_cpu_cases.py` + `tools/sim/tests/cpu_test.mjs`
(see tools/sim/README.md).

## Python web emulator (local server + browser, reference)

```
python3 -m tools.sim.server            # inside dpos-re; then open http://localhost:8350
```

The page `tools/sim/web/` shows the device in its frame (`frame.svg`) with the emulated screen.
The emulator core runs in this Python process. The page talks only to the JSON/binary API in
`tools/sim/server.py`, so the core can later move to WebAssembly without changing the page.

**Firmware.** Upload the `.iso` update file or `app.bin` from the page (button or drag and drop).
It is kept in `build/sim/` only. The default is `build/extract/app.bin`.

**Screen.** It is refreshed about 7 times per second. Mouse press, drag and release go to the
emulated GT911 touch controller. A quick click is held until the firmware has polled it twice.

**Side panel tabs:**

| Tab | What it does |
|---|---|
| Arquivos | File manager for the emulated SD card (`build/sim/sdcard.img`): browse, download, upload, new folder, delete, FAT32 format. Reset the device after changing files so the firmware sees them. |
| Canais | CH1/CH2 virtual probes: on/off, source (generator or computer audio), waveform, frequency, Vpp, offset, probe 1x/10x. Audio input: choose a capture device, the page streams PCM to the emulator, with a full-scale voltage setting. |
| Sistema | Firmware info and upload, battery voltage and charger state, the "factory process complete" boot option, save/restore emulator state, write the SD image. |
| Dev | PC, instruction count and speed, IRQ counts, FPGA register reads, unmodelled peripheral registers with the first function that touched them, hot spots, UART log. The API also offers `/api/peek` and `/api/poke` to read and write memory. |

**Boot.** A shipped unit has finished the factory flow. Its saved state at 0x42E68AE0 holds
0x5A5555A5, so the firmware skips the POST and the "insert the tester and press OK" steps.
`factory_done` (on by default) reproduces that state, and the main screen appears about 35 s
after power-on. With it off, the firmware runs the full factory flow:
- the POST: disk, FPGA, security chip and ADC pass. Calibration, channel multiplex and the
  150 MHz bandwidth check need calibrated measurements, so `bypass_calibration` marks them OK.
- then the tester screen.

**Battery.** The battery is read from the LRADC (0x01C2280C, 6 bits). The firmware's default
coefficients give mV = 32.947 × code + 2735.7, and it shows 10 bars at 4150 mV or more. The
charger status comes from FPGA selector 0x02, bit 0.

**Time.** Emulated time advances 0.5 OSC tick per instruction, about 48 M instructions per
emulated second. The emulator runs at about 45–50 M instructions per second, so the
interface runs near real time.

**Live trace (browser emulator).** The firmware converts each raw ADC byte with per-lane
coefficients loaded from SPI flash (calibration_load_wrapper 0x400446B4, storage SYS_INFO+0x7E0 =
0x1FB000) and chosen by the selector 0x4004C544. A blank flash gives A = B = -1, so every sample
converts to the same value (the flat -796 mV trace). The browser emulator writes a synthetic
calibration there (`tools/sim/web/core/calibration.js`). Its front end produces the ADC codes that
this calibration converts into offset + volts / (V/div) x 62.9 px/div. The Python reference does not
have this yet.

Findings (STATIC-CONFIRMED from the code unless marked):
- Serialized format: big-endian u32. A matrix of **50 rows x 32 columns** comes first, written row by
  row (RAM object+0x190 + col*0xC8 + row*4). Then come four arrays of 20 values, interleaved per
  index: +0x50, +0xF0, +0xA0, +0x140. That is 0x1A40 bytes in all. The Hack notes (SYS_INFO.md
  section 10) say 25 columns, but the traversal at 0x40044450 walks 32.
- Selector: the row is min(SYS+0x588, SYS+0x6ED) - SYS+0x6EC.
  - Two-channel mode (SYS+0x100 != 0): A = columns 0..7, B = columns 8..15.
  - One-channel mode: A = columns 16..23, B = columns 24..31.
  - The auxiliary arrays (indexed by the CH1/CH2 V/div) go to object+0..+0xC.
- Conversion 0x4004073C:
  - ip = gain[V/div] * A / 1000 (magic 0x10624DD3, shift 6);
  - x = ip * (raw - SYS+0x1B8) + B;
  - out = r2 + E0 * ((x >> SYS+0x264) - r2) / 32, clamped to [0, SYS+0x0A * SYS+0x1C0];
  - r2 = SYS+0x1C0 * offset.
  - At run time: SYS+0x1B8 = 20, SYS+0x264 = 6, SYS+0x1C0 = 64, E0 = 32, SYS+0x0A = 503 px.
- The offset (SYS+0x60 / +0x120) is the screen position of 0 V in pixels. The POST
  channel-multiplex check 0x4004AF0C sets it to 0xAA and 0x1E and expects the measured mean
  (SYS+0xB34 / +0xB50) to follow.
- V/div index (SYS+0x41 / +0x105): 6 shows 200 mV, 5 shows 500 mV (seen in the emulated UI). DERIVED:
  0 = 20 V ... 12 = 2 mV in 1-2-5 steps.
- The 8 lanes behave as interleaved ADC phases (DERIVED from the stable trace and the frequency
  reading): byte i of lane L is sample 8i + L.
- Capture (RUNTIME-CONFIRMED in the emulator):
  - Each CSI capture fills **one** bank of 0x1800 bytes at BUF0. The firmware steps BUF0 through the
    8 banks (0x42DF76D4 + bank * 0x2134).
  - The conversion places lane L at 8i + slot (one channel). With two channels, lanes 0..3 are CH1
    and 4..7 are CH2, 4 per channel.
- The firmware converts the first N samples of every lane, where N*8 = samples on screen + ~1604:
  N = 6138 at 20 us, 2576 at 2 us, 794 at 500 ns and 438 at 200 ns per lane.
- The screen holds 10 horizontal divisions of 95 px, at 50 samples per px. The capture rate is
  therefore 4750 samples per division: 237.5 MS/s at 20 us/div, where the label says 250 MS/s. It
  tops out near 0.95 GS/s, where the label says 1 GS/s.
- Time/div index SYS+0x588: 22 = 10 us, 23 = 20 us, 24 = 50 us (picked in the HOR menu). DERIVED:
  41 = 20 s ... 12 = 5 ns, in menu order.
- The firmware stops timer 0 around each capture (func_40040638: timer_stop, start CSI, wait for
  SYS+0x70E, timer_start).
- Timer 0 runs at 1 kHz (interval 24000). Both simulators used to deliver interrupts only every
  500k instructions, so the 1 ms tick lost about 9 of 10 IRQs and the UI read the touch panel only
  every ~2 s. The browser emulator now stops at the next timer or capture event. The Python
  reference still has the old behaviour.

## Command-line simulator

`tools/sim/` runs a complete application image on an emulated V3s (Unicorn, Cortex-A7)
from `Reset_Handler` (0x4008CB28). Every peripheral access without a model is logged together
with the function that made it, so the log shows what to model next.

```
python3 -m tools.sim.sim [--image build/extract/app.bin] [--insns 600M] [--sd card.img] [--trace-mmio]
```

`screenshot(sim, path)` in `tools/sim/sim.py` renders the DE2 UI layer to a PNG.

## Status (stock ROM)

The stock image reaches the power-on self test and draws it on the emulated display:

| POST line | Meaning | Result |
|---|---|---|
| 磁盘初始化 | disk (SD card) init | 成功 (OK) |
| FPGA检测 | FPGA identity | 成功 |
| 加密芯片检测 | security chip | 成功 |
| ADC检测 | ADC / acquisition | 成功 (CSI frames + DAC-following front end) |
| 系统校准 | system calibration (per-channel, FFT-based) | fails with the simple front end (通道1/2异常); `bypass_calibration=True` marks it OK (emulator shortcut, OPEN) |
| 通道复用检测 | channel multiplex check | fails (异常): next to model |

## Peripheral models (`tools/sim/devices.py`)

| Block | Address | Model | Evidence |
|---|---|---|---|
| CCU | 0x01C20000 | every PLL reports LOCK | DRIVERS.md |
| Timers / AVS | 0x01C20C00 | AVS counters advance on every read (delays end quickly) | DRIVERS.md |
| UART0..2 | 0x01C28000.. | TX captured (USR always "not full") | stock putc |
| SPI0 + SPI NOR | 0x01C68000 | streaming exchange (XCH first, then TX FIFO), W25Q-style flash, 2 MiB area used: settings at 0x1EB000..0x200000; blank flash means defaults are written back | stock sys_spi_cpu_transfer |
| SD/MMC0 + SDHC card | 0x01C0F000 | commands, PIO FIFO and IDMAC descriptors; raw image (default: blank 64 MiB) | stock SD driver |
| FPGA on PE bus | PE0/PE2/PE3/PE8..15 | pin-level strobe protocol; 0x01 identity = A5 A5 5A 5A, 0x60 security chip = present, jobs 0x64..0x6B always ready with result 0 | Hack FPGA_PROTOCOL.md, SMEC.md; stock code |
| DE2 display | 0x01100000 | framebuffer read only: UI channel 1 (0x01104000), RGB565, 1022x600, pitch 2044, at 0x41FB8FE8 | registers written by the stock code |
| Low address space | 0x00000000..0x00FFFFFF | zero-filled memory | see "Firmware quirks" |

Assumptions:
- **SD long responses.** The stock driver asks for a long response on ACMD41 and reads the OCR
  from RESP3. The model copies a short reply into all four response words.
- **FPGA job results.** The job engine (0x64..0x6B) returns 0 for every job. The real transform
  is OPEN.

## Firmware quirks found with the simulator

- **Copy from a NULL pointer.** While creating `/Captures/data.cap`, the firmware calls
  `copy_u16(dst, NULL, n)`. This copies from address 0, which does not fault on the SoC.
- **Halt on a failed check.** A failed POST check halts the firmware (`b .`) after drawing
  异常 in red.
- **Different status selector.** This build polls job status on selector 0x64. The Hack notes
  describe 0x65.

## Interrupts, capture, touch, live view (second iteration)

- IRQ delivery: GIC model (enables 0x01C81100, IAR/EOIR 0x01C8200C/2010) + ARM exception entry
  done by the simulator between execution slices (500k instructions, ~0.5 ms emulated).
- Timer 0/1 -> IRQ 50/51. CSI0 -> IRQ 116 (0x74): a capture start (CAP +0x08) fills the ring
  (BUF0 holds address/4; 8 slots x 6144 bytes, stride 0x2134) from the virtual front end:
  ADC code = (DAC at SYS_INFO+0x44 - 3000) * 256 / 7000 + probe sine. FPGA 0x0A (acquisition
  complete) reads 1.
- Fast paths: FPGA bus primitives, delay_us/ms and GT911 register access are executed in Python
  at function level (same effect, far fewer emulated PIO accesses). D-cache set/way clean
  (0x4006A550) is skipped.
- GT911 touch model (register level; one point; panel axes = display axes, ASSUMPTION).
- `python3 -m tools.sim.live` serves the live display on http://localhost:8350 (frames + mouse
  clicks -> touch). Snapshots: `save_snapshot` / `load_snapshot` in sim.py (not yet wired to
  the live viewer).
- Speed: ~25-45 M instructions/s; the POST (ADC test = 7200 captures) takes about 4 minutes.

## Next steps

1. Add interrupts: GIC distributor/CPU interface (0x01C81000/0x01C82000) and IRQ entry into the
   CPU. These are needed for the timer tick, CSI and USB.
2. Model CSI (0x01CB4000) frame capture, so the FPGA's sample stream lands in RAM and raises
   "frame done". This is where the virtual probe signal will enter.
3. Touch (GT911 on bit-banged I2C, PG6/PG7) and the buttons.
4. Live display window and input.
5. Run the same scenario on three images, as asked:
   - the stock ROM;
   - the image rebuilt from assembly (byte-identical, so it must behave the same);
   - a TEST image with the C rewrites linked in place of the assembly.
