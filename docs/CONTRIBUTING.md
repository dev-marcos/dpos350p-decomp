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
