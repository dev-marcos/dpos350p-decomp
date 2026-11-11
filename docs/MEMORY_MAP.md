# Memory map: application image (build 2025-06-19)

`app.bin` = `iso[0x51020 : 0xE6800]`, 612,320 bytes (`0x957E0`), SHA-256
`8eeb56ba00865d7bacde2d56c006c57cc8284d22bd26b24fcf0e3cf4defbca19`. The boot chain loads it
at, and enters it at, `0x40000000` (DDR2, 64 MiB). All addresses below apply to this build
only.

Evidence levels: **HW-CONFIRMED / STATIC-CONFIRMED / DERIVED / OPEN** (same convention as
the Hack repo).

Regenerate:

```bash
make map
```

`make map` runs `tools/import_refs.py`, `tools/analyze.py` and `tools/mkmap.py`, and produces
`symbols/symbols_2025-06-19.csv` and `build/analysis/map.json`.

## 1. Image layout

| Range | Size | Content | Level |
|---|---:|---|---|
| `0x40000000–0x4000001F` | 32 | 8 × `ldr pc,[pc,#0x18]` exception vectors | STATIC-CONFIRMED |
| `0x40000020–0x4000003F` | 32 | vector addresses: reset `0x4008CB28`, undef `…CCE4`, swi `…CCE8`, pabort `…CCEC`, dabort `…CCF0`, reserved `0`, irq `…CCF4`, fiq `…CD2C` | STATIC-CONFIRMED |
| `0x40000040–0x40019AF3` | 105,140 | bitmap fonts (upstream). No code. Internal structure not yet decoded | DERIVED (area), OPEN (structure) |
| `0x40019AF4–0x4006DDEB` | 344,824 | **main code**: 1,305 ARM functions + 25 Thumb runtime/libm functions, with their literal pools | DERIVED |
| `0x4006DDEC–0x4008CB27` | 126,268 | graphics/bitmaps (upstream). Starts with a small parameter table (`04 6B 00 12 64 00 …`) | DERIVED (area), OPEN (structure) |
| `0x4008CB28–0x4008CFB7` | 1,168 | **startup and runtime**: reset, VFP enable, `__scatterload`, exit, vector handlers, zero-init, plus a few strings in between | STATIC-CONFIRMED / DERIVED |
| `0x4008CFB8–0x400954A3` | 34,028 | read-only data: UI strings (EN/PT/RU, UTF-8), USB/SD/FatFs messages, region table (`0x4008D9F0`), tables, GB2312 font, and at the end a small RW area | DERIVED |
| `0x400954A4–0x400957DF` | 828 | **not part of the program**: lies past the ZI base and is zeroed at boot. The bytes are x86 Windows stack residue (`0x00F6FAxx` stack addresses, `0x77xxxxxx` ntdll return addresses), i.e. an uninitialised buffer in FNIRSI's packer | DERIVED |

**Byte classification: 100% of the image** (from `build/analysis/coverage.txt`):

| area | function/code | literal pool | data | padding |
|---|---:|---:|---:|---:|
| vectors | 32 | 0 | 32 | 0 |
| fonts | 0 | 0 | 105,120 | 20 |
| code | 337,044 | 7,288 | 431 | 61 |
| graphics | 0 | 0 | 93,822 | 32,446 |
| startup | 400 | 24 | 740 | 4 |
| rodata | 0 | 0 | 29,580 | 4,448 |
| packer_garbage | 0 | 0 | 0 | 828 |
| **total** | **337,476** | **7,312** | **229,725** | **37,807** |

Evidence by bytes: STATIC-CONFIRMED 37,395 · DERIVED 573,454 · **OPEN 1,471**. The OPEN bytes
are short unclassified runs inside the code area, mostly inline strings such as `"ON"`,
`"OFF"`, `"MAX"` and `"MIN"`, and a few possible leaf functions that the heuristics rejected.

"Padding" means runs of 16 or more zero bytes outside code. Shorter zero runs between data
bytes are counted as data. Function extents include the bytes that the function's own control
flow jumps over (235 holes, 2,674 bytes). These are armcc inline constants and strings, kept
inside the function so that a byte-exact split stays trivial.

## 2. Startup sequence and RAM layout

`Reset_Handler` (`0x4008CB28`, ARM):

1. `cpsid i`, then sets up the stacks:
   - IRQ: `sp = 0x43068D28`
   - SYS: `sp = 0x42EE8D28`
   - SVC (stays in SVC): `sp = 0x42FE8D28`
2. Clears SCTLR.V, then copies the 64-byte vector page from `0x40000000` to `0x00000000`.
   If SCTLR.V was set, it copies to `0xFFFF0000` instead (high vectors).
3. `vfp_enable` (`0x4008CB94`): CPACR grants cp10/cp11 full access, FPEXC.EN = 1,
   FPSCR = `0x02000000` (default-NaN on, flush-to-zero off).
4. `__scatterload` (`0x4008CBEC`, **Thumb**) walks the region table at `0x4008D9F0`–`0x4008DA00`.
   The table has exactly **one** entry: handler `__scatterload_zeroinit` (`0x4008CF80`,
   Thumb) with the list `{size 0x02DD3880, addr 0x400954A4}, 0`.
5. `bl main` (`0x40034700`). If `main` returns, the code runs `exit` → `__rt_exit` →
   `_sys_exit`, which does a semihosting `SVC 0xAB` and then `b .`.

So the original linker produced a **single load region whose load address equals its
execution address**. There is no copy-down of RW data; RW lives in the image just before the
ZI base:

| Section (armlink names) | Range | Level |
|---|---|---|
| ER_RO (code + RO data) | `0x40000000` – start of RW | DERIVED |
| ER_RW | starts somewhere in `0x400950B4`–`0x40095340`, ends at `0x400954A4`. The lowest image address that code stores to is `0x40095340` | OPEN (exact start) |
| ER_ZI (.bss) | `0x400954A4`–`0x42E68D24` (`0x2DD3880`, about 45.8 MiB) | STATIC-CONFIRMED |
| SYS stack | `0x42E68D28`–`0x42EE8D28` (512 KiB) | DERIVED |
| SVC stack | `0x42EE8D28`–`0x42FE8D28` (1 MiB) | DERIVED |
| IRQ stack | `0x42FE8D28`–`0x43068D28` (512 KiB) | DERIVED |
| framebuffers | `0x43000000` / `0x43200000` (upstream; they overlap the top of the IRQ stack range, OPEN) | DERIVED (upstream) |

The stack sizes are inferred from consecutive stack tops; each stack's base is assumed to be
the previous stack's top. This matches an armcc `ARM_LIB_STACK`/startup-file layout.

## 3. Instruction sets and toolchain fingerprints (input for Phase 4)

| Observation | Value | Level |
|---|---|---|
| Main code | ARM (A32), `push {…, lr}` / `pop {…, pc}` prologues, `stmdb sp!,{lr}` for single-register saves | STATIC-CONFIRMED |
| Runtime/libm | Thumb-2 with VFPv4 double precision (`vsqrt.f64`, `vmla.f64`, `vldmia`) | STATIC-CONFIRMED |
| Interworking | `blx imm` from ARM into Thumb library code; a Thumb `ldr.w pc` long-branch veneer | STATIC-CONFIRMED |
| Startup | armlink `__scatterload` + `Region$$Table` + `__scatterload_zeroinit`; semihosting `_sys_exit` | DERIVED → **ARM Compiler (armcc/armclang + armlink)** |
| Jump tables | 60 × `cmp rX,#N; addls pc,pc,rX,lsl #2; b default; b case…`. No GCC-style `ldrls pc,[pc,rX,lsl #2]` tables | STATIC-CONFIRMED |
| Literal pools | shared by several consecutive functions (one pool per section, e.g. `0x4001A6B8` serves `0x40019AF4`, `0x40019B8C`, `0x40019C7C`) | STATIC-CONFIRMED |
| Library | `__aeabi_memcpy`/`memset`/`memclr`, `__aeabi_{u,}ldivmod` in ARM; `__aeabi_uidivmod` uses hardware `UDIV`; fp conversions and libm in Thumb | DERIVED |
| Float ABI | libm takes and returns values in `d0`/`d1` (hard-float) | DERIVED |
| FatFs | strings `"NO NAME    FAT32   "`, `"EXFAT   "`, `"sd_nand_fatfs_check"` → FatFs with exFAT (R0.12 or later) | DERIVED |

## 4. Functions

1,345 functions: 1,315 ARM and 30 Thumb. Where each entry was found:

| Origin | Count | Meaning |
|---|---:|---|
| `rea` | 1,072 | REA/Ghidra 12.1.4 procedure inventory (`evidence/phase2/rea-list_procedures.json`) |
| `prologue` | 139 | uncovered `push {…,lr}` in a code span (ARM, or Thumb that is not shadowed by an ARM `push`/`pop` word) |
| `gap-code` | 124 | first byte of an uncovered gap that decodes as a straight-line run ending in a return or branch (ARM: 3 unconditional instructions first; Thumb: a 32-bit or `push` start) |
| `vector` | 5 | exception handler entries |
| `pointer` | 3 | a word in the image points at uncovered code with a prologue, or at a short leaf |
| `chunk` | 2 | detached code with a prologue reached only by a tail branch |

Function boundaries are DERIVED: each body is the code reachable from the entry, up to the next
entry. Known residual issues:

- 45 functions reach code past the next entry: shared tails, or an entry that sits inside a
  function.
- 35 functions fall through into the next entry.
- One `add pc` jump table at `0x4003F71C` has no bound that the analyzer can see.

All of these are listed in `build/analysis/map.json` (`notes`) and are checked again by the
splitter in Phase 3.

Names: 105 functions have names.

| Source | How applied | Level |
|---|---|---|
| `symbols/manual.csv` | curated, identified in this project (startup, vectors, AEABI helpers, libm, `main`) | as given per row |
| upstream | harvested from the Hack/Boot docs by `tools/import_refs.py` into `symbols/upstream_refs.csv`; applied only when the address is a function entry | DERIVED; the source `file:line` is in the CSV |
| everything else | `func_XXXXXXXX` | |

## 5. OPEN items

- Exact start of ER_RW.
- Internal structure of the font area, the graphics area and the rodata tables (Phase 6).
- About 1.5 KB of OPEN data inside the code area: inline strings, plus possible leaf functions
  reached only through computed addresses.
- Whether `0x40066530` is cos and `0x40066534` is sin (or the reverse), and the exact AEABI
  identity of the three `*_like` conversion helpers.
- Whether the framebuffers really overlap the IRQ stack range.
