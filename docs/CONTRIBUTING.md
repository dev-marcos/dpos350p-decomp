# Contributing: decompiling a function (Phase 5 workflow)

The golden rule never changes: `make check` must print `MATCH` for
SHA-256 `6635ddd6…d58c` on every commit. The shipped image is always built from `asm/`.

Without a licensed Arm Compiler 5 (see `docs/COMPILER.md`), rewritten C is verified by
**behaviour**, not by bytes. It lives in `nonmatching/` and never enters the ISO build.

## Steps

1. **Pick a function** from `symbols/symbols_2025-06-19.csv` or `docs/PROGRESS.md`. The
   suggested order is libraries (FatFs, libc, maths), then low-level drivers, then the FPGA
   protocol and acquisition, then measurements, FFT and FRA, and the UI last.
2. **Understand it.** Use REA/Ghidra pseudocode (MCP `rea-agents`, or
   `rea … --provider ghidra`), plus `python3 tools/armdis.py ADDR LEN [--thumb]` and the
   original `asm/func_XXXXXXXX.s`.
3. **Write C** in `nonmatching/<module>/<name>.c` (see "Source layout" below). Header requirements:
   - the first comment line must be `* <name> -- stock 0xXXXXXXXX (...)`. The tools take the
     function name and the stock address from this line, not from the file name
     (`tools/srcindex.py`), so a file can be renamed or moved freely;
   - put real types and structs in `include/`, with `_Static_assert` on every field offset
     you rely on (see `include/fatfs/ff_stock.h`);
   - every external function and global is resolved to its **original** address at link
     time, so each test isolates one function.
4. **Name it.** Add a row to `symbols/manual.csv` with an evidence level and its source, then
   run `make map`.
5. **Write a test** in `tools/emu/tests/<module>.py`. A case builder returns
   `Case(args, setup, ret, stubs)`:
   - **stubs** replace I/O and driver calls (`disk_read`, MMIO-heavy helpers). The same stub
     runs for both versions, and its calls become part of the compared behaviour.
   - **cover every branch.** Where a callee makes a branch unreachable (e.g. CP437 never
     returns a DBC), stub that callee too.
   - **Scenario tests** for code that needs a filesystem (`tools/emu/tests/fatfs_fs.py`) build
     the machine with `Case.build(code)`. `tools/emu/fatdisk.py` provides FAT12/16/32/exFAT
     volumes that were formatted by the stock `f_mkfs` and populated through the stock API
     (`build/emu/disks/`). Disk I/O is recorded as (sector, count, data hash). The callee
     stack area is cleared before each measured call, so uninitialised reads are
     deterministic. Cases where both versions fault identically are counted as `both-fault`
     and should be avoided.
6. **Run the tests:**
   - `make emutest` runs every case for the stock function and for your C, and compares
     return values, final memory and stub-call sequences;
   - `make selftest` runs the mutation self-test, which should report all mutants detected.
     Add a mutant for any non-trivial branch you test.
7. **Update the progress report:** `make progress` regenerates `docs/PROGRESS.md`.
8. **Check and commit:** run `make check` (still `MATCH`), then commit the C file, the test,
   the symbols and `docs/PROGRESS.md`.

When an Arm Compiler 5 licence is available, the same C files are compiled with the original
flags and compared with `tools/cmpfunc.py`. A byte-identical function moves to `src/` and
replaces its `asm/func_XXXXXXXX.s`.

## Source layout

The C files are the source: edit them directly. (On `main`, most of them were generated from
the Python specs in `tools/platform_spec/` and `tools/usb_spec/` by `gen_platform.py`, `gen_usb.py` and
`fatfs_gen.py`; the `readability` branch retired those generators.) Each file holds one stock
function, plus its `static` helpers, and is named after the function.

| Folder | Contents |
|---|---|
| `drivers/<peripheral>/` | SoC and board drivers: `gic`, `uart`, `spi`, `csi`, `sdmmc`, `display`, `lradc`, `ccu`, `timer`, `avs`, `pio`, `wdt`, `mmio`, and the external chips `gt911` (touch), `ms5351` (clock generator), `fpga_bus` |
| `fpga/` | FPGA command protocol, SMEC jobs, ADC lane calibration |
| `frontend/` | analogue front end: V/div, offsets, coupling, auto set |
| `acquisition/` | time base, trigger, capture and bank unpacking |
| `processing/` | trace processing, filters, resampling, FFT, measurements, zoom and cursors |
| `generator/` | signal generator |
| `bode/` | frequency response (Bode) sweep |
| `power/` | battery, charger, auto power-off |
| `storage/settings/` | settings, calibration and the field serializer |
| `storage/files/` | screenshot, waveform and record files and their lists |
| `storage/diskio/` | FatFs disk glue |
| `ui/draw/` | drawing primitives, fonts, text, colours, frame buffers |
| `ui/windows/` | window stack, redraw and frame update |
| `ui/screens/` | menus, panels, dialogs, browsers and the status bar |
| `ui/input/` | touch buttons, button handlers, numeric pad |
| `ui/text/` | UI text tables and language selection |
| `selftest/` | factory and self tests |
| `system/` | `main`, defaults, state backup and restore |
| `util/` | string and memory helpers |
| `usb/` | USB controller (`usbc`), core, mass storage, CDC, HID, UVC, PHY |
| `fatfs/`, `libc/`, `libm/` | FatFs R0.13b and the compiler run-time and maths library |

The folder of a function is an interpretation (DERIVED), like its name. Move a file when a
better place is found, and move its prototype to the matching header.

### Headers

Each folder has one header in `include/` with the same path: `nonmatching/ui/draw/` →
`include/ui/draw.h`, `nonmatching/fpga/` → `include/fpga.h`. It declares every function of the
folder, in address order, with the parameter names of the definition and the stock address. A C
file includes the headers of the functions it calls and of its own function, so the compiler
checks each definition against its prototype. `include/hw/` keeps the hardware addresses,
register macros and shared types.

Three headers are special:

- `include/stock_asm.h`: functions that stay in assembly, declared the way the C callers use them;
- `include/fatfs_app.h`: the FatFs API as the application calls it (`FIL`, `DIR` and `FATFS`
  passed as raw buffers). FatFs's own files keep `include/fatfs/ff.h`;
- the FatFs disk layer (`disk_read`…) is declared for FatFs by `include/fatfs/diskio.h` and for
  the rest by `include/storage/diskio.h`.

Some callers use a different register interface from the definition. For example,
`ui_window_close(id, a, b)` is called with only `id`, and r1 and r2 keep whatever they hold.
Those files declare the call form under another C name bound to the same symbol, and say so in a
comment:

```c
void ui_window_close_raw(u32 id) __asm__("ui_window_close");
```

The C must pass exactly the registers the stock caller passes, so these aliases stay until a
change is verified in the emulator.

The headers were introduced on the `readability` branch without changing a single instruction:
every file's object code (code, data and relocations, with the nmbuild `-O2` flags and the
testiso `-O2` and `-Os` flags) was compared with the one built before the change.

### State blocks

`include/sys_info.h` describes SYS_INFO, the main state block at 0x40544E68, as
`struct sys_info`. It lists the 423 fields the C reaches with a constant offset; the gaps are
padding, and a `_Static_assert` pins each field to its offset. A named field has a comment with
its offset, its evidence level and its source (the Hack map
`third_party/FNIRSI-DPOS350P-Hack/docs/SYS_INFO.md` or the project's own analysis). A field
without evidence keeps the offset as its name (`f_598`). To rename one, change the name in the
header and in the files that use it, and say what the evidence is.

Read and write a field with `SI(field)`:

```c
SI(ch1_enabled) = 1;
if (SI(trigger_mode) == 2) ...
```

`SI` is a volatile access at `0x40544E68 + offsetof(struct sys_info, field)`. GCC emits the same
instructions as the raw `SYS8/16/32/64(off)` and `B8(SYS + off)` accessors. Member syntax on a
volatile struct pointer is not used, because GCC then picks a different base and offset split
and the code changes. Some accesses stay raw:

- computed offsets (`SYS32(0x808 + 4 * i)`) and addresses taken (`SYS + 0x872`);
- accesses whose type differs from the field's type, for example an s64 read of a field that is
  u64 in most places, or a 32-bit read of half of a 64-bit field.

## TEST image for the emulator

`make testiso` builds `build/test/DPOS350P_TEST.iso`: the stock image with every EQUIVALENT C
function compiled in place (`tools/testiso.py`). **Emulator only, never write it to a device.**

- Each function is compiled with GCC and linked at its original address. Calls and data go to
  the original addresses.
- It is written over the stock bytes only if it fits in the original function's size. `-O2` is
  tried first, then `-Os`.
- The TEST builds use VFP without NEON. The stock code has no NEON and neither does the browser
  emulator: one NEON instruction made the first attempt stop.
- Every chosen build is therefore run again through its `tools/emu` cases, and used only if it
  is EQUIVALENT.
- Calls into stock **Thumb** code (`sin`, `exp`, `sqrt`, `__aeabi_l2d`, `__aeabi_d2lz`…, including
  the helpers GCC calls on its own for `long long` ↔ `double`) must switch state, as the stock
  BLX does. The stock symbols are absolute values in the link script, so ld does not know they are
  Thumb and links a plain BL. `nmbuild.fix_thumb_calls` rewrites each such BL as BLX, using the
  relocations kept by `ld -q`. A B or BL<cond> straight into Thumb code cannot switch state: that
  function is left out of the image. The emulator tests cannot catch this, because they stub
  those routines by address and the scratch build at 0x48000000 reaches them through an
  interworking veneer. Before this fix, 46 calls in 12 functions ran Thumb code in ARM state, and
  the TEST image stopped at boot (undefined instruction at 0x4003A5E0). After the fix, the
  headless boot (`node tools/sim/tests/boot.mjs --image build/test/app_TEST.bin`) gives a screen
  identical to the stock image's.
- Left out:
  - functions that do not fit;
  - functions whose stock code is Thumb (callers enter them in Thumb state);
  - C files that define their own variables.
- `build/test/testiso_report.json` lists what was replaced and why the rest was not. The
  container (logo, header, checksum footer) is packed by `tools/dposiso.py`, as for the stock
  file.

## Build and test speed (parallel jobs and caches)

- `tools/nmbuild.py` compiles the C files in parallel, one job per CPU (`-j N` changes this).
  - The JSON of each build keeps two keys. `okey` hashes the C file, every header in `include/`,
    the compiler flags and the script. `key` adds the address that each symbol in the object's
    `undef` list resolves to.
  - A file is skipped when both keys match, so adding unrelated symbols rebuilds nothing.
  - The link script lists only the symbols the object uses. The images stay byte-identical,
    which was checked: every test result stayed cached after the switch.
  - `--force` rebuilds everything.
  - Compilation waits mostly on `/mnt/f`: a full rebuild uses about 55 s of CPU in 6 minutes,
    so `-j 32` can help there.
- `tools/emu/run_tests.py` runs the functions in parallel worker processes (`-j N`). Each case
  is seeded with `seed ^ address`, so results do not depend on order.
  - Each result in `build/emu/results.json` keeps a key, and a function whose key is unchanged
    keeps its result without running.
  - The key covers the C build (`.bin` and entry), the test suite file, the harness core
    (`emu.py`, `fatdisk.py`, `run_tests.py`, `tests/__init__.py`, `tests/fatfs.py`), `app.bin`,
    the case count and the seed.
  - Editing a suite file such as `tests/fpga.py` reruns every test in that suite.
  - New tests go in `tests/sweep.py` (from batch 28 on), so that the older suites stay cached.
    A function that takes seconds per run in the emulator (a whole-screen loop) goes in
    `tests/heavy.py` instead, which runs 6 cases (4 in the self-test). A test can raise the
    instruction budget in its setup (`m.max_insns = ...`).
    That file copies the helpers it needs instead of importing them from `fpga.py`, because a
    suite's key covers only its own file.
  - A plain `Stub` records only r0..r3. For a callee with stack arguments, record them as well
    (`rec_stk` in `tests/sweep.py`), or a wrong fifth argument goes unnoticed.
  - The opposite also matters: do not record the address of a buffer on the caller's stack (a
    `FIL`, for example). It depends on the frame layout, so an `-Os` build that is correct fails
    against it. `quiet` in `tests/sweep.py` gives a stub the registers without recording them.
  - C that calls a function renamed later must use the new name. Search the C files for the
    old name after every rename.
  - In a `compose(...)` setup, later writes win. A random block written after `t17(r)` that
    covers touch byte +0x17 undoes the gate choice, and the function then almost always returns
    early. Put `t17(r)` last.
  - `--no-cache` runs everything.
- `tools/testiso.py` processes the C files in parallel and keeps two caches:
  - `build/test/process_cache.json` holds the outcome per function: the chosen build bytes, or
    the reason it was skipped. Its key is the nmbuild link key, `testiso.py`, the stock size and
    type, the option and the test suite.
  - `build/test/verify_cache.json` holds the passing emulator checks.
  - `--no-cache` redoes every function.
- `make progress` still calls `run_tests`. With the cache, that call only reruns what changed.

Measured on 2026-11-16 with 822 functions and 8 CPUs, under WSL on `/mnt/f`. The results were
identical to the serial run.

| Step | Before | Parallel, no cache | Cached |
|---|---:|---:|---:|
| `nmbuild` | 866 s | 426 s | 5 s |
| `run_tests` | 1242 s | 397 s | 6 s |
| `testiso` | not measured | 920 s | 5 s |

Compilation gains less than the CPU count because file access on `/mnt/f` is the limit there.

## Tools

| Tool | Purpose |
|---|---|
| `tools/armdis.py` | quick Capstone disassembly of the image |
| `tools/srcindex.py` | maps each stock address to its C file, from the file's header line |
| `tools/nmbuild.py` | builds each `nonmatching/**/*.c` alone at `0x48000000` (GCC) |
| `tools/emu/` | Unicorn harness (`emu.py`), runner (`run_tests.py`), mutation self-test (`selftest.py`), per-module tests (`tests/`) |
| `tools/cmpfunc.py` | byte and instruction similarity of an object against the image |
| `tools/progress.py` | writes `docs/PROGRESS.md` |
| `tools/whydiff.py` | first divergence when `make check` fails |

Evidence levels (HW-CONFIRMED / STATIC-CONFIRMED / DERIVED / OPEN) apply to every name and
claim. Do not promote a working name to a confirmed one without evidence.
