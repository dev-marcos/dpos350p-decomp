# Platform drivers (build 2025-06-19)

The C rewrites live in `nonmatching/platform/`. They are generated from
`tools/gen_platform.py` and `tools/platform_spec/*.py`, with one spec entry per function. Every
function is EQUIVALENT under tools/emu: same return value, final memory, ordered MMIO log and stub
calls. The test suite is `tools/emu/tests/platform.py`. Names are DERIVED. Addresses and register
values are STATIC-CONFIRMED.

| Group | Functions | Range | Notes |
|---|---:|---|---|
| GIC / IRQ | 14 | 0x400668D0..0x40066C80 | GIC-400 at 0x01C80000. `gic_init` (stock name `irq_disable`) fills a table of 0x9C handlers and initialises the distributor and CPU interface. `irq_register` sets the priority (prio << 3) and the enable bit. `irq_dispatch` uses IAR (+0x200C) and EOIR (+0x2010). |
| UART / console printf | 16 | 0x40066C80..0x40067134 | `dbg_printf` prints only when the bus clock gate of the debug UART is on (CCU 0x01C2006C bit 16 + port). The formatter is FNIRSI's (see below). |
| SPI | 9 | 0x40042F00..0x4004341C | Clock divider search, master mode, soft reset, PIO transfer with FIFO-level polling, chip select by TCR or GPIO. |
| CSI | 38 | 0x4006582C..0x40066118 | Allwinner `csi_reg` layout (interface/timing/format config, buffer address = address / 4, size, offset, interrupts), clock bring-up, frame-done ISR (sets SYS_INFO+0x70E). |
| SD/MMC, part 1 | 21 | 0x400675A8..0x40068900 | Base table, range check, RINTSTS wait, update-clock command, module clock (OSC24M or PLL_PERIPH0, M/N dividers, phases), PIO and IDMAC data paths, cache maintenance by address. |

## Findings

- **The formatter.** `console_format_conv` handles the flags `0` and `-`, a width, and the
  conversions `c s d i u x X o %`. Any other conversion character prints the argument **in
  base 3**. A `%` at the end of the format calls `console_print_num` with base 0. The stock
  `UDIV` returns 0 for a zero divisor, so one digit is printed. C division by zero is
  undefined, so the C code handles base 0 explicitly. In base 10, `console_print_num` inserts
  `,` every three digits.
- **Stack garbage in `csi_init_cfg`.** The fourth byte of its format descriptor is
  uninitialised stack in the stock code. The C code uses 0, which is what the test harness
  stack contains. OPEN.
- **`csi_set_fmt` stays in assembly.** This function (0x40065D04) reads uninitialised stack
  arrays, so its result depends on stack contents. OPEN.
- **SD/MMC wait timeouts.** The SD driver polls with timeouts:
  - RINTSTS: 5 µs steps; error mask 0xBFC2; returns -0x98.
  - Update-clock command: 200 000 × 1 µs.
  - PIO data: 1000 µs per 256 bytes, at least 2 s.
- **IDMAC descriptors.** Each descriptor covers 4 KiB, with at most 256 per transfer. The
  first descriptor gets FD, the last gets LD and the end-of-ring flag. The FIFO thresholds are
  0x20070008.
- **Unset register in `uart_putc`.** A port number above 2 uses an unset register for the
  base address. OPEN, and not tested.

## Not yet rewritten

The SD/MMC part 2 functions remain in assembly:
- command send, 0x40067D70 (832 bytes);
- 0x400680B0 and the command wrapper 0x4006810C;
- CSD/size decode, 0x40068134;
- block read/write: 0x400682E0, 0x4006840C, 0x4006866C, 0x40068794;
- card init, 0x40068918 (1220 bytes).
