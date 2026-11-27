# Assembly split (Phase 3)

`make check` rebuilds the ISO from these inputs only:

- `asm/*.s`: 1,359 unit files, committed:
  - `asm/vectors.s`: the exception vectors;
  - `asm/func_XXXXXXXX.s`: 1,354 functions;
  - `asm/data_XXXXXXXX.s`: 4 blob wrappers.
- `asm/sections/sec_XXXXXXXX.s`: 277 cluster files. Each one opens one section
  (`.text.<address of its first unit>`) and `.include`s its unit files. Only these are
  assembled: one object per cluster (see "Relocatable output" below).
- `data/*.bin`: 4 large data areas (fonts, graphics, rodata, packer garbage). These are
  **extracted from the user's ISO** at build time (`tools/split.py extract`, checked against
  the SHA-256 values in `data/blobs.csv`) and are never committed.
- `ld/dpos350p.ld` + `ld/sections.ld`: one input section per cluster (`.text.<address>`),
  placed in original address order at `0x40000000`. The script asserts that the image is exactly
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
- Every reference to something inside the image is a symbol, so that the code can move (see
  "Relocatable output"). Addresses in RAM (from the ZI base `0x400954A4` up) and in the
  peripherals stay numeric: they do not move with the code.

`tools/split.py gen` verifies the text. It assembles every unit into one object, with one
section per cluster so that relocation behaviour matches separate objects, and compares the `as`
listing against the original instruction by instruction. Sizes must match, and so must bytes,
except fields that carry a relocation. Anything that fails is emitted raw:

- `.inst 0x…` in ARM code;
- `.inst.n` or `.inst.w` in Thumb code.

Currently **107 of 84,071 instructions** are raw, in 21 files, for these reasons:

- `as` rejects Capstone's syntax, e.g. `adr` with an immediate, or some VFP/system forms;
- `as` would pick a different encoding;
- a 16-bit Thumb branch, `cbz` or `cbnz` reaches another unit; it keeps its encoded offset, so
  its target is kept in the same cluster;
- a branch targets a function of the other instruction set, where `ld` would insert an
  interworking veneer.

The last case currently happens only at `0x4006A678 → 0x4006A658`. The analyzer's Thumb
"prologue" at `0x4006A658` is a false positive inside ARM code; this is OPEN and should be fixed
in `tools/analyze.py`.

## Relocatable output (step 1 towards a build from C)

The goal is a firmware linked from C, where every function lands wherever the linker puts it.
For that, nothing in the assembly may depend on the original addresses. `tools/split.py gen`
now writes:

| Reference | Count | Written as |
|---|---:|---|
| PC-relative load (`ldr`, `ldrh`, `ldrd`, `vldr`… `[pc, #n]`) | 3,931 | `ldr r0, .Lp_XXXXXXXX` |
| `add`/`sub rX, pc, #n` | 228 | `adr rX, .Lp_XXXXXXXX` |
| pool or vector-table word holding an image address (function entries included) | 898 | `.word <symbol>` |
| `movw`/`movt` pair building an image address (libm tables) | 4 pairs | `#:lower16:sym` / `#:upper16:sym` |
| `movw`/`movt` + `add rX, pc` (+ `sub rX, rX, #k`): position-independent address (`__scatterload`, a table of `pow`) | 3 pairs | `#:lower16:(sym - . + A)`, a PC-relative relocation |
| word holding the offset from itself to a function (the armlink `Region$$Table` entry) | 1 | `.word __scatterload_zeroinit - .` |

The symbols for things that are not function entries:
- `d_XXXXXXXX` for every image address the assembly or the C refers to, inside a unit (strings
  and tables after the code) or inside a data blob (split into several `.incbin` pieces there).
  The C uses the same names (`include/image.h`), and `tools/split.py` reads them from the C
  sources so that each one has a label;
- `vectors` for `0x40000000` (copied to the vector page at reset).

What is **not** a symbol, and why:
- 8-byte literals are doubles (`vldr d`, `ldrd`): their words are never addresses, even when one
  looks like one (`0x400921FB`, the high word of pi);
- `movw #0` / `movt #0x4000` in `pow` is the double 2.0;
- the 2 pool words `0x400954A4` (read by 18 loads) are the first ZI address, the trace-intensity
  layer, in RAM;
- the data blobs hold no pointers: the image-range words found in fonts and graphics are pixels,
  and the six in rodata are flags and the ZI base.

**Clusters.** armcc shares one literal pool between the functions of a section, so 2,647 loads
read a pool in another unit. Such units cannot move apart. A cluster is a contiguous run of units
that must stay together:
- a PC-relative operand in one reaches into another;
- a function runs on into the next one (no final branch or return);
- a raw `.inst` branch reaches another unit.

There are 277 clusters (the largest has 99 units). Each is one section, so the linker can place
the clusters in any order. Within a cluster, the labels are resolved by `as`. A unit can still be
removed from its cluster (when its C replaces it) as long as the pool entries the other units use
stay.

**Test: `tools/relink_test.py` + `tools/relink_check.mjs`.**
1. The test links the same objects in another order: the vector page first, then the rodata/RW
   blob, then every other cluster in reverse order (or shuffled, with `--shuffle N`). 276 of 277
   sections move. The image keeps its size, so it still ends at the ZI base.
2. `reloc.json` maps each stock address to its new place, and the emulator moves its hooks and
   its start address with it (`--reloc`, in `tools/sim`).
3. `relink_check.mjs` boots the stock image and the moved one for 1.5 billion instructions each,
   and compares instruction count, interrupts, captured frames, final PC (mapped back) and
   framebuffer.

Result: the same for the reversed order and for shuffles 1 and 7. What it does not cover: code
the boot and the main screen never run (menus, files, USB). For that code, the static rules
above are the argument, not the test.

```bash
make check                                        # the stock layout: MATCH
python3 tools/relink_test.py [--shuffle N]        # inside dpos-re
node tools/relink_check.mjs                       # on the host (node)
```

## Mixed link: C in place of the assembly (step 3, branch `relink-tests`)

`tools/mixlink.py` (`make mixed`) links the firmware with every function that has a C file
taken from its GCC object (`build/nonmatching/func_X.o`, the object `make emutest` checks)
instead of from `asm/func_X.s`. The rest is the relocatable assembly above. The result,
`build/mix/app.bin`, is **not** the matching build (`make check` still is): the linker places
everything, and the image ends where the last section ends.

An assembly unit leaves only when nothing that stays depends on where it is:
- a branch that stays targets one of its interior labels, or an `adr` points at its code;
- a raw `.inst` branch or load that stays reaches it or crosses it (fixed offset): every unit
  between the two stays as well;
- a unit that stays runs on into it.

What stays of a unit that leaves:
- literals that the assembly around it still reads PC-relative (shared armcc pools) stay where
  the unit was, as a stub with only those words;
- data inside it that something still names (`d_XXXXXXXX`: inline strings, tables) becomes a
  small section of its own (`build/mix/sections/rem_*.s`).

Names the C uses that nothing defines resolve like `tools/nmbuild.py` does: RAM addresses (past
`0x400954A4`) as absolute symbols, image addresses as aliases of what is now at that place
(`irq_disable` = `gic_init`, …: old names in `include/stock_asm.h`). `build/mix/report.txt`
lists every unit kept in assembly and the instruction that keeps it.

Result:

| | |
|---|---|
| functions linked from C | 1,254 of 1,264 |
| kept in assembly although they have C | 10: Thumb 16-bit branches that cross them (`strlen`, `__aeabi_memset4`, USB disk) and branches into their interior |
| literal stubs | 14 units, 45 literals |
| remnant data sections | 10 (16 labels) |
| image | 582,112 bytes, ends at `0x4008E1E0`: 29,380 bytes below the ZI base |

The C must not contain NEON (`-mfpu=vfpv4` in `tools/nmbuild.py`): the stock code has none and
neither does the browser emulator. With `neon-vfpv4`, GCC vectorised 36 functions and the
first boot stopped on `vmov.i32`.

**Test: `tools/mix_check.mjs`.** The C runs a different number of instructions, so the stock
and the mixed image are compared at the same point of the acquisition: after 100, 400 and 800
captured frames, framebuffer against framebuffer. Result: the same picture at all three (main
screen with the trace and live measurements). As with the relink test, code that the boot and
the main screen never run is covered only by `make emutest`, function by function.

```bash
make mixed                                        # inside dpos-re
node tools/mix_check.mjs --png build/mix          # on the host (node)
```

## Regenerating

```bash
make split
```

`make split` runs `make map`, then `tools/split.py gen`. **It overwrites `asm/` and
`asm/sections/`**, so it is
meant only for the time before functions are hand-edited or moved to `src/`. After that, edit
the files directly. `tools/whydiff.py` links without the size assert and prints the first
misplaced unit and the first differing byte when `make check` fails.

Assembler warnings such as "source register same as write-back base" reproduce encodings
from the original binary. They are expected.

## Analyser fix: tail-called entries hidden in gaps (Phase 5, batch 7)

armcc emits entries such as `mov r0,#0` that fall into another function's store (0x4006C4CC).
The analyser used to classify these as zero padding. It now runs one more pass after the
function list converges. A direct `B` target becomes a new function entry when two conditions
hold:
- the target lies outside the caller's span [entry, next entry);
- no analysed function body covers it.

Nine entries were added. No other function changed. `make check` still matches.
