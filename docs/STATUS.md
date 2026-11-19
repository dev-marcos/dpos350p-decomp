# Project status and resume point

Snapshot from 2026-11-18, after Phase 5 batch 64. All addresses apply to build
2025-06-19 only. Work is paused here; this page is the place to start from next time.

## 1. Where things stand

| Item | State |
|---|---|
| `make check` | MATCH, SHA-256 `6635ddd6…cefd58c` |
| Functions with equivalent C | 1213 of 1354 (89.59%) |
| Code bytes covered by C | 223,230 of 337,504 (66.14%) |
| MATCHING C | 0 by construction: Arm Compiler 5 is not licensed here (docs/COMPILER.md) |
| Emulation tests | every EQUIVALENT function passes 200 cases (400 for the C library); full suite after batch 64, all 1213 EQUIVALENT (unchanged builds keep their cached result, see CONTRIBUTING.md) |
| Mutation self-test | 954/954 mutants detected (`make selftest`, full run after batch 64). Before that, from batch 27 to batch 43, a mutant made stale by a rename stopped the full run at mutant 168, so those batches checked only their own mutants. The self-test now lists stale mutants and stops before building anything |
| TEST image | `make testiso`: 741 functions compiled in place, emulator only; boots and the menus work (checked by the user on the batch 13 image) |
| Browser emulator | repo `dev-marcos/dpos350p-emu`, submodule `tools/sim` at `5ed2f7e`, runs fully in the browser (docs/SIM.md) |

The two numbers in the table differ because the finished work is mostly small and medium
functions. The large ones (UI, the table builders, the calibration steps) are still assembly.

### Batches done (Phase 5)

| Batch | Area | Functions | Docs |
|---|---|---:|---|
| 1–2 | emulation harness; all of FatFs R0.13b | 72 | CONTRIBUTING.md |
| 3–4 | Arm C library: AEABI, libm, `pow` (bit-exact) | 27 | LIBRARY.md |
| 5 | low-level drivers (MMIO-exact) | 46 | DRIVERS.md |
| 6 | timers, AVS, delays, CCU clock set-up | 30 | DRIVERS.md |
| 7 | USB device stack (mass storage, UVC) | 179 | USB.md |
| 8 | platform drivers: PIO, IRQ, UART, SPI, FPGA bus, GT911, MS5351M, CSI | 94 | DRIVERS.md, PLATFORM.md |
| 9 | SD/MMC part 2 | 9 | DRIVERS.md |
| 10 | display: TCON0, DE2, framebuffer | 26 | DRIVERS.md |
| 11 | FPGA start-up, SMEC jobs, touch service, power, disk glue | 40 | DRIVERS.md |
| 12 | battery, front end, lane levels, POST multiplex check | 24 | DRIVERS.md |
| 13 | signal generator, calibration selector, system calibration | 16 | DRIVERS.md |
| 14 | acquisition control (state backup, FPGA acquisition selectors, trigger arithmetic) | 29 | DRIVERS.md |
| 15 | settings serialiser, CP437 code page, `ff_wtoupper`, `strlen`, USB disk veneers and mode switches, LRADC read, touch helpers | 19 | DRIVERS.md |
| 16 | touch-control hit tests and drag helpers, complex helpers, LRADC init, `strcmp`, USB disk geometry | 15 | DRIVERS.md |
| 17 | acquisition pipeline: dispatcher, sweep path, capture step, CSI capture blocks and banks, channel post-processing | 15 | DRIVERS.md |
| 18 | CSI bank unpacking: raw ADC lanes to calibrated, interleaved channel samples | 7 | DRIVERS.md |
| 19 | channel filters: moving average, step limiter, display copy, frame-deviation blend | 5 | DRIVERS.md |
| 20 | settings records (save/load through the serialiser and persistence layer), SPI flash command helpers | 23 | DRIVERS.md |
| 21 | UI window layer: window table and stack, redraw dispatcher, 42 redraw wrappers, animated reveal, partial present | 51 | DRIVERS.md |
| 22 | per-trace processing chain (order and gates), runtime re-arm, window close/invalidate/reset, frame update | 18 | DRIVERS.md |
| 23 | user slots, calibration/G/H settings records, load-all at start-up, zoom/time-base index helpers | 19 | DRIVERS.md |
| 24 | LCD bring-up, factory defaults, state-block initialisers, applied-state invalidation, acquisition geometry, SPI flash erase and paged write | 13 | DRIVERS.md |
| 25 | time-base index clamp and steps, committed parameter copy, acquisition apply/reconfigure/re-arm, sample counter, trace buffer pointers, SPI flash page program | 19 | DRIVERS.md |
| 26 | sectioned settings image (encode/decode, records I and J), UI text table with four language columns, `str_copy`, numeric entry pad, trace-span veneers | 26 | DRIVERS.md |
| 27 | touch buttons (record update, rectangles, 36 group updaters, three release handlers), window touch handling, entry-pad modes and sign, sine table, layer clear, small gates | 59 | DRIVERS.md |
| 28 | small leftovers: setters, touch and SYS+0x652 gates, one-byte FPGA commands, pixel and rectangle wrappers, display start, window 5 draw, reload chains (tests in the new `sweep.py` suite) | 45 | DRIVERS.md |
| 29 | drawing colour, buffer rectangle copy, the list at SYS+0xFA0 (insert, count, clamps), cursor clamps, dB/dBmV labels, file-exists checks, per-frame UI chain, window 40 draw, FPGA 0x1C | 23 | DRIVERS.md |
| 30 | file lists for screenshots, waveforms and captures: index load/save, page marks, page check, pruning of missing files, deletion of marked files, capture data block | 19 | DRIVERS.md |
| 31 | file lists, second part: entry removal, next free number, index creation, thumbnails and file loading, saving a new screenshot or waveform, capture records, cursor pair clamp | 22 | DRIVERS.md |
| 32 | widgets (switch, slider, battery, window frame), font descriptors and glyph drawing, pixel blending, framebuffer copy, key repeat, trace cursor limits and defaults | 19 | DRIVERS.md |
| 33 | drawing primitives (vertical line, rectangle fill and outline, line), tinted glyphs, message box, touch button groups, marker values, channel min/max | 14 | DRIVERS.md |
| 34 | view bounds and zoom fit, MMU sections, FPGA fill, FFT input and drawing, storage check file, calibration flags, factory steps, window touch polling, trace cursor values | 12 | DRIVERS.md |
| 35 | touch-button handlers (delete, file lists, language, format, calibration, drag), cursor clamp, FPGA counters, level statistics, second FFT input, FPGA position, measurement setup | 19 | DRIVERS.md |
| 36 | cursor and marker drags, calibration buttons, window and file buttons, numpad fields, level update chain, FPGA parameters 0x0E..0x13, generator self-test | 12 | DRIVERS.md |
| 37 | output and main-menu buttons, numpad keys, third level statistics, second generator self-test, 64-bit window recalculation, table SYS+0x684 | 7 | DRIVERS.md |
| 38 | marker values (dB and linear), FFT levels in dB, FFT trace scale, shaded level drawing, frequency counter | 5 | DRIVERS.md |
| 39 | channel settings, list A/B windows, system window, settings slots, trigger options, measurement numpad fields | 8 | DRIVERS.md |
| 40 | list A thumbnail and list C windows, button groups 0x20A / 0x5C, numpad key buttons, two generator self-tests | 7 | DRIVERS.md |
| 41 | two generator self-tests, touch-target test screen, window touch dispatch, generator settings to the FPGA | 5 | DRIVERS.md |
| 42 | output-level and capture-lane self-tests, numpad text parser, measurement value limits | 4 | DRIVERS.md |
| 43 | trace segment scaling, two level-column graphs, channel 1 level search | 4 | DRIVERS.md |
| 44 | text and number rendering: UTF-8 text draw and width, glyph index, number formatting, alpha glyphs | 6 | DRIVERS.md |
| 45 | SD-card update check (`.iso` in the SD root), SPI flash read and read-modify-write | 3 | DRIVERS.md |
| 46 | dialogs and menus (delete, notices, factory reset, USB, language), colour buttons, XY grid, channel markers, time cursors | 14 | DRIVERS.md |
| 47 | automatic measurements, SI scaling, measurement readout and panel, manual cursors | 5 | DRIVERS.md |
| 48 | saved files: screenshot and waveform names, previews, saved trace, record list (tests of the whole-screen preview in the new `heavy.py` suite) | 7 | DRIVERS.md |
| 49 | screenshot .bmp writer (preview inside the file), linear trace zoom, list C record preview, number text | 4 | DRIVERS.md |
| 50 | value + unit drawers (degrees, uV/mV/V, levels, Hz..GHz), Bode plot grid and cursor panel | 6 | DRIVERS.md |
| 51 | Bode measurement: frequency sweep, point measure with auto-range, phase from crossings, calibration, auto-scale, curves and messages | 6 | DRIVERS.md |
| 52 | acquisition defaults reset, power-on defaults (from a store trace), FPGA parameter sync, generator output calibration, generator self-test | 5 | DRIVERS.md |
| 53 | trace horizontal path: visible bounds, sample shift on a position change, per-segment zoom, gather into the display buffer | 4 | DRIVERS.md |
| 54 | radix-2 FFT, Hann window for the FFT view, amplitude normalization, two trigger-crossing searches; sample-rate table of 0x4003F2F4 documented (not converted) | 5 | DRIVERS.md |
| 55 | zoom resampling (dispatch, cubic spline, windowed sinc), spline evaluation and resample, trace intensity (persistence) view | 7 | DRIVERS.md |
| 56 | spline coefficients, intensity palettes and hit accumulation, peak-hold decimation, level 'Max =' panel, FFT spur capture and cancel | 7 | DRIVERS.md |
| 57 | intensity accumulators with column smoothing (line mode, capture window), level-view layout and frequency axis, level sample window | 4 | DRIVERS.md |
| 58 | level-view frame (grid, level and frequency labels, side colour bars), C1 / C2 markers with readout panel, numeric keypad | 3 | DRIVERS.md |
| 59 | time-base 1-2-5 step table, time-base divisor set-up, Bode gain / phase curve zoom | 3 | DRIVERS.md |
| 60 | automatic trigger level (fewest crossings), SMEC sync of 8 byte parameters, settings record C field list (FFT spurs) | 3 | DRIVERS.md |
| 61 | ADC lane set-up per sample rate, UI layout table, field lists of the user slot, the settings image and the calibration record | 5 | DRIVERS.md |
| 62 | Zoom-window marker of the split view, three-row menu, screenshot viewer frame | 3 | DRIVERS.md |
| 63 | Configuration menu and slot list, zoom ratio and zoom window | 3 | DRIVERS.md |
| 64 | Bode and level-view toolbars and settings bars | 4 | DRIVERS.md |

The exact list, with addresses and case counts, is in PROGRESS.md (`make progress`).

## 2. What is left

762 functions and 267,602 bytes are still assembly only. By size:

| Size | Functions |
|---|---:|
| ≤ 64 bytes | 291 |
| 65–256 bytes | 261 |
| 257–1024 bytes | 148 |
| > 1024 bytes | 62 |

12 of them are Thumb. Remaining work by 16 KB region (functions with C / total):

| Region | Done / total | Notes |
|---|---|---|
| 0x40018000–0x40033FFF | 0 / 144 | Nearly 150 KB, mostly very large functions (up to 5.7 KB). Not analysed yet. Probably UI screens and drawing (OPEN). |
| 0x4002C000 | 0 / 81 | many small functions inside that range; good batch material once their role is known |
| 0x40034000–0x4003BFFF | 11 / 136 | `main` (0x40034700, 148 bytes), runtime state helpers, a 3.6 KB function at 0x4003A984 |
| 0x4003C000 | 113 / 157 | the time-base switch 0x4003F160..0x4003F868 (one function split by the analyser), `final_per_channel_normalization` 0x4003FE14, fragment 0x4003FFE4 |
| 0x40040000 | 39 / 124 | acquisition pipeline with Hack names: `full_acquisition_entry`, `one_runtime_capture_step`, `acquisition_dispatcher`, conversion wrappers, `persistence_read/write`; `sys_mmu_init` (CP15, not emulated) |
| 0x40044000 | 0 / 27 | the largest function in the image (0x40045974, 13.5 KB); calibration serialisation and save/load (0x40044680…) |
| 0x40048000 | 85 / 89 | almost done; `power_off` stays in assembly on purpose (it waits forever for the supply to drop) |
| 0x4004C000 | 16 / 30 | the four big calibration/generator functions: 0x4004B0A0 (3.8 KB), 0x4004BFC8, 0x4004C7C8, 0x4004D880 (5 KB, generator table builder) |
| 0x40050000 | 0 / 28 | not analysed (0x4004FB44 2.4 KB, 0x40053B78 2.5 KB) |
| 0x40054000–0x4005FFFF | 89 / 162 | resampling helpers (Hack names), `__aeabi_memset_entry` |
| 0x40060000–0x4006BFFF | 161 / 295 | CSI format, the field serialiser (`serializer_*`), `mmc_read_csd_cid`, FatFs code-page tables, `strlen`, `__exp_kernel` |
| 0x4006C000 | 147 / 147 | done |
| 0x4008C000 (startup) | 0 / 15 | reset, scatter-load, exception vectors: tiny, but they need a different harness (mode switches, CP15) |

### Suggested next batches, in order

1. **Rest of 0x40060000–0x4006BFFF**: `mmc_read_csd_cid`, `csi_set_fmt`, the two large touch
   functions 0x400695D0 / 0x40069858, 0x40069B0C, and `func_4006a738/4006a868` behind the USB disk
   veneers. Batches 15–16 did the serialiser, code page and touch helpers.
2. **Rest of the acquisition pipeline in 0x40040000**: `full_acquisition_entry` (1 KB),
   `moving_average_and_crossing_alignment_search`, 0x400403AC. Batches 17–19 did the dispatcher,
   sweep, capture blocks, bank unpacking (raw ADC to samples) and the channel filters.
3. **Remaining small functions in 0x4003C000 and 0x40034000–0x4003BFFF**, including `main`.
4. **The time-base switch** (0x4003F160..0x4003F868). First merge the fragments into one function
   in the symbols (`make map` / analyser), then write it.
5. **Generator table builder 0x4004D880** and the calibration steps 0x4004B0A0 / 0x4004BFC8. These
   are large but their callers and data are already understood (batch 13, docs/SIM.md).
6. **First look at 0x40018000–0x40033FFF**: identify the screens through their strings and the
   draw calls, before writing any C. Batch 21 gives a starting point: each of the 42 window draw
   functions there is called from one `ui_window_NN_redraw` wrapper (table in
   `tools/platform_spec/p_ui_windows.py`) and reads its own rect from the window table.

## 3. How to resume (checklist)

Environment: WSL distro `dpos-re`. Run every command as
`wsl -d dpos-re -- bash tools/run.sh <cmd>` from the project root (`F:\dpos350p-decomp`).

1. Sanity check: `make check` (MATCH), then `make emutest` (about 20 minutes; every line must
   say EQUIVALENT).
2. For a new batch:
   1. Read the assembly (`asm/func_XXXXXXXX.s`, or REA/Ghidra) and write C in a new spec file
      `tools/platform_spec/j_<area>.py`. Follow the `q(addr, size, name, desc, body)` pattern of
      `i_acq_control.py`, which includes only `hw/fpga.h`. `gen_platform.py` already includes
      `hw/platform.h`, so do **not** include `hw/drivers.h` as well: its PIO prototypes conflict.
   2. Add prototypes and RAM macros to `include/hw/fpga.h`, or to a new header.
   3. Add the symbol rows **before** building: `python3 tools/gen_platform.py --csv`, then append
      the rows whose address is not in `symbols/manual.csv` yet.
   4. `python3 tools/gen_platform.py`, then `python3 tools/nmbuild.py` (about 10 minutes for all).
      To rebuild one file, call `nmbuild.build(nmbuild.ROOT / "nonmatching/platform/func_X.c",
      nmbuild.original_symbols())`. It needs an absolute path.
   5. Write the tests in `tools/emu/tests/fpga.py`, or in a new suite added to
      `tools/emu/run_tests.py` and `selftest.py`. Run them with
      `python3 -m tools.emu.run_tests --only 0x...`.
   6. Add mutants to `tools/emu/selftest.py`. Each should target a threshold or an operand that a
      careless rewrite would get wrong. All must be caught.
   7. `make map`, `make progress`, `make check`, `python3 tools/testiso.py`.
   8. **After `make map`**, look for older C that still calls the new functions by their
      `func_XXXXXXXX` name. The TEST image report (`build/test/testiso_report.json`, "build
      failed") lists them. Rename them and rebuild.
   9. Update docs/DRIVERS.md (or the area's doc) and this page. Then commit with a short summary
      in Portuguese, without any Claude attribution, and push over SSH
      (`git@github.com:dev-marcos/...`).

### Lessons that cost time before

- When a stub records only some of the arguments, the C can drop an argument without the test
  noticing. Batch 14 found such a case in batch 12. Give every stub its true argument count
  (`rec(name, addr, nargs)`).
- Test setups must draw all random values **before** `setup(m)` runs. The setup runs twice (once
  for stock, once for C), so drawing inside it gives each run different data.
- Stub randomness must be re-seeded per run with `seeded(seed, inner)` and `m.srng`.
- Thumb library helpers (`__aeabi_d2ulz`) must be called through their Thumb address with
  `pcs("aapcs")`. See `D2ULZ` in `include/hw/fpga.h`.
- TEST image builds use VFP only, with no NEON, and `SUBALIGN(4)`. Every chosen build is
  re-verified before it is placed.
- Pass through the stock quirks (functions that forward r0..r3 untouched) with register asm, and
  document them under "Quirks kept".

## 4. Open questions (OPEN)

- What the backup blocks 0x42E180C8 (0xAD20 bytes) and 0x42E68668 (0x198 bytes) hold
  (batch 14).
- The meaning of the trigger fields SYS_INFO+0x288/0x28C/0x2A0/0x63C/0x6F2, and of the
  class written to SYS+0x51C/0x51D. The arithmetic is STATIC-CONFIRMED; the names are DERIVED.
- The generator's FPGA table (selectors 0x3F/0x40/0x43). Decoding it would let the emulator
  produce the real generator waveform instead of a synthesised one. It was offered but not
  started.
- The role of the 0x40018000–0x40033FFF region (probably UI).
- The original compiler options (armcc 5 is assumed). MATCHING needs a licensed Arm Compiler 5.

## 5. Rules that stay in force

- Never write anything to the device: no `xfel write`, no `spinor write`. Any ISO that differs
  from the original has `TEST` in its name and is for the emulator only.
- Never commit FNIRSI binaries: the `.iso`, `app.bin`, or extracted `data/*.bin`.
- Keep fact and interpretation separate, with an evidence level (HW-CONFIRMED / STATIC-CONFIRMED /
  DERIVED / OPEN). Do not call a name "confirmed" unless it is.
- Everything stays inside the project folder. Ask before installing anything large or licensed.
- No Claude attribution in commits or pull requests.
