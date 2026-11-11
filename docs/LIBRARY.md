# Run-time library (Arm C library) in the stock image

Build 2025-06-19. The routines below come from the Arm Compiler C library (armlib)
linked into the application, not from FNIRSI code. The library members are scattered between
application objects. Names are DERIVED unless stated otherwise:

- by **behaviour**: emulating the stock routine and comparing its output with Python's
  `math` (`build/identify_*.py`-style probes, recorded in `symbols/manual.csv`);
- by **structure**: AEABI register contracts, call sites and constants.

Every routine marked EQUIVALENT has a C rewrite in `nonmatching/libc` or `nonmatching/libm`
that `tools/emu` verifies **bit-exactly**: return registers, VFP `d0`, and memory. The
comparison uses 400–3000 random cases per function, with half the doubles drawn as raw
64-bit patterns (NaN payloads, subnormals, values at the boundaries).

## AEABI and libc helpers (`nonmatching/libc`)

| Address | Routine | ISA | Notes | Status |
|---|---|---|---|---|
| `0x4005C31C` | `__aeabi_memcpy` | ARM | byte loop until src aligned, then `0x4005C340` or `0x4005C38C` | EQUIVALENT |
| `0x4005C340` | `__aeabi_memcpy4` | ARM | 16-byte `LDM/STM` loop + 8/4/2/1 tail | EQUIVALENT |
| `0x4005C38C` | `__memcpy_dst_unaligned` (name OPEN) | ARM | word loads, halfword/byte stores | EQUIVALENT |
| `0x4006A6FC` | `__aeabi_memset4` | ARM | stores the r2 *word* as is (no byte replication) | EQUIVALENT |
| `0x40068E5C` | `__aeabi_memclr4` | ARM | `movs r2,#0; b __aeabi_memset4` | EQUIVALENT |
| `0x4003E6F8` | `__aeabi_uidivmod` | Thumb | hardware `UDIV`/`MLS`; divisor 0 → q=0, r=n | EQUIVALENT |
| `0x40034590` | `__aeabi_uldivmod` | ARM | divisor 0: n < 2^32 → q=0, r=n; otherwise `__aeabi_ldiv0` → q=n, r=0 | EQUIVALENT |
| `0x4003453C` | `__aeabi_ldivmod` | ARM | signed wrapper; remainder takes the dividend's sign | EQUIVALENT |
| `0x4003E704` | `__aeabi_ldiv0` | ARM | `bx lr` | EQUIVALENT |
| `0x4003DF94` | `__aeabi_d2lz` | Thumb | saturates, but Inf/NaN → 0 | EQUIVALENT |
| `0x4005F6E8` | `__aeabi_d2ulz` | Thumb | negatives → 0; +Inf and positive NaN saturate | EQUIVALENT |
| `0x4006A4E8` | `__aeabi_f2lz` | Thumb | float → int64 | EQUIVALENT |
| `0x4003A5D8` | `__aeabi_l2d` | Thumb | round to nearest even | EQUIVALENT |
| `0x4005F730` | `__aeabi_ul2d` | Thumb | round to nearest even | EQUIVALENT |
| `0x4006A4DA` | `llabs` | Thumb | | EQUIVALENT |

The 64-bit divisions return quotient and remainder in r0–r3. The C rewrite computes them in a
core and returns them through a small naked shim (`include/rt/ldiv.h`).

## libm (`nonmatching/libm`, hard-float: arguments and results in `d0`/`d1`)

| Address | Routine | Implementation style | Status |
|---|---|---|---|
| `0x4003DF74` | `sqrt` | `VSQRT`; for x < 0 the high word becomes `0x7FFFFFFF` and x's low word is kept | EQUIVALENT |
| `0x40069F54` | `frexp` | wrapper around the soft-ABI core `0x40069F6C` (result in r0:r1 + r2) | EQUIVALENT (core inlined) |
| `0x4006B86C` | `ldexp` | exponent arithmetic + `VMUL` by powers of two. Its subnormal-input path for n ≥ 0x3FC scales by 2^(n−7) (reproduced) | EQUIVALENT |
| `0x40068F08` | `exp` | hand-written assembly, table at `0x4008C898`; shared kernel `0x4006A3F8` | EQUIVALENT |
| `0x4006A31C` | `log` | hand-written assembly, table at `0x4008C7F0` | EQUIVALENT |
| `0x4005DAD0` | `log10` | same reduction as `log`, split 1/ln10 and log10(2) | EQUIVALENT |
| `0x4005F91C` | `tanh` | uses the exp kernel (`0x4006A400` entry) for \|x\| ≥ ln2 | EQUIVALENT |
| `0x4006613C` | `__sincos_core` | compiled C (MOVW/MOVT constants); Cody-Waite reduction with π/2 in 5 parts | EQUIVALENT |
| `0x40066530` / `0x40066534` | `cos` / `sin` | `movs r0,#1` / `#0`; `b __sincos_core` | EQUIVALENT |
| `0x4005D4A4` | `pow` | 1,580 bytes; calls `frexp` and `__pow_core` | **ASM** (next) |
| `0x40069FD0` | `__pow_core` | 842 bytes; exp-like, calls `ldexp` | **ASM** (next) |
| `0x4006A400` | `__exp_kernel` entry | register interface (d0–d7, r0, r2); covered through `exp` and `tanh` | ASM (no C entry) |
| `0x40069F6C` | `__frexp_core` | 3-register return; covered through `frexp` | ASM |

Notes for bit-exactness:

- `VMLA`/`VMLS` are not fused, so `nonmatching` is built with `-ffp-contract=off`.
  VFPv4 would otherwise let GCC emit `VFMA`.
- Plain `char` is signed in the stock build, so the builds use `-fsigned-char` (see
  `docs/COMPILER.md` #10).
- The harness sets the stock FPU state (`vfp_enable`: FPSCR = `0x02000000`, default NaN).
