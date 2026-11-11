# Assembly split (Phase 3)

`make check` rebuilds the ISO from these inputs only:

- `asm/*.s`: 1,350 files, committed:
  - `asm/vectors.s`: the exception vectors;
  - `asm/func_XXXXXXXX.s`: 1,345 functions;
  - `asm/data_XXXXXXXX.s`: 4 blob wrappers.
- `data/*.bin`: 4 large data areas (fonts, graphics, rodata, packer garbage). These are
  **extracted from the user's ISO** at build time (`tools/split.py extract`, checked against
  the SHA-256 values in `data/blobs.csv`) and are never committed.
- `ld/dpos350p.ld` + `ld/sections.ld`: one input section per unit (`.text.<address>`), placed
  in original address order at `0x40000000`. The script asserts that the image is exactly
  `0x957E0` bytes.

## Unit layout

A function file holds:

- the function;
- everything after it up to the next function: shared literal pools (`.word`), inline
  strings (`.byte`) and zero padding (`.space`);
- bytes that the function's own branches jump over, at their original position.

Functions are named after `symbols/symbols_2025-06-19.csv` (`func_XXXXXXXX` when unnamed). The
file name is always the address, so file names stay stable when symbols are renamed.

## Instruction text

- Every instruction is Capstone's disassembly, written in GNU `as` unified syntax.
- Direct branches and calls use symbols:
  - **another function**: its global symbol, which becomes a normal `R_ARM_CALL`/`JUMP24`
    relocation at link time;
  - **the same function**: the local label `.LXXXXXXXX`, or `.Lstart_XXXXXXXX` for the entry;
  - **inside another function**: `loc_XXXXXXXX` (22 such shared-code labels). These are global,
    and the owning file defines a local alias too, so that its own short branches stay short.
- A literal-pool word becomes `.word <symbol>` only when its value is the address of a function
  entry (with bit 0 set for Thumb entries). Everything else stays numeric: 11 symbolised and
  2,298 numeric pool words. Turning pointers into data, RAM and peripherals into symbols is
  Phase 7 work.
- PC-relative loads keep their encoded offset (`ldr r0, [pc, #0xbbc]`). This is valid because
  the layout is fixed; Phase 7 will replace these with labels.

`tools/split.py gen` verifies the text. It assembles every unit into one object, with one
section per unit so that relocation behaviour matches separate objects, and compares the `as`
listing against the original instruction by instruction. Sizes must match, and so must bytes,
except fields that carry a relocation. Anything that fails is emitted raw:

- `.inst 0x…` in ARM code;
- `.inst.n` or `.inst.w` in Thumb code.

Currently **115 of 84,055 instructions** are raw, in 27 files, for these reasons:

- `as` rejects Capstone's syntax, e.g. `adr` with an immediate, or some VFP/system forms;
- `as` would pick a different encoding;
- a 16-bit Thumb branch, `cbz` or `cbnz` crosses into another object, where it cannot be relocated;
- a branch targets a function of the other instruction set, where `ld` would insert an
  interworking veneer.

The last case currently happens only at `0x4006A678 → 0x4006A658`. The analyzer's Thumb
"prologue" at `0x4006A658` is a false positive inside ARM code; this is OPEN and should be fixed
in `tools/analyze.py`.

## Regenerating

```bash
make split
```

`make split` runs `make map`, then `tools/split.py gen`. **It overwrites `asm/`**, so it is
meant only for the time before functions are hand-edited or moved to `src/`. After that, edit
the files directly. `tools/whydiff.py` links without the size assert and prints the first
misplaced unit and the first differing byte when `make check` fails.

Assembler warnings such as "source register same as write-back base" reproduce encodings
from the original binary. They are expected.
