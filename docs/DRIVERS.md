# Low-level drivers (build 2025-06-19)

C rewrites live in `nonmatching/drivers/`. Shared definitions are in `include/hw/` (V3s
addresses, the PIO port layout, prototypes). Every function listed here is EQUIVALENT under
`tools/emu`: same return value, same final memory, and the **same ordered sequence of MMIO
accesses** (address, width, value, read/write). Names are DERIVED unless noted.

## Pin map recovered from the code

| Pins | Use | Evidence |
|---|---|---|
| PE0..PE3 | FPGA bus strobes. PE0 is the clock strobe (low then high). PE2 and PE3 select write-data (PE2=1, PE3=0), command (both 1) or read (both 0) | `fpga_*` (0x400490DC–0x400492E0); Hack FPGA_PROTOCOL.md |
| PE8..PE15 | FPGA 8-bit data bus. `PE_CFG1` = `0x11111111` (out) or `0` (in); direction cached in `g_fpga_bus_is_output` (0x42E68D18) | `fpga_bus_output/input` |
| PE20 / PE21 | MS5351M bit-banged I2C, SCL / SDA (device 0xC0) | `ms5351_*` (0x4005F014–0x4005F6E4) |
| PE22 / PE24 | GT911 reset / INT (address-select reset sequence) | `gt911_reset_seq` |
| PG6 / PG7 | GT911 bit-banged I2C, SDA / SCL (device 0xBA/0xBB) | `gt911_*` (0x40049350–0x400498E0) |
| PF0..PF5, PC0..PC5, PC0..PC10 | SD/MMC controller 0 / 1 / 2 pin mux (function 2, pull-up) | `sdmmc_pins_init` |

## Function groups

| Group | Functions | Notes |
|---|---|---|
| PIO helpers | `pio_cfg_pin`, `pio_set_bit`, `pio_clr_bit`, `pio_get_bit`, `pio_set_drive` | V3s PIO port block (CFG×4, DAT, DRV×2, PUL×2), port stride 0x24 |
| Register accessors | `readl_a/b/c`, `writel_a/b/c` | one copy per source file (armcc does not inline across files) |
| FPGA parallel bus | `fpga_bus_input/output/init`, `fpga_data_byte_read/write`, `fpga_selector_command_write` | |
| Watchdog 0 | `wdt_config`, `wdt_enable`, `wdt_irq_ack`, `wdt_system_reset` | registers 0x01C20CA0..0x01C20CB8 |
| GT911 touch | start/stop/ack, byte I/O, register read/write (8/16-bit address), pin and reset sequences | `gt911_read_byte` keeps the stock 80-iteration NOP delay |
| MS5351M clock | I2C primitives, `ms5351_write_pll`, `ms5351_write_ms_int`, `ms5351_set_clk0/1` | Si5351-compatible register map, below |
| Timers / AVS / delays | `timer_setup/start/stop`, `timer0_init`, `timer0_isr` (increments `g_tick_count` 0x42E68CD8), `avs_init`, `avs_set_div`, `avs0/1_restart`, `avs0/1_read`, `delay_us`, `delay_ms`, `timers_init` | timer block 0x01C20C00; interval = µs·24 (24 MHz OSC); delays poll AVS counter 0 |
| CCU | `ccu_set_cpu_pll`, `sys_clock_init`, `periph0_freq`, `ccu_gate_on/off`, `spi0_clk_init` | below |

### MS5351M frequency planning (`ms5351_set_clk0/1`)

1. `freq` is clamped to [2500, 200 000 000] Hz.
2. Below 1.024 MHz an output R divider is chosen (register bits 0x10..0x70), and `freq` is
   scaled by 2..128.
3. The output multisynth divider is the largest even integer that keeps VCO = freq·div at or
   below 900 MHz (or 600 MHz for freq < 3.5 kHz). Above 150 MHz the code uses VCO = 4·freq,
   MS=4, R bits 0x0C.
4. The VCO is clamped to [600, 900] MHz and decomposed as VCO/25 MHz = a + b/1048575. `a`
   comes from a multiply-high by `0xABCC7712`; `b` is computed in single precision.
5. Registers written:
   - PLLA (0x1A) or PLLB (0x22), with Si5351 P1/P2/P3;
   - MS0 (0x2A) or MS1 (0x32);
   - PLL reset `0xB1` = 0x20 or 0x80;
   - CLK0 control `0x10` = 0x4F, or CLK1 control `0x11` = 0x6F.
6. Side effects: `set_clk0` stores the requested frequency in `SYS_INFO+0x5F0`; `set_clk1`
   stores the VCO frequency in `0x42E68C68`.

### Clock setup (CCU 0x01C20000)

- `ccu_set_cpu_pll(f)`: N = f / 24 MHz (multiply-high by 0xB2F4FC08). Above 768 MHz it sets the
  P bit (0x10) and halves N. AXI/APB dividers are 0x10102 before and 0x20101 after; it waits on
  LOCK (bit 28).
- `sys_clock_init(f)`: f < 1500 means MHz. It rounds down to a multiple of 24 MHz, then writes:
  - PLL_VIDEO (0x10) = 0x91004107;
  - PLL_PERIPH0 (0x28) = 0x90041811, then waits for LOCK; this gives 600 MHz;
  - AHB/APB0 (0x54) = 0x3180;
  - MBUS (0x15C) = 0x81000003.
- `ccu_gate_on/off(code)`: one word encodes a bus-gating register and bit (bits 31..21 and
  20..16) and a reset register and bit (bits 15..5 and 4..0). The function sets or clears both
  bits, then calls `delay_us(100)`.
- `spi0_clk_init(0)`: SPI0_CLK (0xA0) is set to PLL_PERIPH0 with divider 3. The function returns
  that rate, 200 MHz.

Register meanings are DERIVED from the V3s CCU map. The values are STATIC-CONFIRMED.

## Test harness additions

- `Machine.mmio` is the ordered log of every access to 0x01000000..0x01FFFFFF. It is compared
  between the stock function and the C build.
- `Machine.devices` holds models that answer reads. The GT911 tests drive PG_DAT bit 6 (SDA)
  from a seeded bit stream, so both outcomes of every ACK poll and data bit are exercised.
- The delay routines (`delay_us` 0x40042D0C, `delay_ms` 0x40042CF0) are stubs in the tests of
  their callers, and the calls and arguments are compared. Their own tests use an AVS counter
  model: AVS_CNT0/1 advance by a seeded step on every read.
- PLL wait loops use a LOCK model. Reads of PLL_CPU and PLL_PERIPH0 report bit 28 with a seeded
  probability, so both "not yet locked" and "locked" are exercised.

## FPGA start-up, SMEC jobs, touch, power, disk glue (batch 11)

C: `tools/platform_spec/f_fpga_smec.py` → `nonmatching/platform/`. Types and macros:
`include/hw/fpga.h`. Tests: `tools/emu/tests/fpga.py`. Addresses and behaviour are
STATIC-CONFIRMED from the code. Names are DERIVED unless they come from the Hack notes.

- **Shared PE pins** (`fpga_pe_mode_select`, 0x42E68D19): mode 0 sets them up as the FPGA control
  bus (`fpga_bus_init`); any other mode sets up CSI capture (`csi_pins_init_0`). The pins are only
  reprogrammed when the mode changes.
- **`FPGA_Start`**:
  1. `csi_full_init(8000)`;
  2. switches the pins to bus mode;
  3. starts the MS5351M with CLK0 at 95 MHz and CLK1 at 10 MHz (`ms5351_start_clocks`).

  `perform_fpga_identification_probe` reads selector 0x01. A5 A5 5A 5A means OK, and the result
  goes to SYS_INFO+0x7FC.
- **SMEC jobs** (`smec_run_job`). A parameter goes to the FPGA as a job, (index, 32-bit value):
  1. timing to selector 0x69 (16 bit), index to 0x6A, value to 0x6B (32 bit, MSB first);
  2. start with 0x68 = 1, wait for 0x64 bit 0, then 0x68 = 0;
  3. wait for 0x65 bit 0;
  4. the FPGA echoes the index on 0x68 and sets 0x67 bit 0 when the result is valid;
  5. the result is read from 0x66 (32 bit).

  On a wrong echo or an invalid result, the per-index timing (0x42E67558, reset to 2000) grows by
  100 up to 3000. After that come 4 more tries at 3000, then the job returns 0.

  `fpga_param_updater` caches the last value per index (state block 0x42E62C2C) and sends only
  changes. `fpga_params_push` sends SYS_INFO 0x1B8, 0x1C0, 0x268, 0x270, 0x26C, 0x08 and 0x62C as
  jobs 1..7. `smec_commit` sends a closing job (value 0) after changes, and
  `security_chip_check` uses it with job 0xB4 before reading selector 0x60.
- **Touch** (state block 0x42E68BB0, `touch_t`). `gt911_poll` runs at most every 15 ms:
  - it reads the GT911 status 0x814E (low nibble = point count) and the first point at 0x8150,
    clamped to 1022 × 600, then clears the status;
  - it latches press and release edges with their points.

  `touch_service` (UI loop) and `runtime_event_service` (acquisition loop) mark an event for the
  UI on an edge, or while held when repeat is on (0x42E61FF8). The Hack name of the second one is
  kept; the code services the touch panel.
- **Power**:
  - `auto_off_arm` sets the auto power-off deadline: tick + minutes (SYS_INFO+0x658) × 60 000,
    kept in SYS_INFO+0x65C.
  - `auto_off_check` runs when that deadline has passed. It draws the shutdown image at the screen
    centre, waits 3 s and calls `power_off`. `power_off` (0x4004A1B4) sets FPGA selector 0x03 to 1
    and waits forever for the supply to drop, so it stays in asm.
  - `battery_coeffs_default` stores the LRADC conversion, as floats: 32.947 × code + 2565.74 + 170.
  - `charge_status_read` reads selector 0x02 bit 0 into SYS_INFO+0x779.
- **FatFs disk glue** for drive 0 (the SD card): reads and writes go through the bounce buffer
  0x427EB634. Stock quirks, kept as they are:
  - `disk_read` copies the bounce buffer to the caller even when the read failed;
  - a wrong drive or an unready card makes `disk_read` return 0, and `disk_write` return 0 when
    the card is not ready;
  - `get_fattime` is fixed at 2021-11-21 12:00:00.

  `disk_mount`, `disk_remount`, `disk_format` (FAT, 16 KiB clusters) and `disk_free_bytes` wrap
  `f_mount`, `f_mkfs` and `f_getfree` on "0:".
- **Tests**:
  - A seeded FPGA model answers each selector (ready bits, index echo, valid flag, identity), so
    the retry paths run.
  - The model and the stub random streams are created in each run's setup, so the stock code and
    the C see the same answers.
  - Stubs that receive stack buffers (GT911 register access, `f_getfree`, `f_mkfs` with its stack
    argument) record contents instead of addresses.
- **Kept in asm**: `power_off`. Also `smec_job_result`'s tail at 0x4004A094, which the analyser
  split off; the C of `smec_job_result` covers the whole function.

## Not yet rewritten (next batches)

- Remaining clock users: `sys_mmu_init` and 0x400429E4 (CP15, not emulated), and 0x4003E00C..0x4003E314 (with TCON).
- (USB stack: done, see USB.md.)
- The display engine and TCON code (DE2 0x01000000, functions of up to 5 KB).
- The rest of 0x4004A2D0..0x4004AF0C (FPGA runtime helpers around the POST checks).
