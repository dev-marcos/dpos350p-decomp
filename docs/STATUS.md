# Project status and resume point

Snapshot from 2026-11-18, after Phase 5 batch 84. All addresses apply to build
2025-06-19 only. Work is paused here; this page is the place to start from next time.

## 1. Where things stand

| Item | State |
|---|---|
| `make check` | MATCH, SHA-256 `6635ddd6…cefd58c` |
| Functions with equivalent C | 1264 of 1354 (93.35%) |
| Code bytes covered by C | 331,398 of 337,504 (98.19%) |
| MATCHING C | 0 by construction: Arm Compiler 5 is not licensed here (docs/COMPILER.md) |
| Emulation tests | every EQUIVALENT function passes 200 cases (400 for the C library); full suite after batch 84, all 1264 EQUIVALENT (unchanged builds keep their cached result, see CONTRIBUTING.md) |
| Mutation self-test | 1363/1363 mutants detected (`make selftest`, full run after batch 84). Before that, from batch 27 to batch 43, a mutant made stale by a rename stopped the full run at mutant 168, so those batches checked only their own mutants. The self-test now lists stale mutants and stops before building anything |
| TEST image | `make testiso`: 788 functions compiled in place, emulator only; boots and the menus work (checked by the user on the batch 13 image) |
| Browser emulator | repo `dev-marcos/dpos350p-emu`, submodule `tools/sim` at `5ed2f7e`, runs fully in the browser (docs/SIM.md) |

The two numbers in the table differ because the 90 functions left are almost all small (about
6 KB together; section 2 says why each stays in assembly).

### Readability work (branch `readability`)

`main` keeps the decompilation exactly as Phase 5 left it. The `readability` branch makes the C
easier for a person to read and edit, and is merged into `main` once it is mature. Every step
keeps `make check` at MATCH, all 1264 functions EQUIVALENT, the self-test complete and testiso at
788 functions.

| Step | State |
|---|---|
| 1. Files named after their function and grouped by module (`nonmatching/<module>/<name>.c`); tools find a function by its header address (`tools/srcindex.py`); the spec generators retired, so the C files are the source | done |
| 2. One header per module (`include/<module>.h`, 45 headers) instead of prototypes repeated in each file; prototypes removed from `include/hw/*.h`; 58 files keep an `__asm__` alias where the stock caller uses a different register interface. Object code of every file unchanged (compared byte for byte, three flag sets) | done |
| 3a. SYS_INFO as `struct sys_info` (`include/sys_info.h`, 423 fields, 102 named with their evidence level) read through `SI(field)`; 2191 of 2280 constant-offset accesses in 317 files rewritten, object code of every file unchanged (three flag sets) | done |
| 3b. The other blocks (`include/state/`): six flat blocks (generator `GENS`, level view `LEVELS`, Bode view `BODES`, factory state `FACT`, touch control `TCS`, entry pad `NUMPAD`) and two record tables (touch buttons `BREC(k, field)`, UI windows `WREC(id, field)`); 1214 of 1287 constant-offset accesses in 133 files rewritten, object code unchanged | done |
| 4. Names for the wrappers and the mechanical names: the 14 `func_…` wrappers and `sys_rect_redraw_gated` after their callees (`plot_markers_draw_gated`, `hspan_fill`, `number_draw_mode1`…), 28 touch updaters after the handler that uses them (`touch_group_0002` → `main_menu_buttons_update`), 6 handlers after the LEVELS / BODES fields they write (`buttons_b56_handler` → `levels_toolbar_buttons`) and 8 other wrappers after their callees; 57 functions in all, every name DERIVED, old → new list in `docs/RENAMES.md`. The rest (handlers `buttons_05c/106/20a/9f8_handler`, `buttons_ac_open_windows`, `button_71e_drag_handler`, the data-address helpers, the self-test steps `gen_selftest_…`) was named in steps 6a and 6b | done |
| 5. Local aliases and raw offsets by name, in lots, each one checked file by file (three flag sets) and the self-test mutants carried along: (a) 53 defines repeated in 381 files moved into `include/shared/`; (b) the touch block through `g_touch->field` (79 files); (c) 82 more SYS_INFO fields named from `docs/DRIVERS.md` (`record_length`, `timebase_period`, `grid_on`…); (d) the UI text table through `UI_TEXT(n)` / `TEXT_STR` / `TEXT_DX` / `TEXT_DY` (32 files); (e) the colour table SYS + 0x3DD530 through `THEME_…` names (34 files); (f) parameter names for small wrappers after their callee (`hspan_fill(y, x0, x1)`, `usb_mass_disk_read(lun, lba, count)`…); (g) the Bode block fully described (plot box, axis ranges, per-point arrays `gain_ratio` / `gain_db` / `phase_deg`, cursor values, calibration) and its aliases `BH(o)` / `BW(o)` / `BD(o)` / `TC32(o)`… rewritten as `BODES(field)` (20 files). All names DERIVED, from the docs they cite | done |
| 6a. 47 more functions whose names carried an address or offset, named from what the code already says: the file-list helpers after their list (`list_cursor_clamp_400362f4` → `list_b_cursor_clamp`, `reload_chain_40035744` → `list_b_reload`, `ui_delete_confirm_164` → `list_b_delete_confirm_draw`), the text-state setters (`word_42e68c74_set` → `font_set`, `half_42e68be2_set` → `text_spacing_set`), the message-box save / restore (`fb_rect_save`, `fb_rect_restore`, `message_box_erase`), setters and clamps after the SYS_INFO field they write (`time_cursors_clamp`, `view_position_set`), veneers after their target, the state-block initialisers after their block (`bode_state_init`…). Every name DERIVED, table in `docs/RENAMES.md`; object code of the 132 changed files identical (three flag sets; for the renamed files with the old and new symbol and section names mapped to the address). The window handlers, self-tests and level functions followed in step 6b | done |
| 6b. 39 more: the window handlers and their updaters after the draw function of their window (`window_buttons_dispatch` id → `ui_window_NN_redraw` → draw function: `buttons_05c_handler` → `side_menu_buttons`, `buttons_20a_handler` → `display_menu_buttons`, `touch_btn_0804_release` → `screenshot_view_buttons`, `button_71e_drag_handler` → `trigger_level_drag_buttons`…), the factory self-tests after the firmware's own step labels (通道平坦度检测: `gen_selftest_50898` → `flatness_selftest`, `selftest_4f5e4` → `adc_lanes_selftest`…), the level functions after the LEVELS fields (`levels_panel_a_stats`…) and five helpers after named SYS_INFO fields (`cursor_pair_clamp`, `capture_samples_decimate`…). Every name DERIVED, table in `docs/RENAMES.md`; object code of the 58 changed files identical (three flag sets, names mapped as in 6a). Still mechanical (meaning OPEN): `icon_19c7c_draw`, `table_684_init`, `trace_defaults_37b48`, `fpga_params_6e4_sync`, `trace_pointers_2b4` | done |

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
| 65 | FFT settings menu, plot grid, right-hand menu column | 3 | DRIVERS.md |
| 66 | Display menu, high-resolution menu, main toolbar | 3 | DRIVERS.md |
| 67 | CH1 and CH2 settings menus | 2 | DRIVERS.md |
| 68 | Time-base menu, measurement selection menu | 2 | DRIVERS.md |
| 69 | Menu touch handlers (measurement, time base), system menu | 3 | DRIVERS.md |
| 70 | Trigger menu, plot edge markers (channel offsets, trigger level and position) | 2 | DRIVERS.md |
| 71 | Generator panel touch handler, plot drag and tap handler | 2 | DRIVERS.md |
| 72 | Drag object picker, trace hit distances and their veneers | 7 | DRIVERS.md |
| 73 | Offset calibration of one volts/div range, AUTO routine and its entry | 3 | DRIVERS.md |
| 74 | USB disk data stage on the SD card, full acquisition, block bank selector | 5 | DRIVERS.md |
| 75 | Acquisition timing set-up from the time base (3.6 KB) | 1 | DRIVERS.md |
| 76 | Generator waveform tables (5 KB, 15 waveforms) | 1 | DRIVERS.md |
| 77 | Lane gain calibration and generator self-test trim searches (5.5 KB) | 3 | DRIVERS.md |
| 78 | Lane phase measurements behind the trim searches (2.4 KB) | 2 | DRIVERS.md |
| 79 | Factory test flow: board test, tester test, generator-to-channel loop-back (4.8 KB) | 3 | DRIVERS.md |
| 80 | UI text table fill (141 entries x 4 languages), main boot sequence and loop, factory reset that keeps the language (13.8 KB) | 3 | DRIVERS.md |
| 81 | File browsers: screenshots, waveforms and records (thumbnail grid, title and bottom bars, 9.0 KB) | 3 | DRIVERS.md |
| 82 | Signal generator screen: parameter tabs, digit read-outs, arrow pad, preview and waveform picker (5 KB) | 1 | DRIVERS.md |
| 83 | Top status bar: CH1, CH2, HOR, ACQ, TRI, GEN and MOV tiles (5.1 KB) | 1 | DRIVERS.md |
| 84 | Cursor read-out panel: cursor-to-volts/seconds conversion, H1 H2 D-T D-F and V1 V2 D-V rows (4.7 KB) | 1 | DRIVERS.md |

The exact list, with addresses and case counts, is in PROGRESS.md (`make progress`).

## 2. What is left

90 functions (6106 bytes) stay in assembly. None of them is a candidate for C; each group says why.

| Group | Functions | Bytes | Why |
|---|---:|---:|---|
| startup | 15 | 400 | Startup code and exception vectors (0x4008Cxxx): reset, scatter-load, VFP enable, exit stubs, one-instruction handlers. They switch CPU modes and run before C; they need a different harness. |
| cp15 | 16 | 824 | Cache, MMU and control-register code (CP15 `mcr`/`mrc`, cache walks by set/way, `sys_mmu_init`). C has no way to express it, and the emulator does not model it. |
| frag | 23 | 1186 | Fragments that the analyser split off a larger function: they start in the middle of a function, use registers set by the code before them, or are literal pools and Thumb data decoded as code. The real function around them is already in C, or is listed here as a whole. |
| switch | 25 | 1440 | The sample-rate switch 0x4003F2F4 with its case fragments 0x4003F4A4..0x4003F6E8 and its tail 0x4003F704 (one function split by the analyser; its table is documented in DRIVERS.md). Its default case reads a stack word that is never written and the caller's registers, so no C version can be EQUIVALENT. |
| undef | 4 | 1756 | Functions whose stock behaviour depends on undefined values (uninitialised registers or stack), so no C version can be EQUIVALENT: 0x400300E8 (reads ip when the value is not found), 0x40061F10 (uninitialised VFP registers), `csi_set_fmt` (uninitialised stack arrays), `mmc_read_csd_cid` (copies 4 bytes past its buffer). |
| lib | 6 | 472 | Library entry points with a register interface, not a C one: `__exp_kernel`, `__frexp_core`, the inner entries of `memset`, the switch-dispatch helper 0x400657FC (reads its table after the call). |
| noret | 1 | 28 | `power_off`: waits forever for the supply to drop (kept on purpose). |

Addresses by group:

- **startup**: 0x4008CB28 `Reset_Handler`, 0x4008CB94 `vfp_enable`, 0x4008CBBC, 0x4008CBEC `__scatterload`, 0x4008CC1C `__rt_exit_veneer`, 0x4008CC24 `exit`, 0x4008CC28 `_sys_exit`, 0x4008CC3C `__rt_exit`, 0x4008CCE4 `Undef_Handler`, 0x4008CCE8 `SWI_Handler`, 0x4008CCEC `PAbort_Handler`, 0x4008CCF0 `DAbort_Handler`, 0x4008CCF4 `IRQ_Handler`, 0x4008CD2C `FIQ_Handler`, 0x4008CF80 `__scatterload_zeroinit`
- **cp15**: 0x40042A60 `sys_mmu_init`, 0x400632F8 `dcache_clean_inv_all`, 0x400632FC, 0x4006330C, 0x4006680C, 0x40066814, 0x40066830, 0x4006684C, 0x40066868, 0x40066884, 0x400668A0, 0x400668BC, 0x4006A550, 0x4006A5FC, 0x4006A658, 0x4006A65C
- **frag**: 0x40040004, 0x40042024, 0x4004219C, 0x400421A0, 0x400421FC, 0x40042200, 0x4004A094, 0x4005E0A8, 0x4005E0AC, 0x4005E104, 0x40062354, 0x40066FB8, 0x40066FC0, 0x40066FD4, 0x40067010, 0x40067058, 0x40067060, 0x40067068, 0x40067070, 0x40067078, 0x40068F64, 0x4006A3F8, 0x4006A4D0
- **switch**: 0x4003F2F4, 0x4003F4A4, 0x4003F4C0, 0x4003F4DC, 0x4003F4F8, 0x4003F514, 0x4003F524, 0x4003F540, 0x4003F55C, 0x4003F56C, 0x4003F57C, 0x4003F598, 0x4003F5B4, 0x4003F5D0, 0x4003F5EC, 0x4003F608, 0x4003F624, 0x4003F640, 0x4003F65C, 0x4003F678, 0x4003F694, 0x4003F6B0, 0x4003F6CC, 0x4003F6E8, 0x4003F704
- **undef**: 0x400300E8, 0x40061F10, 0x40065D04 `csi_set_fmt`, 0x40068134 `mmc_read_csd_cid`
- **lib**: 0x4005C604 `__aeabi_memset_entry`, 0x4005C630, 0x4005C664, 0x400657FC, 0x40069F6C `__frexp_core`, 0x4006A400 `__exp_kernel`
- **noret**: 0x4004A1B4 `power_off`

Phase 5 is complete for everything that can be written as C: each remaining function belongs to one of the groups above. Further work is on the documentation of data and behaviour (section 4), and on new features built on the C sources.

## 3. How to resume (checklist)

Environment: WSL distro `dpos-re`. Run every command as
`wsl -d dpos-re -- bash tools/run.sh <cmd>` from the project root (`F:\dpos350p-decomp`).

1. Sanity check: `make check` (MATCH), then `make emutest` (about 20 minutes; every line must
   say EQUIVALENT).
2. For a new batch:
   1. Read the assembly (`asm/func_XXXXXXXX.s`, or REA/Ghidra) and write C in
      `nonmatching/<module>/<name>.c` (folders in docs/CONTRIBUTING.md, "Source layout"), with
      the `* <name> -- stock 0x<address> (...)` header line. Include `hw/platform.h` and
      `hw/fpga.h` for the hardware macros, and the module headers of the functions it calls.
   2. Add the prototype to the folder's header (`include/<module>.h`), and RAM macros to
      `include/hw/fpga.h`.
   3. Add the symbol row to `symbols/manual.csv` **before** building.
   4. `python3 tools/nmbuild.py` (about 10 minutes for all). To rebuild one file, call
      `nmbuild.build(nmbuild.ROOT / "nonmatching/<module>/<name>.c", nmbuild.original_symbols())`.
      It needs an absolute path.
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
