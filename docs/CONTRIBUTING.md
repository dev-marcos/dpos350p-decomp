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
3. **Write C** in `nonmatching/<module>/func_XXXXXXXX.c`. Header requirements:
   - the first comment line must be `* <name> -- stock 0xXXXXXXXX (...)`, because
     `tools/nmbuild.py` reads the function name from it;
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
| `tools/nmbuild.py` | builds each `nonmatching/**/func_*.c` alone at `0x48000000` (GCC) |
| `tools/emu/` | Unicorn harness (`emu.py`), runner (`run_tests.py`), mutation self-test (`selftest.py`), per-module tests (`tests/`) |
| `tools/cmpfunc.py` | byte and instruction similarity of an object against the image |
| `tools/progress.py` | writes `docs/PROGRESS.md` |
| `tools/fatfs_gen.py` | regenerates `include/fatfs/*` and `nonmatching/fatfs/*` from the pristine FatFs R0.13b sources |
| `tools/whydiff.py` | first divergence when `make check` fails |

Evidence levels (HW-CONFIRMED / STATIC-CONFIRMED / DERIVED / OPEN) apply to every name and
claim. Do not promote a working name to a confirmed one without evidence.
