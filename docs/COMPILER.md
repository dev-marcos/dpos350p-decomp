# Original compiler identification (Phase 4)

**Status: GCC ruled out; Arm Compiler 5 (armcc 5.x + armlink) is the leading candidate and is
not yet tested** (it is licensed software, so installation waits for approval).

Evidence levels: HW-CONFIRMED / STATIC-CONFIRMED / DERIVED / OPEN.

## 1. Fingerprints in the binary

| # | Observation | Where | Points to | Level |
|---|---|---|---|---|
| 1 | Startup through `__scatterload`, a `Region$$Table` with relative handler offsets, `__scatterload_zeroinit`, and position-independent `movw/movt + add pc` | `0x4008CBEC`, `0x4008D9F0`, `0x4008CF80` | **armlink** (Arm Compiler 5/6 C library). GNU ld/newlib startup looks nothing like this | STATIC-CONFIRMED |
| 2 | `_sys_exit` via semihosting `SVC 0xAB`, r0=0x18, r1=0x20026 | `0x4008CC28` | Arm C library `__rt_exit` path | STATIC-CONFIRMED |
| 3 | 60 jump tables of the form `cmp rX,#N; addls pc,pc,rX,lsl #2; b default; b case…`. Zero GCC-style `ldrls pc,[pc,rX,lsl #2]` + `.word` tables | code area | **armcc** (GCC never emits a table of `b` instructions) | STATIC-CONFIRMED |
| 4 | One literal pool shared by several consecutive functions, emitted only at the end of a run of functions | e.g. pool `0x4001A6B8` used by `0x40019AF4`, `0x40019B8C`, `0x40019C7C` | armcc without `--split_sections` (one code section per source file). GCC gives each function its own pool | STATIC-CONFIRMED |
| 5 | A single saved register handled as `stmdb sp!,{lr}; sub sp,sp,#4 … add sp,sp,#4; ldm sp!,{lr}` to keep the stack 8-byte aligned | e.g. `0x4003E708` | armcc (GCC uses `push {r4, lr}` or `str lr,[sp,#-4]!` + `sub`) | STATIC-CONFIRMED |
| 6 | Small static helpers are called, not inlined (`ld_word`, `mem_cmp`, `move_window` stay separate inside FatFs) | FatFs area | armcc at `-O1`/`-O2` without cross-function inlining, or `-Ospace`. OPEN which | DERIVED |
| 7 | Runtime: `__aeabi_memcpy`/`memset`/`memclr` and `__aeabi_{u,}ldivmod` in **ARM** state (shaped like armcc `rt_memcpy`); `__aeabi_uidivmod` uses `UDIV`/`MLS`; fp conversions and libm in **Thumb-2** with VFPv4 | `0x4005C340`, `0x4006A6FC`, `0x40034590`, `0x4003E6F8`, … | Arm C library (armlib) built for v7-A. Newlib in this GCC toolchain is a different Thumb-2 implementation (`memcpy` 1,248 B vs the original 76 B) | DERIVED |
| 8 | libm takes and returns doubles in `d0`/`d1` | `sqrt`, `pow`, … | hard-float AAPCS-VFP (`--fpu=vfpv4 --apcs=/hardfp` in armcc terms) | DERIVED |
| 9 | Application code is ARM; interworking uses `blx imm`; there is a Thumb `ldr.w pc` veneer | | `--arm` for the app, plus an interworking library | STATIC-CONFIRMED |

## 2. FatFs version: R0.13b

The method follows the plan: identify the open-source FatFs, then compile its exact source with
each candidate compiler.

| Evidence | Value | Conclusion |
|---|---|---|
| `check_fs` at `0x40059454` (196 B) | checks exFAT first with `mem_cmp(win, "\xEB\x76\x90" "EXFAT   ", 11)`, then JmpBoot ∈ {0xE9, 0xEB, 0xE8}, then `mem_cmp(…,"FAT",3)` / `mem_cmp(…,"FAT32",5)` | only R0.13b and R0.13c have exactly this body (R0.13/R0.13a differ; R0.14 changed the logic) |
| `f_mkfs` at `0x4005B1F8` (4,352 B) | 5 arguments `(path, opt, au, work, len)`, `sz_blk` must be ≤ 32768 | R0.13 family; R0.14 takes `MKFS_PARM*` |
| `FATFS` layout used by `check_fs`/`move_window` | `wflag` +3, `winsect` +0x34, `win` +0x38 | R0.13b with LFN + exFAT (+`lfnbuf`, +`dirbuf`) gives exactly 0x34/0x38. R0.13c adds `bitbase` under exFAT, which would push `winsect` to 0x38 |
| Strings | `"\xEB\x76\x90EXFAT   "`, `"\xEB\xFE\x90MSDOS5.0"`, `"NO NAME    FAT32   "`, `"NO NAME    FAT     "` | consistent with R0.13b `f_mkfs` |

**FatFs R0.13b (April 2018)** – DERIVED. Configuration known so far: `FF_FS_EXFAT=1`,
`FF_USE_MKFS=1`, `FF_USE_LFN≥1`, `FF_MAX_SS=FF_MIN_SS=512`, `FF_FS_RPATH=0`, `FF_FS_REENTRANT=0`,
`FF_FS_READONLY=0`. Code page and LFN storage mode are OPEN.

Located so far: `ld_word` `0x40056A70`, `mem_cmp` `0x40056BBC`, `move_window` `0x40056D80`,
`get_ldnumber` `0x400593DC`, `check_fs` `0x40059454`, `f_mkfs` `0x4005B1F8`. All DERIVED,
identified through call structure and constants.

## 3. GCC results (ruled out)

`tools/compiler_probe/probe_gcc.py` compiles R0.13b `ff.c` (config in
`tools/compiler_probe/fatfs/`) with `arm-none-eabi-gcc` **15.2.1** (Arm GNU Toolchain 15.2.Rel1).
Flags were `-marm -mcpu=cortex-a7 -mfpu=neon-vfpv4 -ffunction-sections`, crossed with
{`-O0`,`-O1`,`-O2`,`-Os`,`-O3`} × {`-mfloat-abi=hard`,`soft`} × {default, `-fno-inline`}.

Similarity is the difflib ratio over normalised instruction sequences (`tools/cmpfunc.py`).
Branch targets and pool offsets are abstracted out. 1.0 means identical modulo layout.

| function | original size | best GCC similarity | best flags | exact |
|---|---:|---:|---|---|
| `ld_word` | 16 | 0.50 | `-O1 -fno-inline` | no |
| `mem_cmp` | 36 | 0.13 | `-O0` | no |
| `move_window` | 88 | 0.56 | `-Os` | no |
| `get_ldnumber` | 120 | 0.20 | `-Os` | no |
| `check_fs` | 196 | 0.33 | `-O1 -fno-inline` | no |
| `f_mkfs` | 4,352 | 0.08 | `-O1 -fno-inline` | no |

The differences are structural, not just register allocation. For example, the original
`mem_cmp` is a 9-instruction loop (`ldrb …,#1; subs; ldrb; sub; beq; cmp; beq; bx lr`), while
GCC produces 15 instructions with a different shape. `move_window` differs in the order in
which the `winsect` comparison and the result variable are set up. Together with fingerprints
1–5, which no GCC version produces, older GCC releases (4.x–13.x) cannot match either: the
startup, the jump tables and the shared pools are toolchain features, not optimisation
choices. **Testing more GCC versions is therefore not recommended** (see §5).

## 4. libc helpers

| helper | original | newlib (GCC 15.2 multilib `thumb/v7ve+simd/hard`) |
|---|---|---|
| `memcpy` | `__aeabi_memcpy` at `0x4005C340`, ARM, 76 B, `ldm r1!,{r3,r4,r5,ip}` / `stm r0!` 16-byte loop | `memcpy`, Thumb-2, 1,248 B |
| `memset` | `__aeabi_memset` at `0x4006A6FC`, ARM, 56 B, `stmhs r0!,{r2,r3,ip,lr}`, plus `__aeabi_memclr` = `movs r2,#0; b` | `memset`, Thumb-2, 376 B |
| `strlen` | not yet located | Thumb-2, 216 B |

## 5. Next step: Arm Compiler 5 (needs approval)

The candidate is **Arm Compiler 5.06** (armcc/armasm/armlink). update 7 (build 960) is the
last release, and it is what recent Keil MDK v5 projects for Allwinner F1C/V3s targets usually
use. The test is to compile R0.13b `ff.c` with combinations of:

- `--cpu=Cortex-A7 --arm --apcs=/hardfp --fpu=VFPv4`;
- each of `-O0`/`-O1`/`-O2`/`-O3`, with `-Ospace` and with `-Otime`;
- with and without `--split_sections`.

Each object is then compared with `tools/cmpfunc.py`. The acceptance target is one FatFs
function that matches byte for byte.

What I cannot do and you would need to do:

- Downloading AC5 needs an Arm account login and acceptance of the EULA.
- Running it needs a licence: a Keil MDK licence, the free MDK-Lite (32 KB link limit, which
  does not matter for compiling single objects), or an evaluation licence.

Proposed layout, everything inside the project:

| Item | Path | Size (approx.) |
|---|---|---|
| AC5.06u7 installer, downloaded by you | `tools/downloads/armcc/` | ~120 MB |
| AC5 install (Windows build; can be called from WSL by path) | `tools/compilers/armcc-5.06u7/` | ~350–450 MB |
| optional: Keil MDK-Lite, only if its licence mechanism is the one used | `tools/keil/` | ~1–1.5 GB |
