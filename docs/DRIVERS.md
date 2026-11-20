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

## Battery monitor, front-end set-up, lane levels, POST multiplex check (batch 12)

C: `tools/platform_spec/g_frontend_power.py`. Tests: `tools/emu/tests/fpga.py` (second part).

- **Battery**:
  - `battery_service` (UI loop) reads the charger status and sums LRADC readings.
  - About once a second (at least 1001 ms and 3 readings), it averages them.
  - `battery_level_update` converts the average with the float coefficients:
    mV = code × 32.947 + 2565.74 + 170, truncated. It then sets level 0..10 in SYS_INFO+0x778. The
    thresholds are 4150, 4080, 3970, 3900, 3840, 3790, 3760, 3730, 3690 and 3560 mV. The code also
    tests 3620, which changes nothing.
  - Level 0: `battery_empty_check` draws the empty-battery image, waits 3 s and powers off.
- **Vertical scale** (`vscale_update`):
  - The u64 table at SYS_INFO+0xEF0 (`rate_table_init`) holds µV per division: 20 000 000 down
    to 2 000, in 1-2-5 steps, indexed by the V/div index.
  - It is multiplied by the probe factor: SYS_INFO+0x40 / +0x104 = 0, 1, 2 for ×1, ×10, ×100.
    This gives SYS_INFO+0x50 (CH1) and +0x110 (CH2).
  - The 1x/10x/100x setting therefore only scales the displayed values. It does not change the
    hardware.
  - `vdiv_label` turns those values into the "200mV" / "2V" / "1KV" text at SYS_INFO+0x70 /
    +0x130. Below 1000 µV the stock code uses the r2/r3 it was called with; the C takes them as
    parameters.
- **Positions**: `offset_clamp` keeps SYS_INFO+0x60/+0x120 within [m, height − m], with
  m = SYS_INFO+0x1AE (doubled when SYS_INFO+0x2AA is set).
- **Front end to the FPGA**:
  - CH1 (`ch1_frontend_apply`): the V/div index goes to selector 0x28 when it changes, with index
    12 sent as 11. The calibration aux values (object+0, +8) go out as jobs 8 and 10, and the
    position as job 12. Then come commit 0xB5 and the 16-bit selector 0x2A (SYS_INFO+0x44).
  - CH2 (`ch2_frontend_apply`): selector 0x2B, jobs 9, 11 and 13, commit 0xB6, selector 0x2D.
  - Other selectors: 0x2E is the channel mode (SYS_INFO+0x100, 50 ms settle). 0x62, 0x2F, 0x29,
    0x30 and 0x2C send single SYS_INFO bytes whose meaning is OPEN.
  - `frontend_apply_all` runs the whole sequence.
- **Lane levels**: `lane_level_sum`, `lane_level_mean` and `channel_level_mean` sum
  sample × q − base over one lane's raw samples, counting negative terms as 0. Here
  q = gain[V/div] × SYS_INFO+0x26C / 1000 and base = q × SYS_INFO+0x1B8. When asked, they first
  re-arm an acquisition. Stock quirks:
  - for ch > 1 the lane number is used as the gain;
  - the borrow uses base >> 31 (logical). This is unreachable, because base < 2³¹.
- **POST channel multiplex** (`post_channel_mux_check`, 0x4004AF0C):
  1. both channels on, positions 0xAA, then 0x1E;
  2. each time, the measured mean position (SYS_INFO+0xB34 / +0xB50) must be within ±3, or ±6 at
     V/div index 12;
  3. it returns 1 (both), −2 (CH1 only), −1 (CH2 only) or 0.

## Signal generator, calibration selector, system calibration (batch 13)

C: `tools/platform_spec/h_generator.py`. Tests: `tools/emu/tests/fpga.py` (third part).

- **Generator state**:
  - The block G = 0x42E3F984 holds the sample table at G+0xE (2000 u16), a second table at
    G+0xFB6, and the loaded samples at G+0x1F56. The count is at G+0x2EF8 and the period length at
    G+0x2EF6.
  - The settings block G+0x2EFC (0x42E42880) is the one the emulator reads for the GEN menu:
    frequency, amplitude, offset, duty, limits and float scales. The waveform byte is at G+0x2EFA.
- **`Gen_Init`** (Hack name):
  1. reads the generator files (0x40036B00 / 0x40036BDC);
  2. applies the value at 0x4091F396;
  3. loads the samples (`gen_load_samples`, from SYS_INFO+0x3DA548);
  4. falls through into `gen_update`.
- **`gen_update`** runs, in order: 0x4004EEEC, `gen_limits`, `gen_send_params`, 0x4004D880 (the
  big table builder, not rewritten yet), 0x4004ED18, then `gen_table_repeat_default`.
- **`gen_limits`** clamps the settings:
  - amplitude to [0, max];
  - offset to ±limit, and offset ± amplitude/2 into the output range;
  - waveform index;
  - duty to [1, max];
  - frequency to the waveform's maximum: 50 MHz (Sine, Ringing), 10 MHz (Square, Half, Full),
    5 MHz (Triangular, Sawtooth1/2, Step, Multi-sin, SinC, DC), 3 MHz (Exponent, Sqrt, Capture).
- **`gen_send_params`** sends the FPGA parameters as SMEC jobs 0x14..0x18, then commits 0xB7.
  These are the Hack "Gen_Init owns SMEC IDs 0x14..0x18":
  - 0x14: offset;
  - 0x15, 0x16, 0x18: the floats at +0x70, +0x68, +0x6C, sent as both raw word and integer;
  - 0x17: a step value derived from the amplitude through the float scale at +0x50 and the
    divisors at +0x5C / +0x54.

  For a sine at 1 MHz or above, the amplitude is first corrected by the table at 0x400951E8, one
  entry per MHz above the reference frequency: 4180, 4620, 4700, 4580, …. This is the output's
  frequency-response compensation. Finally it sets the period length: +0x60 / frequency, capped by
  +0x38.
- **Table building**:
  - `gen_normalize` / `gen_normalize2` copy the loaded samples, invert them (max − v) and scale
    them in double precision to the output span.
  - `gen_resample` / `gen_resample2` compute a step of (u64)(period × 1.02) × SYS_INFO+0x554 × m /
    total. They decimate by m, resample with the nearest-neighbour mapper into the work buffer
    0x41648008, copy back and clamp.
  - `gen_table_repeat` inverts one period and repeats it over the table.
- **Calibration**:
  - `retained_inside_current_sc1_3_t6` (Hack working name) is the coefficient selector described
    in docs/SIM.md.
  - `system_calibration` is the factory "System calibration":
    1. clears the calibration object;
    2. calibrates each row (0x4004BFC8, not rewritten);
    3. fills the aux arrays above the calibrated rows with the last value;
    4. runs 0x4004B0A0;
    5. runs the channel-multiplex check for V/div 0..10, and saves the calibration only when all
       11 pass.
- **Quirks kept**: `gen_limits` (wave index above 14) and `gen_update` pass on whatever r0..r3
  held. `gen_table_repeat_default` (0x4004D094) is the real entry of `gen_table_repeat`; the
  analyser split it off. `__aeabi_d2ulz` is Thumb with the base AAPCS, so the C calls it through
  its Thumb address.

## Acquisition control (batch 14)

C: `tools/platform_spec/i_acq_control.py`. Tests: `tools/emu/tests/fpga.py` (fourth part).

- **State backup / restore** (all STATIC-CONFIRMED copies through `memcpy`):

  | Block | Size | Copy at |
  |---|---|---|
  | SYS_INFO 0x40544E68 | 0x3DD588 | 0x409223F0 |
  | generator block G 0x42E3F984 | 0x2F70 | 0x42E428F4 |
  | 0x42E180C8 | 0xAD20 | 0x42E22DE8 |
  | 0x42E68668 | 0x198 | 0x42E68800 |

  `state_backup_all` (0x4003F0C0) saves all four and `state_restore_all` (0x4003F0E0) restores
  them. `system_calibration` brackets the factory calibration with this pair. Restoring SYS_INFO
  also runs 0x4003EF1C. What the third and fourth blocks hold is OPEN.
- **FPGA acquisition selectors** (bus primitives from batch 8):
  - 0x08: written 0 then 1 (re-arm);
  - 0x09: 16-bit value from SYS_INFO+0x6F0, high byte first;
  - 0x0A: read, bit 0 → SYS_INFO+0x38 (acquisition done);
  - 0x0B: read, bit 0 → SYS_INFO+0x6EE;
  - 0x0E: 32-bit sample count minus 2, MSB first;
  - 0x1F: one byte;
  - 0x20 + strobe 0x21 (0 then 1): 16-bit trigger position;
  - `acquisition_prepare` (Hack name): 0x0F=0, 0x01 pulse 1→0, 0x11 pulse 0→1,
    0x02 ← SYS_INFO+0xBB3, 0x0F=1.
- **Trigger arithmetic** (DERIVED meaning; the arithmetic itself is STATIC-CONFIRMED). With
  L = SYS_INFO+0x520 (record length) and k = (SYS+0x570 / SYS+0x590 / SYS+0x564) × SYS+0x560 ×
  SYS+0x558 / SYS+0x1B1 (signed 64-bit divisions):
  - `trigger_offsets_update`: a = SYS+0x2BC/2 + k; SYS+0x288 = a mod L, SYS+0x28C =
    (SYS+0x2BC − a) mod L;
  - `consumer_translator_of_the_16_bit` (Hack name): the delay SYS+0x63C = SYS+0x290 −
    SYS+0x2BC/2 − k, raised by whole records until it is ≥ 1, or 1 when SYS+0x274/0x275 is 0.
    It then sends the trigger position and sets SYS+0x2A0;
  - `trigger_pos_update`: position = (L + SYS+0x63C − SYS+0x6F2) mod L, sent via 0x20/0x21.
- **`acq_wait_done`** polls selector 0x0A. After 2500 polls it also runs `runtime_event_service`
  on each poll, and gives up as soon as a touch event sets the touch block's busy flag.
- **`amplitude_statistical_normalization`** (Hack name; the code classifies, it does not
  normalise) writes a class per channel to SYS+0x51C / 0x51D:
  - 0 when the 64-bit sample span is below 22,000,000 / rate × SYS+0x1B1 (rate = SYS+0x524),
    or when the amplitude / SYS+0x1C0 is below 25;
  - 2 when the span reaches 250,000,000 / rate × SYS+0x1B1 and SYS+0x55C × scale / SYS+0x554
    is at least 23;
  - 1 otherwise.
- **Fix to batch 12**: `lane_level_sum` passes the acquisition length n × 12 / 10 in r0 to
  `fpga_sample_count_write`. The batch 12 C called it with no argument, and its stub did not record
  r0. The C and the stub now pass and check it.
- **Not rewritten here**: the time-base switch at 0x4003F160..0x4003F868 (one function split into
  fragments by the analyser) and the fragment at 0x4003FFE4; `final_per_channel_normalization`
  (0x4003FE14).

## Settings serialiser, code page, strlen, USB veneers (batch 15)

C: `tools/platform_spec/j_serializer.py`. Tests: `tools/emu/tests/fpga.py` (fifth part).

- **Serialiser** (Hack names). It encodes or decodes fields into a big-endian image at
  SYS_INFO+0x360404 (0x408A526C):
  - the cursor is at 0x4091F38C (SYS_INFO+0x3DA524);
  - the mode byte at 0x4091F390 is 0 for encode and 1 for decode;
  - `serializer_process_field(p, size)` picks encode or decode by the mode, then advances the
    cursor;
  - sizes 1, 2, 4 and 8 are handled; any other size does nothing but still advances the cursor.

  The encoder re-reads the source for every byte, and the decoder stores each partial result.
  The C keeps both through `volatile`.
- **FatFs code page** (ffunicode.c, CP437 only; table of the upper half at 0x4008C50C):
  - `ff_uni2oem`: an unmatched character gives 0, because (0x80 + 0x80) & 0xFF = 0;
  - `ff_oem2uni`: the comparison with 0x80 is signed, and the value is returned untruncated;
  - `ff_wtoupper`: the R0.13b algorithm with tables at 0x400877AC and 0x4008C6E0.
- **`strlen`**: the stock code scans a word at a time once aligned; the result is the same.
- **USB mass-storage veneers** `usb_mass_disk_read/write`. When the SD state (0x40095490) is 1
  they call 0x4006A738 / 0x4006A868, otherwise they return their first argument. Stock quirk
  kept: the callee also receives r3 = 0x40095490.
- **USB mode switches**:
  - `usb_mode_enter` runs 0x4006A98C, then `usb_device_init(4)`;
  - `usb_mode_exit` runs `usb_device_exit`, then `disk_remount`.
- **`lradc_data_read`** (was `func_40068e74`) returns LRADC data register 0 (0x01C2280C) & 0x3F.
  The battery service reads it. The block address is from the V3s manual.
- **Touch helpers**:
  - `touch_drag_delta`: the second point minus the first, divided by 10 when SYS_INFO+0x671 is
    set, stored at 0x42E68998+0x30/+0x34;
  - `touch_moved_flag`: 0x42E68998 = 0x12 when the second point is not (0, 0), else 0.
- **Skipped**: 0x40066FB8..0x40067078 are fragments of a split formatting function (they pop a
  frame they never pushed), so they wait until that function is merged.

## Touch controls, complex helpers, LRADC, strcmp (batch 16)

C: `tools/platform_spec/k_touch_misc.py`. Tests: `tools/emu/tests/fpga.py` (sixth part).

- **Touch-control state** at 0x42E68998 (DERIVED from use). Byte +0 holds the control code of
  the current touch. The words hold distances and limits:

  | Offset | Meaning |
  |---|---|
  | +0x04 | limit for the cursors |
  | +0x30 / +0x34 | drag dx / dy |
  | +0x48 / +0x4C | x cursors |
  | +0x50 / +0x54 | y cursors |
  | +0x58 / +0x5C | second x pair |
  | +0x60, +0x64 / +0x68 | limit, then the pair of object B |
  | +0x6C / +0x70, +0x74 | the pair of object A, then its limit |

- **Drag helpers** `touch_drag_delta`, `_b`, `_c`, `_d`. These are four copies of one routine:
  the second touch point minus the first. The result is divided by 10 when an enable byte is set:
  SYS_INFO+0x671, 0x42E6866B or 0x42E1810E.
- **Hit tests**:
  - `touch_hit_pair_a`: the marks of the object at 0x42E68668 (the block that batch 14 backs up);
  - `touch_hit_pair_b`: the object at 0x42E180C8, using 32-bit sums;
  - `touch_hit_cursor_x` / `_y`: the cursors at SYS_INFO+0x808..0x814;
  - `touch_hit_x_b`: SYS_INFO+0x3DA534/8.

  Each takes |touch − mark|, and a distance above the limit becomes 0xFFFF. With SYS+0x2AA set,
  touches above the top third of the plot (y < SYS+6 + SYS+0xA / 3) are ignored. The y cursors
  are then mapped to 2/3 of the plot.
- **`touch_pick_a` / `_b`** turn the nearer mark within the limit into a control code:
  0x10/0x11 for object A, 0x0E/0x0F for object B. When the second point is (0, 0) the code is 0.
- **`cvec_magnitude`** computes |z| for an array of complex floats through the (Thumb) `sqrt`.
  **`cmul`** is the complex float product, with both values in s0..s3.
- **`lradc_init`** (was `func_40068e94`) does read-modify-writes on LRADC_CTRL 0x01C22800:
  bits 12–13 cleared, bit 13 set, bits 4–5 cleared, then enable (bit 0).
- **`strcmp`** compares unsigned bytes; the stock code works a word at a time.
- **`usb_disk_geometry_update`** (was `func_4006a98c`, called by `usb_mode_enter`):
  - sector size → 0x42E68D0C;
  - when the SD state is 1, the byte capacity = sectors × size → 0x42E68C60;
  - 0x42E68D10 = capacity / size.
- **Skipped**: the CP15 helpers (0x4006680C..0x400668BC, cache maintenance 0x4006A5FC), which the
  harness does not emulate, and the Thumb fragments of the maths library.

## Acquisition pipeline (batch 17)

C: `tools/platform_spec/l_acq_pipeline.py`. Tests: `tools/emu/tests/fpga.py` (seventh part).

This is the path that brings CH1/CH2 samples from the FPGA into memory. Call structure is
STATIC-CONFIRMED; the roles are DERIVED.

- **`acquisition_dispatcher`** (Hack name) runs once per main-loop pass:
  - it does nothing while touch byte +0x17 is set;
  - with SYS_INFO+0x3C (stop) set, it only clears SYS+0x38;
  - otherwise it runs the full acquisition (`full_acquisition_entry`, when SYS+0x28 = 0) or
    `runtime_sweep_path`, then `runtime_state_commit_copy`.
- **`runtime_sweep_path`** (Hack name) runs only for run modes 1, 2, 3, 5 and 0x13 (byte
  0x42E68BC8) and only with no touch activity. It re-arms, then loops while SYS+0x700 <
  SYS16+8. Each pass runs:
  1. `one_runtime_capture_step`;
  2. `runtime_state_commit_copy`;
  3. `per_channel_processing`;
  4. `runtime_counter_progress_advance`;
  5. the 0x09 write and the 0x08 re-arm;
  6. `runtime_event_service`.

  A touch stops the loop and sets SYS+0x38 = 1.
- **`one_runtime_capture_step`** (Hack name):
  1. waits for FPGA ready (selector 0x0B; a touch also ends the wait);
  2. applies the time base;
  3. for each enabled channel (CH1 = SYS+0x3E, CH2 = SYS+0x102), writes the stream geometry
     (selector 0x22) and captures the CSI banks chosen by the bank mode SYS+0x100:

     | Mode | CH1 | CH2 |
     |---|---|---|
     | 0 | banks 0–3, then 0x40040D74 | banks 4–7, then 0x40040E44 |
     | 1 | all 8 banks, then 0x40040BEC | — |
     | 2 | — | all 8 banks, then 0x40040BEC |

  4. copies SYS16+0x6F6 samples of each channel buffer (SYS+0x7B4 / 0x7B8, from sample SYS+0x704)
     into the display buffers at 0x42AC7CF4 (+0x62250 for CH2), at index SYS+0x6FC.
- **`capture_block(v, buf)`**:
  1. trigger position, selector 0x1F ← v;
  2. FPGA pins switched to capture (`fpga_pe_mode_select(1)`), `csi_set_buffer(buf)`;
  3. timer 0 stopped, CSI capture on;
  4. PE0 high until SYS+0x70E is set (by the CSI interrupt), then PE0 low;
  5. timer 0 restarted, pins back to the control bus, `csi_clear_frame_flag`.

  `capture_bank(i)` captures bank i into 0x42DF76D4 + i × 0x2134 (8512 bytes per bank).
  `capture_with_length(v, len, buf)` sets the delay to 20 and the length to len + 20 first.
  `lane_level_sum` (batch 12) uses it; its old prototype there was a wrong guess, now fixed.
- **`channel_post_process(ch)`** runs while SYS+0x554 < SYS+0x55C, for an enabled channel. It
  applies the time base × 1.3, then `amplitude_statistical_normalization`, then
  `workspace_copy_and_resampling` of the channel buffer, then `final_per_channel_normalization`.
- **Time-base steps** `timebase_apply_2b4` / `_2b0` / `_2b0_x1_3` call the fragment 0x4003FFE4
  of the split time-base switch with SYS+0x2B4, SYS+0x2B0 or SYS+0x2B0 × 1.3.
- **Still assembly in this area**:
  - `full_acquisition_entry` (1 KB);
  - the bank unpackers 0x40040BEC / 0x40040D74 / 0x40040E44 (they turn the CSI banks into channel
    samples, so they are the next step for getting at the raw ADC data);
  - the conversion wrappers;
  - `moving_average_and_crossing_alignment_search`.

## CSI bank unpacking: raw ADC bytes to channel samples (batch 18)

C: `tools/platform_spec/m_bank_unpack.py`. Tests: `tools/emu/tests/fpga.py` (eighth part).

This is where the raw ADC data becomes the channel signal. The arithmetic is STATIC-CONFIRMED;
the meaning of the fields is DERIVED.

- **Banks**:
  - each capture lands in 8 CSI banks of 0x2134 bytes at 0x42DF76D4 (batch 17);
  - a bank holds one 8-bit ADC lane, starting at byte SYS16+0x6F2 plus a per-lane phase;
  - the channel buffer interleaves the lanes. With 8 lanes, sample j of lane i goes to
    buf[(7 − i) + 8j]; with 4 lanes, to buf[(3 − i) + 4j];
  - the lane → bank map is SYS_INFO+0x6DC..0x6E3, and the per-lane phase SYS+0x6D4..0x6DB.
- **`calibration_gain_baseline_clamp_conversion`** (Hack name) converts one lane. Per sample:

  ```
  q    = gain[V/div] × cal_gain[bank] / 1000          (signed)
  off  = cal_offset[bank] − q × SYS+0x1B8
  v    = ((q × raw + off) >> SYS+0x264) − base
  out  = clamp(base + (k × v) >> 5,  0,  SYS+0x1C0 × SYS16+0xA)
  ```

  The calibration object is 0x42E53A0C: per-bank gain at +0x10 and offset at +0x30. The
  per-channel parameters are:

  | | CH1 | CH2 |
  |---|---|---|
  | gain | SYS+0x1C4[SYS+0x41] | SYS+0x214[SYS+0x105] |
  | base | SYS+0x1C0 × SYS+0x60 | SYS+0x1C0 × SYS+0x120 |
  | k | SYS+0xE0 | SYS+0x1A0 |

  For a bank above 7 or a channel above 1 the stock code reads stale registers, so the C uses 0.
  The firmware never does this.
- **Wrappers**:
  - `bank_unpack_all` and `first_conversion_wrapper` (Hack name) unpack all 8 lanes;
  - `bank_unpack_0_3` / `bank_unpack_0_3_n` unpack lanes 0–3;
  - `bank_unpack_4_7` and `second_conversion_wrapper` (Hack name) unpack lanes 4–7.

  The plain ones use SYS16+0x6F4 samples per lane; the `_n` / `*_conversion_wrapper` ones take
  the count as an argument.
- **For an external analysis tool**: the samples in SYS+0x7B4 (CH1) / 0x7B8 (CH2) are already
  calibrated and clamped. The raw 8-bit ADC values only exist in the banks, before this step.
  Each lane is probably one ADC of a time-interleaved set: 8 for one channel, 4 per channel when
  both are on (DERIVED from the bank modes, not checked on hardware).

## Channel filters after unpacking (batch 19)

C: `tools/platform_spec/n_channel_filters.py`. Tests: `tools/emu/tests/fpga.py` (ninth part).

These run on the calibrated samples, after batch 18 and before display. All are
STATIC-CONFIRMED; the names are DERIVED. They matter for an external analysis tool because they
change the signal the device shows.

- **`channel_moving_average(ch)`**: a box filter of width SYS16+0x662 (off below 2) over the
  SYS+0x7BC samples of the channel buffer, written in place at the start of each window.
- **`channel_step_limiter(ch)`**, enabled by SYS+0x70D. Each pair of neighbouring samples is
  pulled together by half its difference. The difference is clamped to ±SYS+0x1C0 × 5 and the
  filter runs 3 passes. At the V/div index 12 (the most sensitive range) the clamp is × 15 and it
  runs 6 passes. This acts as an extra low-pass on the most sensitive range.
- **`channel_to_display_copy(ch, k)`** copies SYS+0x454[k] samples into the display buffer
  (0x42AC7CF4, +0x62250 for CH2), at the frame index SYS+0x2C4[k].
- **`frame_deviation_blend(ch, n, f)`** works across the F = SYS16+0x550 frames stored in the
  display buffer, each L = SYS+0x2B0 points long:
  1. computes the mean over the frames at each point (into 0x42C947D4);
  2. computes each frame's largest deviation from that mean. It starts at (1 − f) × lim and is
     capped at lim = SYS+0x1C0 × n;
  3. replaces every sample by (w × sample + (100 − w) × mean) / 100, with
     w = 100 × deviation / lim.

  A frame close to the mean is pulled further towards it, so this works as a noise-reduction
  "persistence" blend.
- **`channel_filters_by_timebase`** applies the blend to the enabled channels at the fast time
  bases only:

  | Time-base index | n | f |
  |---|---|---|
  | below 16 | 40 | 0.8 |
  | 16–20 | 8 | 0.7 |

## Settings records and SPI flash helpers (batch 20)

C: `tools/platform_spec/o_settings_spiflash.py`. Tests: `tools/emu/tests/fpga.py` (tenth part).

- **Settings records** are built on the batch 15 serialiser. Each record has three parts:
  - a field list: the cursor is set to 0, then `serializer_process_field` runs per field;
  - a save: encode mode, the field list, then `persistence_write_read_modify_write(0, flash
    address, image, length)`;
  - a load: `persistence_read` into the image, decode mode, then the field list.

  The image is SYS_INFO+0x360404. The flash address of each record is a word in SYS_INFO:

  | Record | Fields | Flash address at | Length |
  |---|---|---|---|
  | A | 64 bytes at 0x405454EC (SYS+0x684) | SYS+0x7F4 | 0x1000 |
  | B | SYS+0x7AC (1), +0x658 (4), +0 (4) | SYS+0x7E4 | 0x1000 |
  | C | `record_c_fields` (0x40043B0C, batch 60): the FFT spur lists at 0x42E3C000 | SYS+0x7F0 | 0x4000 |
  | D | 0x42E554B0: 4 words, then 300 × (word, double, double) | SYS+0x7EC | 0x2000 |
  | E | 4 words at 0x42E68AE0; the first is the magic 0x5A5555A5 | SYS+0x7DC | 0x1000 |
  | F | the slot byte SYS+0x7C0 | SYS+0x7C4 | 0x1000 |

- **`settings_b_load`**: when the loaded SYS+0 differs from the value before, it sets
  SYS+0x7AC = 2 and runs 0x400447C8.
- **`settings_e_load`** without the magic (first start or erased flash) rebuilds the defaults:
  1. runs 0x4003EFAC;
  2. runs 0x4004435C for each slot 1..5 (SYS+0x7C0);
  3. saves slot 1 in record F;
  4. sets SYS+0x7AC = 2 and saves record B.

  The record C load entry (0x40043C9C) is 12 bytes and falls through into 0x40043CA8.
- **SPI flash helpers**:
  - `spi0_pins_init`: PC0–PC3 set to SPI0 (function 3), drive 3;
  - `spi0_clock_on`: the CCU gate;
  - `spi0_init`;
  - `spi_flash_cmd_read1`: one command byte, then one byte read;
  - `spi_flash_wait_ready`: polls status 0x05 until the busy bit clears;
  - `spi_flash_write_enable`: command 0x06.

  These are the standard SPI-NOR commands. The C only touches the flash through the stubbed
  `sys_spi_*` calls: nothing in the tests writes to a real device.

## UI window layer (batch 21)

Spec: `tools/platform_spec/p_ui_windows.py`, 51 functions.

The UI is a set of 42 numbered windows (screen regions), each with a 14-byte record in the
table at 0x42E64520 (id × 14), plus a 20-slot window stack at 0x42E68BC8. The record and stack
fields below are read from the dispatcher and the draw functions. The meaning of each flag is
**DERIVED**; which screen each window id shows is still **OPEN**.

| Record offset | Use |
|---|---|
| +0 | open (set by `ui_window_open`) |
| +1 | dirty |
| +2 | self-contained: when 0 the window is redrawn whenever a window below it set +4 |
| +3 | animate on the next redraw |
| +4 | forces the windows above to redraw |
| +6 / +8 / +0xA / +0xC | x, y, w, h (halfwords, pixels) |

Stack: +0 is the id on top, +1 is its slot index, and +2..+0x15 hold the 20 slots, bottom first.

- **Buffers** (all 1022 pixels wide, RGB565):
  - `fb_back` 0x41BFBF68, where the windows are composed;
  - `fb_front` 0x41FB8FE8, scanned out (names from `e_display.py`);
  - the offscreen buffer 0x4233CBBC, used for animated windows.

  `fb_target_back` / `fb_target_offscreen` set the draw-target pointer at 0x40095498.
  `fb_present_rect` copies a rect from the back buffer to the front buffer, then flushes the
  D-cache.
- **`ui_window_redraw_dirty`** (0x4003CA88):
  - It does nothing while the byte 0x42E68BC7 is set.
  - Otherwise it walks the stack bottom to top. Each window that needs a redraw has its +1 and
    +3 cleared, then gets its wrapper called with the old +3 as the animate flag.
- **42 redraw wrappers** `ui_window_NN_redraw` (0x4003D71C..0x4003DEB4):
  - 14 of them are plain: target the back buffer, then call the draw function.
  - 28 of them, when animate is set, draw into the offscreen buffer and then reveal the window
    with `ui_reveal_slide_down` or `ui_reveal_bottom_up`. Window 33 picks one of the two from the
    byte 0x42E68A8A.

  The draw functions (0x40019DB8 … 0x4002C32C, 0x40037AB8, 0x4005FF88, 0x40062EE8) are still
  assembly; all of them load their own record's rect.
- **Reveal**: per step, n = min(h, h × floor(10000 × (tanh(t) + 0.02)) / 10000).
  - t starts at 0; the step starts at 0.2 and shrinks by 0.012 per frame down to 0.01, so the
    window eases in fast, then slows down.
  - Each step copies n rows and presents them.
  - Both reveals end by calling the dispatcher again.
- **Stack helpers**:
  - `ui_window_open(id)` puts id in the first free slot and sets +0, +1 and +3;
  - `ui_window_stack_top_update`;
  - `ui_windows_open_1_5`.

  The rest of the stack helpers are in batch 22.

## Trace processing chain and window stack helpers (batch 22)

Spec: `tools/platform_spec/q_trace_windows.py`, 18 functions.

- **Trace numbering** (DERIVED from the flags tested):
  - traces 0 and 1 are CH1 and CH2, gated by SYS+0x3E / SYS+0x102;
  - traces 2 and 3 are the second trace of each channel, present when SYS+0x2AA is set.
- **The chain itself.** These functions only fix the order and the gates. The work is done by
  callees (0x4005E10C, 0x4003C0AC, 0x4005E12C, 0x4005DDD4, …; 0x4003C158, 0x4003C314,
  0x4003BB6C and 0x4003C5CC are C since batch 53).
  - `traces_update`:
    - does nothing while the byte 0x42E68BC7 is set;
    - does nothing when SYS+0x27A = 1 with SYS+0x3C and SYS+0x38 both 0;
    - otherwise runs `channels_process_full`, or `traces_process_single` when SYS+0x652 is set.
  - `channels_process_full` processes CH2 first unless SYS+0x664 is set.
  - `trace_process_full` runs only when SYS+0x101 is set.
  - `trace_c5cc_window` runs 0x4003C5CC over the capture window, and only when SYS+0x6FC ≥
    SYS+0x708.
- **`runtime_rearm_setup`** (Hack name): a fixed list of eight re-arm calls, ending with the
  FPGA selector pulse.
- **`table_scatter_42cf64b4`**: clears 0x3AAC bytes, then scatters the 0x3AAC (value, index)
  pairs from the ROM table 0x40090444 into the halfword array 0x42CF64B4.
- **Window stack**:
  - `ui_windows_mark_dirty`;
  - `ui_window_invalidate_from` (0x4003CD44);
  - `ui_window_close` (0x4003CE50): removes the id by moving the slots above down, but slot 19
    keeps its old value;
  - `ui_windows_reset`;
  - `ui_frame_update`: redraw, `battery_service`, present.

  Stock quirk: when the id is not on the stack, invalidate uses the caller's r1 and close uses
  the caller's r2 as the slot. The C takes these as explicit extra parameters, so the behaviour
  stays the same.

## User slots, more settings records and the zoom index (batch 23)

Spec: `tools/platform_spec/r_slots_timebase.py`, 19 functions.

- **Settings records.** Every record lives in the serialiser image at SYS+0x360404.

  | Record | Fields | Flash address word | Bytes |
  |---|---|---|---|
  | slot 1..5 (SYS+0x7C0) | `slot_fields` (0x40043F54, batch 61) | SYS+0x7C8 + 4 × (slot − 1) | 0x1000 |
  | calibration | `calibration_process_serialized_fields` | SYS+0x7E0 | 0x5000 |
  | G | three words at 0x42E3F984 + 0x2F64..0x2F6C | SYS+0x7E8 | 0x1000 |
  | H (fields only) | SYS+0x75C, 768, 760, 76C, 764, 770, 774 | — | — |

  Stock quirk: for a slot number outside 1..5, `settings_slot_save` uses the caller's r4 as the
  flash address and `settings_slot_load` uses the caller's r1. The C uses 0, and the tests keep
  the slot in range.
- **`settings_load_all`** is the start-up order: E (which rebuilds the defaults when the magic
  is missing), F, the slot, B, calibration, A, G, D, C.
- **Left in assembly: 0x400447C8**, the factory reset. It does the following:
  1. rebuilds the defaults, keeping SYS+0x7AC;
  2. saves slots 5..1 and record B;
  3. calls `wdt_system_reset` and spins forever.

  It never returns, so the harness cannot compare it.
- **Zoom / time-base index SYS+0x598** (role DERIVED from how it is used):
  - `zoom_index_set`, `zoom_index_add` and `zoom_index_sub`. The subtract only happens when
    SYS+0x614 < SYS16+0x2AC × SYS+0x554.
  - All three end in 0x400423B8, still assembly. Its first step is to clamp the index to at most
    SYS+0x588 − 1.
  - `zoom_entry_select` copies the 64-bit entry SYS+0xBD0 + 8 × index to SYS+0x5A0.
  - `zoom_mode_check`:
    - it clears SYS+0x2AA and returns 1 when SYS+0x28 is set with SYS+0x3C clear, when SYS+0x652
      is set, or when SYS+0x5B0 = SYS+0x2A8;
    - it clamps the index to SYS+0x5B0 − 1.
  - `zoom_reset`.
  - `timebase_index_set` (SYS+0x588) and the 64-bit setters for SYS+0x570 and SYS+0x578.

## Start-up, defaults and state initialisers (batch 24)

Spec: `tools/platform_spec/s_init_defaults.py`, 13 functions.

- **Display bring-up**:
  - `lcd_pins_init` sets PD1..PD21, all except PD12, to function 2 (the RGB LCD bus);
  - `lcd_start` turns the TCON and DE clocks off, sets up the pins, then calls
    `display_init(1022, 600, fb_front)`.
- **`factory_defaults`** (0x4003EFAC, the call used by `settings_e_load` when the magic is
  missing):
  - it clears SYS_INFO (0x3DD588 bytes), the block 0x42E3F984, the window table and stack, and
    the blocks 0x42E68BB0 and 0x42E68A88;
  - it then runs 0x4003E738 and `state_blocks_init`.
- **`state_blocks_init`**:
  - it runs the three block initialisers, 0x42E3F984, 0x42E180C8 and 0x42E68668;
  - it calls 0x40030060(0, k) for k = 7, 6, 3, 4, 5, 2;
  - it ends with `state_backup_all`.

  The initialisers write fixed defaults. Among them are the 64-bit values 200000, 500000000 and
  250100000 at 0x42E68668+0x120.., and the doubles 20.0 and 60.0 at +0x140 / +0x148. Each one
  ends by setting the byte 0x42E68BC6 = 1. Their meaning is OPEN.
- **`applied_state_invalidate`**: writes the complement of the current values into the
  "last applied" copy at SYS+0xBB0..0xBCA. This guarantees the next `outputs_apply` sends
  everything to the hardware again. The interpretation is DERIVED from the pattern: the
  complement is never equal to the current value.
- **`outputs_apply`** (Hack: SysConfigExec): unless the byte 0x42E68BC7 is set, runs
  0x4003A654, `frontend_apply_all` and 0x40063328.
- **`acquisition_geometry_setup`**:
  - the step SYS+0x6F8 is 2 at time-base index 0x21, else 1;
  - the per-bank count is SYS+0x6F4 = SYS+0x558 × step / SYS+0x1B1 + 2;
  - the window end is SYS+0x708;
  - it also sets the display lengths at SYS+0x454 / SYS+0x38C.
- **SPI flash**:
  - `spi_flash_sector_erase` sends 0x20 with a 3-byte address, or 0x21 with a 4-byte address
    when the word 0x42E68CD4 is set; it returns 0 or −1;
  - `spi_flash_write_paged` splits a write so that no chunk crosses a 256-byte page;
  - `settings_c_load_tail` is the fall-through half of `settings_c_load`.

## Time-base index, committed copy and acquisition glue (batch 25)

Spec: `tools/platform_spec/t_timebase_acq.py`, 19 functions. The names are DERIVED from what
the code does. The meaning of most SYS_INFO fields is still OPEN.

- **Time-base index SYS+0x588**:
  - `timebase_period_load` copies the 64-bit table entry SYS+0xBD0 + 8 × index to SYS+0x590;
  - `timebase_slow_flag_update` sets SYS+0x28 = (index >= SYS+0x2A9);
  - `timebase_index_clamp` (0x4003BCF0) does three things in order:
    - it clamps the index to [SYS+0x2A8, SYS+0x2A7];
    - when SYS+0x3C is set, it caps the index at the committed index SYS+0x5B0 + 3;
    - it then steps the index up for as long as SYS+0x5B8 / period / SYS+0x5D4 >= SYS+0x2AC.
    Each step invalidates the windows from id 3. The stock code passes the old index in the
    hidden r1, and the C passes it explicitly.
  - `timebase_index_step(d)` and `timebase_index_step_down(d)` change the index, clamp it, and
    set the 16-bit flags SYS+0x86E / SYS+0x870. The step down only happens while SYS+0x55C and
    SYS+0x5FC are below SYS+0x2AC × SYS+0x554, and it compares against the full 32-bit d.
- **Committed copy**:
  - `runtime_state_commit_copy` copies the live parameters to SYS+0x5A8..0x5EC, SYS+0x29,
    +0x58, +0x64, +0x118 and +0x124, but only when SYS+0x38 is set;
  - `runtime_state_commit_force` sets SYS+0x38 and then falls into the copy.
- **Acquisition glue**:
  - `sample_divisor_setup` derives the mode byte SYS+0x100, the divisor SYS+0x1B0 (1 or 2) and
    SYS+0x1B1 = SYS+0x524 / divisor from the flags SYS+0x3E / SYS+0x102;
  - `acquisition_apply` (0x4003A654, called by `outputs_apply`) runs, in order:
    `timebase_slow_flag_update`, `sample_divisor_setup`, 0x4003A984,
    `fpga_sample_count_write(SYS+0x30)` and `timebase_apply`;
  - `acquisition_reconfigure` runs the divisor set-up, 0x4003A680, 0x4005DB9C, 0x4005D27C,
    `acquisition_geometry_setup` and the commit copy;
  - `acquisition_rearm` runs 0x4003B834, plus 0x4004201C when the second traces are on, unless
    the lock byte 0x42E68BC7 is set;
  - `runtime_counter_progress_advance` is the per-bank sample counter step. Its udiv by
    SYS+0x1B1 gives 0 when the divisor is 0, and the C reproduces that;
  - `scaled_pair_update(mode)` (0x4003C0AC) scales SYS+0x5E8 + 1 and SYS+0x5EC − 1 by
    SYS+0x5D4 / SYS+0x5F8, or by SYS+0x5D4 / SYS+0x610, with signed 64-bit arithmetic. The
    results go to SYS+0x608/0x60C or SYS+0x620/0x624. `mode_not_0_or_1` picks the branch, and
    `mode_not_0_or_2` is its sibling.
  - `trace_buffer_pointers_setup(n)` (0x4003FFE4) sets the two read pointers SYS+0x7B4 / 0x7B8
    into the sample buffer 0x42C62AF4, 0xC738 samples apart. The symbol table splits this
    function at 0x40040004. That second entry is not a real function and stays in assembly.
- **UI glue**:
  - `ui_windows_invalidate_1_5` invalidates windows 1..5. The stock code chains the hidden r1
    from one call to the next, and the C models that value.
  - `ui_cf88_then_579c` calls 0x4003CF88, whose role is OPEN, then `ui_text_init` (batch 26).
- **SPI flash page program** (`spi_flash_page_program`, 0x4004372C):
  - the command is 0x02 with a 3-byte address, or 0x12 with a 4-byte address;
  - the data follows in the same `sys_spi_write_then_write`;
  - it returns 0, or −1 when the write enable or the transfer failed.
- **Left in assembly on purpose**: 0x4003F2F4..0x4003F894 is a single time-base switch
  function. The analyser split it into 0x4003F2F4, about 24 fragments of 16 to 28 bytes
  (0x4003F4A4..0x4003F6E8) and 0x4003F704. It needs its own batch with corrected bounds.

## Settings image sections, UI text table and the entry pad (batch 26)

Spec: `tools/platform_spec/u_image_text_numpad.py`, 26 functions.

- **Sectioned settings image** (the serialiser image at SYS+0x360404):
  - `settings_image_encode` writes the field list 0x400448B4 from cursor 0x32, then records I
    and J;
  - for each section it records the start (SYS+0x75C / 0x760 / 0x764), the size
    (SYS+0x768 / 0x76C / 0x770) and the end (SYS+0x774);
  - it then writes these words as header H at cursor 0;
  - `settings_image_decode` reads the header and then seeks to each section;
  - record I (`settings_i_fields`) is a w × h array of halfwords at SYS+0x109684;
  - record J (`settings_j_fields`) is SYS+0x2B0 halfwords per enabled channel (SYS+0x3E,
    SYS+0x102), taken from 0x42AC7CF4 and 0x42B29F44.
  What these arrays hold is OPEN.
- **UI text table** at 0x42C2FE74:
  - 141 entries of 0x1A0 bytes;
  - +0 is a pointer to the current string;
  - four 0x64-byte string columns sit at +4, +0x68, +0xCC and +0x130;
  - +0x194/+0x195 are the current byte pair, and +0x196.. hold one pair per column;
  - `ui_text_set_langN(k, s, a, b)` fills column N; the 13.5 KB function 0x40045974 calls
    these setters for every entry;
  - `ui_text_language_apply` selects column SYS+0x7AC (1..4). Reading the columns as languages
    is DERIVED: the strings are per-entry translations, which fits SYS+0x7AC being the menu
    language.
  - `str_copy` (0x400342B4) is a plain strcpy that returns the end pointer.
- **Entry pad** (state 0x42E68A88, window 0x21). The entry-pad reading is DERIVED from the
  '-' / '.' handling and the 20-character buffer.
  - The state holds the 20-character text at +3, the length at +0x17, the '.' and '-' flags at
    +0x18 / +0x19, a double at +0x20 and three pointers at +0x28..+0x30.
  - `numpad_reset` clears the state, using `mem_clear` (memset with 0).
  - `numpad_open_mode1` and `numpad_open_mode3` set the mode byte +1 and the window position,
    then open window 0x21.
  - `numpad_close` closes window 0x21.
  - `numpad_append_char` has no bound check, as in the stock code.
  - `numpad_backspace` clears the matching flag when it removes '-' or '.'.
- **Veneers**: `trace_span_full(tr)` calls 0x4005F778(tr, 0, SYS+0x2C0), and
  `trace_span_window(tr)` calls it with (SYS+0x6FC, SYS+0x6F6). Two 4-byte branches lead to
  them. The role of 0x4005F778 (352 bytes) is OPEN.

## Touch buttons, window touch handling and loose helpers (batch 27)

Spec: `tools/platform_spec/v_touch_buttons.py`, 59 functions. 36 of them are generated from the
`GROUPS` table in the spec.

- **Button records**: the table 0x42E61FF8 holds 10-byte records.
  - Layout: +0 state, +1 a copy of touch byte +0xD, then the rectangle x0, y0, x1, y1 (u16) at
    +2..+8.
  - `touch_button_update` acts only when the current touch point (0x42E68BB0+4/+6) lies strictly
    inside the rectangle. It copies the touch byte and sets the state to 1 on a press or 2 on a
    release.
  - `touch_button_set_rect` sets the rectangle, and `touch_button_disable` moves it to x/y 10000.
  - Byte +0 of the table is cleared together with a released button.
- **Group updaters**: 36 functions call `touch_button_update` for a fixed list of records, from 1
  to 31 of them. They are named after the first record offset (`touch_group_0250`,
  `touch_button_update_0804`, ...). The list probably follows the screens; which screen uses
  which list is OPEN.
- **Release handlers**:
  - each one updates its button and acts on state 2;
  - button +0x804 opens window 0x17;
  - button +0x1EC runs the factory reset 0x400447C8;
  - button +0x19C leaves USB mode and resets the window layer.

  Three more handlers (+0x7FA, +0x912, +0x9EE) also close a window. They pass on hidden r1/r2
  values that a callee leaves behind, so they stay in assembly for now.
- **Window touch handling**:
  - `ui_window_touch_handle(id)`: a touch inside window id invalidates the windows from id;
  - otherwise a release outside a window that is not self-contained runs
    `ui_windows_close_untouched`;
  - `ui_windows_close_untouched` closes windows from the top of the stack down to the first one
    that contains the point. The hidden r1/r2 of `ui_window_close` are reproduced.
- **Entry pad**:
  - modes 4, 5 and 6 work like modes 1 and 3;
  - `numpad_sign_apply` inserts or removes the leading '-' so that the text agrees with flag
    +0x19.
- **Sine table** at 0x42D564B4, 20001 words:
  - `sine_table_build` stores sin(a) × 10⁸ / 3.1415926;
  - stock quirk: the first argument is the caller's d0, and after that a = (i − 1) × π/10⁴,
    because the index is converted before it is incremented;
  - `sine_table_lookup(x)` returns T[x mod 20001], negated for negative x;
  - `acquisition_reconfigure` calls the builder with whatever d0 holds at that point.
- **Small ones**:
  - `layer_clear(n)` zeroes layer n at 0x400954A4 + n × 0x12BE70 (1022 × 600 × 16 bit);
  - `str_find_char` returns the matching character itself, not a pointer;
  - two gates do nothing while the lock byte 0x42E68BC7 is set;
  - `calib_run_4_channels` runs four steps for each of the 4 channels, with the index at
    0x42E68668+0xF4, then `settings_c_save`. That these are calibration steps is DERIVED from
    the block;
  - `index_map_if_above` maps only when SYS+0x554 < d;
  - `disk_format_and_reload` formats, mounts and runs three refresh calls.

## Small leftovers: setters, gates, one-byte FPGA commands, drawing wrappers (batch 28)

Spec: `tools/platform_spec/w_small_sweep.py`, 45 functions. Tests: `tools/emu/tests/sweep.py`, a
new suite (see CONTRIBUTING.md).

- **Setters**: word 0x42E68C74, u16 0x42E68BE2 and byte 0x42E68BE0 each have a one-store setter.
  `draw_target_default` sets the drawing target pointer 0x40095498 to 0x41FB8FE8.
- **Drawing** (names DERIVED from what the code writes):
  - `pixel_put(x, y)` stores the u16 colour 0x42E68660 at target + 2 × (x + 1022 × y), or calls
    the blended path 0x4002EA88 when byte 0x42E6865C is set. The stride 1022 matches the layer
    size from batch 27.
  - `sys_rect_fill` sets colour 0 and draws the rectangle from the u16 at SYS+4..+0xA;
    `rect_42e68c38_draw` draws the rectangle described at 0x42E68C38.
  - `colour_toggle_42e68d1a` flips the byte and picks colour 0x10262626 or 0x10303030.
  - `display_start`: draw defaults, colour 0x00FFFFFF, word 0x42E68C74 = 0x42E64A98, `lcd_start`,
    `fb_clear_all`.
  - `ui_window_05_draw` (0x4001C348, the draw function of window 5) puts the button record
    0x42E620EA on the rectangle of window record 5, then calls 0x400346C0.
- **Gates**: four functions run a screen routine only while touch byte 0x42E68BB0+0x17 is clear.
  Two more choose between screen routines by SYS+0x652. What +0x17 and +0x652 mean is OPEN.
- **One-byte FPGA commands**: each writes a selector and then one or two data bytes from SYS:
  - 0x61 ← SYS+0x3B; 0x16 ← SYS+0x27B; 0x63 ← byte 0x42E428E4;
  - 0x15 ← 1 or 2 from SYS+0x27C; 0x0C ← the u16 SYS+0x276, high byte first;
  - 0x1A ← 1 only when SYS+0x275, +0x274 and +0x27A are set;
  - 0x1B ← bits 8..15 and 0..7 of SYS+0x288 when +0x275 and +0x274 are set, else two zeros;
  - 0x17 ← the low byte of SYS+0x280 under the same condition, else `fpga_cmd61_pulse` (0x61
    with SYS+0x3B = 1, 0x17 ← 0, then 0x61 with SYS+0x3B = 0);
  - 0x18 is read back: SYS+0x279 = bit 0 of the answer.

  `fpga_config_push` sends 0x61, 0x0C, 0x1A, 0x15, 0x16, then 0x400634F0 and
  `trigger_offsets_update`. The selector names are the numbers; what each register does is OPEN.
- **Others**:
  - `value_in_table_405456e8(mode, v)` says whether v (negated for mode 1) is one of the 24 s16
    at 0x405456E8;
  - `sys_3c_enter` and `sys_3c_leave` save and restore SYS+0x3C through +0x3D, together with
    the byte SYS+0x3DA52C;
  - `check_pair_save` runs `settings_a_save` only when both checks 0x40064C28 and 0x40064CF0
    return an odd value, and stores the result at 0x42E68B25;
  - three reload chains call fixed lists of screen refreshes and clear SYS+0xF90, +0xF9E, +0xFA4.

## Colour, buffer copy, the list at SYS+0xFA0, clamps and labels (batch 29)

Spec: `tools/platform_spec/x_lists_colour.py`, 23 functions. Tests: `tools/emu/tests/sweep.py`.

- **Drawing colour**: `draw_colour_set(0xAARRGGBB)` fills the block 0x42E6865C.
  - +0 = A, +1 = R, +2 = G, +3 = B, +4 = RGB565, +8 = the whole word.
  - `pixel_put` (batch 28) blends through 0x4002EA88 when byte +0 is non-zero, so the high byte
    works as alpha (DERIVED).
- **Buffer copy**: `fb_rect_copy_from_e4e84(x0, y0, x1, y1)` copies a rectangle of 16-bit
  pixels, stride 1022, from 0x420E4E84 to 0x41FB8FE8. It then cleans the data cache.
- **The list at SYS+0xFA0** (s16 count at +0xFA0, u16 entries from +0xFA4). The list holds
  entries of some kind; which ones is OPEN.
  - Three insert functions shift the entries up and put the new one at +0xFA6. They take the
    shift length from SYS+0x754, +0x756 or +0x758 (the last one minus 1).
  - Three identical functions count the non-zero bytes at SYS+0x5DC6 into SYS+0x88BE.
  - Three identical functions clamp the indices +0xFA4 and +0xF9E.
  - `list_current_advance(i)` moves the current entry SYS+0x3DA52E on when it is entry i.
  - `array_3da548_decimate(n)` keeps every n-th u16 of the array SYS+0x3DA548.
- **Clamps**:
  - SYS+0x810/+0x814 stay in 12..(SYS+0xA) − 15;
  - SYS+0x808/+0x80C stay in 12..(SYS+8) − 12;
  - the words +0xAC64/+0xAC68 of the block 0x42E180C8 stay in 2..(its +4) − 2.

  The SYS+4..+0xA rectangle of batch 28 suggests that these are cursor positions inside the
  plot area (DERIVED).
- **Labels**: `level_label_db` and `level_label_dbmv` format a value passed in d0 (0x40038C00)
  and write the unit text "dB" or "dBmV" after it.
- **File checks**: two functions build a path into 0x4054D728 (0x40034B78 or 0x40035BC4), then
  try `f_open` in read mode. They return 1 when that works and close the file again.
- **Frame update**: `ui_frame_chain` (0x400346C0) runs the per-frame chain:
  - outputs, plot rectangle, acquisition dispatch and re-arm, traces, zoom views, and further
    screen updates;
  - most of them do nothing while touch byte +0x17 is set;
  - the draw function of window 5 calls it.
- **Window 40**: `ui_window_40_draw` runs the screen chain of the block 0x42E68668. The steps
  0x40060FE8, 0x40060DC8 and 0x40061F10 are skipped when both +0 and +4 are set.
- `fpga_cmd1c_write`: selector 0x1C with the word SYS+0x28C, like 0x1B with SYS+0x288.

## File lists: screenshots, waveforms, captures (batch 30)

Spec: `tools/platform_spec/y_file_lists.py`, 19 functions written from templates. Tests:
`tools/emu/tests/sweep.py`, with FatFs stubs that also check that every call gets the same `FIL`.

- **Three lists, one layout.** Only one list is loaded at a time, into the same SYS fields:
  - s16 count at SYS+0xFA0, u16 entries from SYS+0xFA6;
  - u16 page size SYS+0xF9A, s16 page index SYS+0xF9E, page count SYS+0xFA2, selected entry
    SYS+0xFA4;
  - page marks at SYS+0x84D6, entry marks at SYS+0x5DC6;
  - the path of an entry's file is built into SYS+0x88C0 (= 0x4054D728).
- **What differs per list**:

  | List | Index file | Length field | Path helper | Entry remover |
  |---|---|---|---|---|
  | A: screenshots | 0x4008DB60 `/Screenshots/number_table.sys` | SYS+0x754 | 0x40034B78 | 0x40034A0C |
  | B: waveforms | 0x4008E2D0 `/Waveform/number_table.sys` | SYS+0x756 | 0x40035BC4 | 0x40035A58 |
  | C: captures | 0x4008E2EC `/Captures/num_table.sys` | SYS+0x758 | (0x40036xxx, not in this batch) | |

  List C also stores the u16 SYS+0x3DA52E in its index, at offset 2, and keeps a data block
  (word SYS+0x3DA544 × SYS+0x758 bytes at SYS+0xD744) in the file 0x4008E8D4 `/Captures/data.cap`.
- **Functions** (A and B unless noted):
  - `index_load` reads the count and the entries, then sets the page count. When the file does
    not open, a per-list handler (0x400347A0, 0x400357EC) runs instead;
  - `index_save` writes the index back (C as well);
  - `page_mark` and `marks_clear` (A, B and C);
  - `page_validate` sets the page from the selected entry and checks the files of that page.
    At the first missing one it runs `prune_missing`;
  - `prune_missing` drops the entries whose file no longer opens;
  - `delete_marked` removes the marked entries and deletes their files with `f_unlink`;
  - `list_c_data_load` and `list_c_data_save`.
- The index file names (STATIC-CONFIRMED strings) identify the lists: screenshots, waveforms
  and captures. The per-entry file name format is covered in the batch 31 section.

## File lists, second part (batch 31)

Spec: `tools/platform_spec/z_list_files.py`, 22 functions. Tests: `tools/emu/tests/sweep.py`.

- **Entry file names** (STATIC-CONFIRMED, from the byte constants the path helpers store; not
  rewritten yet):
  - 0x40034B78 builds `/Screenshots/<n>.bmp`;
  - 0x40035BC4 builds `/Waveform/<n>.wav`.

  `<n>` is the entry number in decimal without leading zeros. The helpers have separate branches
  for n < 10, < 100, < 1000 and beyond. Both write the path to SYS+0x88C0. The `.wav` extension
  does not make the file a WAV; the content is a settings image (see below).
- **Per list** (A, B and C unless noted):
  - `entry_remove(v)` removes entry v and shifts the rest down;
  - `next_free` marks the used numbers in SYS+0x8924 and stores the first free one from 1 in
    SYS+0xF9C;
  - `index_create` is the handler that runs when an index file does not open. It creates the
    directory (`/Screenshots`, `/Waveform`, `/Captures`) and writes an empty index. For list C it
    also writes an empty `/Captures/data.cap`. The firmware clears that data block with
    `copy_u16` from address 0, not with `fill_u16`; the C keeps that call;
  - `list_c_index_load` is list C's index loader.
- **Saving a new item**:
  - `list_a_save_new`: draws an image, takes the next free number, builds the path and calls
    0x4005C3F0. That function writes a BMP header (starts with `BM`), so it is DERIVED to be the
    bitmap writer;
  - `list_b_save_new`: refuses when `disk_free_bytes` is below twice word SYS+0x774 (it draws an
    image and waits 1 s). Otherwise it encodes the settings image into SYS+0x360404 and writes
    word SYS+0x774 bytes of it (`file_write_360404`, mode 0xB).
  - On success both insert the number (0x400349B4, 0x40035A00) and save the index.
- **Loading**:
  - `list_a_thumb_load` reads two u16 at offsets 0x46 and 0x48 of a screenshot file, then their
    product × 2 bytes from offset 0x4A. That this is a thumbnail (width, height, pixels) is
    DERIVED;
  - `list_a/b_page_thumbs` loads up to 3 × 5 entries of the current page into slots of 0x256D8
    bytes from SYS+0x12ED5C;
  - `list_a_current_load` reads 0x12B6A0 bytes from offset 0xDEA8 of the selected screenshot to
    0x42468A58;
  - `list_b_file_load` and `list_b_current_load` read a waveform file into SYS+0x360404: a
    0x1C-byte header (`settings_h_decode`), then the rest. The current load also saves the
    acquisition state first (`state_backup_all`, after `state_restore_all` when SYS+0x665 is
    set) and sets SYS+0x665 = SYS+0x3C = 1.
- **List C records**:
  - records in SYS+0xD744 are word SYS+0x3DA544 halfwords long. The key is the low byte of
    halfword 0 × 256 + the low byte of halfword 1;
  - `list_c_record_select(v)` sets SYS+0xF9C and word SYS+0x3DA53C from the record of key v and
    copies its payload to SYS+0x3DA548;
  - `list_c_delete_marked` deletes the marked entries, then saves and reloads the index and the
    data block.
- `cursor_3da534_clamp(flag)` keeps the cursor pair SYS+0x3DA534/0x3DA538 at least the width
  SYS+0x3DA540 apart (moving one or the other, by flag) and between 12 and the screen width − 12.
- Still in assembly, because the result depends on a register the caller leaves:
  - 0x40036DE4, when no record has key 0;
  - 0x40037020, when key v is absent.

## Widgets, fonts, pixel blending, trace cursors (batch 32)

Spec: `tools/platform_spec/za_widgets_fonts.py`, 19 functions. Tests: `tools/emu/tests/sweep.py`.

- **Widgets:**
  - `colour_table_3dd530_init` fills the 22 colour words at SYS+0x3DD530;
  - `switch_on_off_draw(x, y, on)` draws an ON/OFF switch. The strings are at 0x40019DB0 and
    0x40019DB4. The function also saves and restores the font word 0x42E68C74 and the half
    0x42E68BE2;
  - `slider_draw(x, y, v)` draws a 410-pixel bar, filled to v × 410 / 100, and a knob;
  - `battery_icon_draw` uses the level SYS+0x778 (2 pixels per step) and the charging flag
    SYS+0x779. That these bytes are the battery level and the charging state is DERIVED from
    the drawing;
  - also a window frame (0x40023D04), a small icon (0x40019C7C), and a full-screen image copied
    bottom-up from 0x42468A58 (0x40025D08).
- **Fonts:**
  - nine font descriptors of 0x4C0 bytes start at 0x42E64A98;
  - in a descriptor:
    - byte +0 is the kind (1 or 8);
    - +0x4BC points to the glyph data in the graphics blob;
    - `font_metrics_build` fills +2 (count), +4 (height), +6, +8, the glyph start table at +0xA
      and the width table at +0x260;
  - `font_char_draw` draws one glyph of the current font;
  - `char_to_digit_glyph` maps `0`..`9 . + -` to 0..12;
  - `glyph_14x14_draw` draws a 14 × 14 glyph of a two-byte code from the font at 0x40000040
    (code hi × 94 + lo − 0x4141). That this is GB2312 is DERIVED from the formula;
  - `text_shadow_draw` draws a string over an n × m block of offset copies in colour α << 24.
- **Pixels:**
  - `pixel_blend` is the blending path of `pixel_put`. For each channel it computes
    (A × pixel + (255 − A) × colour) / 255 in 8 bits;
  - `fb_rect_copy_to_e4e84` is the reverse of `fb_rect_copy_from_e4e84`.
- **Keys and trace cursors:**
  - `key_repeat_update` updates two repeat counters (SYS+0x86E, +0x870). A counter goes up when
    the previous press was less than SYS+0x87C ticks ago, and down otherwise;
  - `trace_cursor_limits` clamps the cursor and width values in 0x42E180C8 + 0x4C..0x5C, then
    passes them through `gen_limits`;
  - `trace_defaults_37b48` resets a group of SYS flags. It also fills a table at 0x42E554B0 of
    the values from 1,000,000 to 50,000,000 in steps of 200,000 (246 entries);
  - `trace_update_chain` runs both of them, then the redraw steps that the flags
    0x42E1810C..0x42E1810F select.

## Drawing primitives, message box, button groups (batch 33)

Spec: `tools/platform_spec/zb_draw_prims.py`, 14 functions. Tests: `tools/emu/tests/sweep.py`.

- **Primitives.** The colour block is at 0x42E6865C (+0 alpha, +1..+3 R G B, +4 RGB565, +8
  whole word).
  - `vline_draw(x, y0, y1)` draws y0..y1 inclusive;
  - `rect_fill(x, y, w, h)`;
  - `rect_outline_draw`: the firmware falls through into `rect_fill` for its last side;
  - `line_draw` uses vertical segments with a slope × 10000. When x1 = x0 the slope is 0, as the
    `sdiv` instruction gives.
  - When alpha is non-zero, `vline_draw` and `rect_fill` blend every pixel (`pixel_put` and
    `pixel_blend`). Otherwise they store the RGB565 value directly, with stride 1022.
- **Glyphs and text:**
  - `font_char_tint_draw` is `font_char_draw` with each glyph byte used as an intensity
    (anti-aliased text). It rewrites the colour block for every pixel and restores the colour
    word at the end;
  - `text_d6f8_shadow_draw` is the shadow wrapper of the text routine 0x4002D6F8.
- **Message box.** 0x40034134 was declared in `fpga.h` as "draws an image (OPEN)". It is
  `message_box_draw(x, y, text, align)`:
  - width = text width (0x4002D164) + 50, height 50;
  - align 0 centres the box above the point, align 2 puts it to the upper left;
  - it saves the area under the box first (`fb_rect_copy_to_e4e84`), keeps the rectangle in
    0x42E68C38 for the later restore (`rect_42e68c38_draw`), and draws the text with a shadow.

  So the "images" that the save routines of batch 31 show are messages. Their text pointers are
  words in the table at 0x42C2FE74. That this table holds the strings of the current language is
  DERIVED.
- **Button groups.** Five functions call `touch_button_update` for fixed groups of the button
  array at 0x42E61FF8. Three of the groups are as long as the list page size SYS+0xF9A.
- **Data:**
  - `marker_value_add(mode, v)` keeps a set of 24 s16 values at SYS+0x880, with counters at
    SYS+0x800 and SYS+0x801;
  - `channel_minmax_stats(ch)` computes the max, min, mean and range of the middle half of the
    samples of channel ch.
- Still in assembly:
  - 0x4002D164 (text width): for an invalid UTF-8 lead byte it uses a length register that
    was never set;
  - 0x400300E8, the counterpart of `marker_value_add`: when the value is absent it uses an
    index register that was never set.

## View and zoom, MMU, FFT, boot steps, window polling (batch 34)

Spec: `tools/platform_spec/zc_view_fft_boot.py`, 12 functions. Tests: `tools/emu/tests/sweep.py`.

- **View and zoom:**
  - `view_bounds_update` turns the offset words SYS+0x620 and SYS+0x624 (plus u16 SYS+0xC) into
    the visible start and end, clamped to the width SYS+8. The sums are 64-bit, so offsets near
    the int limits do not wrap;
  - `zoom_index_fit` steps the zoom index byte SYS+0x598 up while
    u64 SYS+0x590 / u64 SYS+0x5A0 / word SYS+0x5D4 is at least u16 SYS+0x2AC, then clamps it to
    SYS+0x2A8 .. SYS+0x2A7. Each step invalidates window 3.
- **MMU.** `mmu_map_sections(tbl, va, pa, size, attr)` writes first-level section descriptors of
  1 MB: 0xC12 (AP = 3, section) | pa section | C/B from attr bits 0..1. STATIC-CONFIRMED from the
  code. Which regions the boot maps with it is not yet listed.
- **FPGA.** `fpga_sel42_43_fill(n)`: selector 0x42 = 1, 130 pairs (1, 0) to selector 0x43,
  selector 0x42 = 0, then n pairs. What the FPGA does with it is OPEN.
- **FFT:**
  - `fft_input_prepare` takes n = SYS+0x7F8 points from the middle of the capture buffer
    0x42AC7CF4 (channel 2 at +0x62250), shifted right by SYS+0x264, as floats into 0x42E49EB4 and
    0x42E4DEB4, followed by n zeros (zero padding);
  - `fft_traces_draw` draws the s16 results at 0x42E5B014 and 0x42E5C014 as vertical segments,
    in 0xFF8800 (channel 1) and 0x88FF (channel 2).
- **Boot steps:**
  - `disk_check_file`: formats and reloads the disk, writes a 30-byte string into
    `sd_nand_check_file.bin`, reads it back, deletes it and sets byte 0x42E68AF0 to 1 when the
    two match;
  - `calibration_result_flags` maps the result of `system_calibration` (1, 0, -1, -2) to the
    bytes 0x42E68B01..0x42E68B03;
  - `factory_steps_run`: unless the state word 0x42E68AE0 is 0x5A5555A5, runs each of the three
    factory steps whose done-word is not set yet, saving after each, then reloads the settings.
    The marker values are STATIC-CONFIRMED; reading them as "factory flow complete" is DERIVED.
- **Windows:**
  - `screenshot_button_poll` saves a screenshot when touch button 0x42E62040 is released in an
    open window of type 1;
  - `window_touch_poll` calls the touch handler 0x40051E88 for the windows of the stack from the
    top down. It stops at the first window whose record byte +2 at 0x42E64520 is 0 (DERIVED: a
    modal window).
- **Cursor values.** `trace_cursor_values` converts the two cursor positions (words
  0x42E22D2C/30) into doubles at 0x42E22D38..0x42E22D78 (block 0x42E180C8 + 0xAC70..0xACB0;
  addresses corrected in batch 50). The horizontal values are 10^(base + pos / width × scale), so
  the axis is logarithmic (the frequency axis of the Bode plot, STATIC-CONFIRMED by
  `bode_grid_draw`, batch 50). The vertical values come from the per-x tables 0x42E20DE8 (gain)
  and 0x42E21D88 (phase).

## Button handlers, FPGA counters, level statistics (batch 35)

Spec: `tools/platform_spec/zd_buttons_levels.py`, 19 functions. Tests: `tools/emu/tests/sweep.py`.

- **Button handlers.** Each one updates its buttons in the array at 0x42E61FF8. On release
  (state 2) it clears the button and the touch byte, then acts:
  - delete the marked entries of list A, B or C and close its confirmation window (0x15, 0x19,
    0x1B);
  - `file_lists_open_buttons` opens window 0x14, 0x18 or 0x1A after resetting the window stack.
    For list A it calls `list_a_reload`, which ends by falling through into `list_a_page_thumbs`;
  - `language_select_buttons` sets the language byte SYS+0x7AC to 1..4, applies the texts and
    saves settings block B. The fourth button does not clear the touch byte (stock behaviour);
  - `format_button_handler` formats the disk and shows a message for 500 ms;
  - `calibration_button_handler` shows a message, runs `calib_run_4_channels`, shows a second
    message and waits 700 ms;
  - `button_71e_drag_handler` handles a drag on button 0x71E (state 1 = pressed). In drag mode
    0x12 it moves the s16 SYS+0x27E by the drag delta.

  The handlers call `ui_window_close` with one argument. Its hidden r2 (used only when the window
  is not on the stack) is whatever the previous call left, as in the stock code.
  `buttons_9f8_handler` passes r1 to `ui_window_invalidate_from` explicitly: the flag address on
  one path and the value 1 on the other.
- **Cursor.** `cursor_27e_clamp` clamps SYS+0x27E to m .. height - m, or 2m .. height - 2m.
- **FPGA counters.** Selectors 0x96 + 2i and 0x97 + 2i, for i = 0..7:
  - `fpga_counters_init` sends 130 steps to each counter;
  - `fpga_counter_step(ch)` sends the direction and |target - last| steps, then keeps the
    target.

  The step-pulse pattern (1, 0) suggests up/down counters or digital potentiometers. That is
  DERIVED; which part they drive is OPEN.
- **Levels.** `level_stats_5f010` and `level_stats_60008` turn the s16 samples at the pointer
  0x42E68CB0 into clamped screen levels (u16 arrays 0x42E5F010 and 0x42E60008). They keep the
  mean, max and min in the block 0x42E68668. The divisions follow `sdiv` (division by 0 gives 0).
- **Other functions:**
  - `fft_input_prepare_b` is a second FFT input into 0x42D164B4, using consecutive samples;
  - `fpga_position_290_update` sets SYS+0x290. It uses the 13-bit FPGA value of selector 0x14
    when flag 0x18 is set, otherwise the remainder (u64 SYS+0x30 - SYS+0x28C) mod SYS+0x520;
  - `measure_setup_62e14` runs four setup steps of the block 0x42E68668, then derives the
    doubles +0x140 / +0x148 and the s64 window +0x120 .. +0x138.
- Not taken: 0x40062354 reads r3 and ip as inputs (a fragment); 0x400447C8 ends in an endless
  loop after the watchdog reset; 0x40061608 takes a double in d0 (the harness has no FP
  arguments yet).

## More button handlers, level chain, FPGA parameters, generator self-test (batch 36)

Spec: `tools/platform_spec/ze_buttons_more.py`, 12 functions. Tests: `tools/emu/tests/sweep.py`.

- **Drags.**
  - `cursor_drag_buttons_b42` moves the two cursor words 0x42E22D2C / +4 in drag modes 0xE / 0xF.
    These are the cursor positions read by `trace_cursor_values` (batch 34);
  - `marker_drag_buttons_bd8` moves the words +0x5C / +0x60 of block 0x42E68668 in modes
    0x10 / 0x11.

  Both keep the 64-bit start in the drag block 0x42E68998 on press (state 1). They set
  start - word +0x30 while byte +1 of the button is 1.
- **Calibration buttons.**
  - `system_calibration_button_handler` runs `system_calibration` and shows one of four messages
    for the results -2, -1, 0 and 1;
  - `calibration_b4c_button_handler` runs 0x40039EB8 and picks its message from the bytes
    0x42E22DE1 / 0x42E22DE2.
- **Window and file buttons.**
  - `buttons_ac_open_windows` opens windows 0xD..0x12. Its last button toggles SYS+0x671 and
    passes the new value as r1 to `ui_window_invalidate_from`;
  - `buttons_b56_handler` and `buttons_ac0_handler` restore SYS_INFO and reopen windows 1 and 5.
    They also save a screenshot and toggle several flags;
  - `buttons_be2_handler` sets word +0xF4 of block 0x42E68668 to 0..3 and toggles its bytes +1
    and +2. Byte +1 is the gate of `level_stats_60008`.
- **Numpad fields.** `numpad_buttons_b06` opens the numpad for one of five fields. It sets the
  target in the block 0x42E68A88: a pointer to word 0x42E18114 + 4k, the field number in
  0x42E18110, and two cleared words. The four target words are rewritten even when no button
  fired.
- **Level chain.** `levels_update_chain` runs the level functions of batch 35 unless touch byte
  +0x17 is set.
- **FPGA parameters.** `fpga_params_0e_13_write` writes parameters 0x0E..0x13 for the channel
  selected by SYS+0x27C. Parameter 0x10 is the trace position SYS+0x27E, and the stock reads it
  as an unaligned word. It then calls `smec_commit(0xB8)` and `fpga_cmd17_write`.
- **Generator self-test.** `gen_selftest_64c28` sets four generator values in turn:
  0x2D32355 / 1800, 0x2D32355 / 1800, 0x1797D51 / 1300 and 0xBC84B1 / 1300. It checks each one
  with 0x4006453C and returns bit 0 of the AND of the four results. What the units of the values
  are is OPEN.
- Like batch 35, `ui_window_close` (and `ui_window_invalidate_from` after it in
  `system_calibration_button_handler`) gets the r1/r2 left by the previous call.

## Output and menu buttons, numpad keys, 64-bit window, table 0x684 (batch 37)

Spec: `tools/platform_spec/zf_buttons_tables.py`, 7 functions. Tests: `tools/emu/tests/sweep.py`.

- **Buttons:**
  - `output_select_buttons`: nine buttons set byte SYS+0x660 to 0..8 and apply the outputs;
  - `main_menu_buttons` opens windows 6, 0xA, 0xB and 0xC, toggles SYS+0x802 and SYS+0x803,
    and saves a waveform. Its last button resets the zoom and toggles SYS+0x2AA, but only when
    the zoom count SYS+0x588 is above 12.
- **Numpad.** `numpad_key(k)` handles the keys:
  - 1 toggles the sign (modes 2..4 only);
  - 2 adds the decimal point, once per value;
  - 3..12 add the digits '0'..'9', up to 10 characters;
  - 13 is backspace;
  - 14..16 close the numpad.
- **Levels.** `level_stats_61000` is the third level function, gated by byte +2 of the block
  0x42E68668. The stock entry is 24 bytes long and falls into the code at 0x40062354 with the
  block in r3. The C has the whole body.
- **64-bit window.** `window_64_recalc` keeps the s64 limits +0x120 / +0x128, centre +0x130 and
  span +0x138 of the block 0x42E68668 consistent. The mode word +0x18C selects how: from the
  limits (modes 1 and 2), or from the centre and span (modes 3 and 4).
- **Generator self-test.** `gen_selftest_64cf0` is the second generator self-test. It has the
  same shape as `gen_selftest_64c28`, with other values and the check function 0x400647F4.
- **Table.** `table_684_init` writes 64 fixed bytes (0x34..0x55) at SYS+0x684. The stock stores
  them one byte at a time; the C stores 16 words with the same final contents. What the table
  holds is OPEN.
- Not taken: several remaining functions in 0x40061xxx..0x40062xxx are long double-precision
  calculations with calls into the math library (log10, pow, l2d). They are left for a batch of
  their own.

## Floating-point level functions, FFT dB scale, frequency counter (batch 38)

Spec: `tools/platform_spec/zg_fp_levels.py`, 5 functions. Tests: `tools/emu/tests/sweep.py`.

The C keeps the stock order of the double operations. The build uses `-ffp-contract=off`, so a
`vmla` / `vmls` becomes a separate multiply and add with the same rounding. The math routines
(log10, pow, the 64-bit conversions) are the firmware's own and run inside the tests.

- **Markers.** `marker_values_compute` turns the two marker positions into:
  - s64 x values lo + pos / n * span, and their distance;
  - levels top - y / scale * step, and their distance;
  - linear values 1000 * 10^(level / 20), and their distance.

  The dB-to-linear step is DERIVED: the code computes 10^(level / 20), but the unit is not
  named anywhere.
- **FFT dB.** `fft_levels_db` converts the FFT magnitudes to screen levels:
  20 * log10(m * 6.916 / g * f / 1000 * 80 / g), shifted and scaled by the block 0x42E68668, then
  clamped to -4095..4095. `fft_db_traces_scale` is the simpler version for the two FFT traces:
  45 * log10(x), clamped to half the height.
- **Shaded levels.** `levels_shade_draw` draws the level graph. The colour comes from the
  271-entry table 0x42E56C30 and is dimmed above the mean level.
- **Frequency counter.** `channel_freq_measure(ch)` counts the crossings of a threshold by the
  channel samples over the middle half of the capture. With at least two crossings it stores:
  - the samples per period (float);
  - a 64-bit period: 2 * span * SYS+0x590 / SYS+0x1C * SYS+0x55C / SYS+0x554 / periods;
  - 10^12 / period.

  Otherwise it stores 1000.0f, 10^12 and 0. "Frequency counter" is DERIVED from the computation.
- The disassembly helper used for the specs now decodes the `vldr` literals. That gave the
  constants 6.916, 80, 238, 270, 45 and 1000, which batch 34 could not read.

## Channel, list, system, slot, trigger and numpad buttons (batch 39)

Spec: `tools/platform_spec/zh_settings_buttons.py`, 8 functions. Tests: `tools/emu/tests/sweep.py`.

All eight are touch handlers of the same shape: refresh the button group, then act on the first
button whose state is 2 (released), clearing it and the touch byte. The window and setting meanings
come from the names of the windows they open and the SYS bytes they write. They are DERIVED, not
HW-CONFIRMED.

- **Channels.** `channel1_settings_buttons` and `channel2_settings_buttons` share one layout:
  10 buttons, the channel-2 ones 0x64 further, and the SYS bytes 0xC4 further (0x3E.. and 0x102..).
  The enable byte is forced on while SYS+0x652 is set. Every change ends with `outputs_apply` and
  `ui_windows_invalidate_1_5`.
- **List A and list B windows.** `list_a_window_buttons` moves the current entry (SYS+0xFA4) by one
  and recomputes its page (SYS+0xF9E = entry / page size, hardware `sdiv`). `list_b_window_buttons`
  handles:
  - the entry buttons 0x840 + 10i, with a u8 index (a page size above 255 never ends the loop, in
    the stock code too);
  - mark mode SYS+0xF90 and the marks SYS+0x5DC6 + entry;
  - the page buttons.

  The next button offset is built by adding to or ORing into a running register:
  0x8D6, 0x8EA, 0x8F4, 0x8FE, 0x908.
- **System window.** `system_settings_buttons` opens sub-windows, enters USB mode, and sets the
  auto-off word SYS+0x658 to 0, 15, 30, 45 or 60. The unit is OPEN. The values look like minutes.
- **Settings slots.** `settings_slot_buttons` stores the slot number 1..5 in SYS+0x7C0. Then, by the
  window mode byte 0x42E64587 (1 save, 2 load, 3 save block F then load), it saves or loads, and
  shows message 0xAAA0 for 500 ms.
- **Trigger options.** `trigger_option_buttons` sets SYS+0x27A (0..2), +0x27B, +0x27C (0..1) and
  +0x284 (0..3). Each change invalidates from window 3.
- **Measurement numpad.** `measure_numpad_buttons` opens the numpad for one field of the
  measurement block 0x42E68668: two doubles at +0x140 and +0x148, or four s64 at +0x120..+0x138
  (mode +0x18C = 1..4). It sets the numpad target, callback 0x400617AC and flag.
- **Test harness.** The call-mismatch message of `tools/emu/emu.py` now starts at the first
  differing call, instead of always showing the first three. Where `ui_window_invalidate_from`
  follows other callees, r1 is the previous callee's leftover, so the two list tests record only
  its first argument (the convention of batch 35).

## List A / C windows, settings buttons, numpad keys, generator self-tests (batch 40)

Spec: `tools/platform_spec/zi_lists_selftests.py`, 7 functions. Tests: `tools/emu/tests/sweep.py`.

- **List windows.** `list_a_thumbs_buttons` and `list_c_window_buttons` follow the layout of
  `list_b_window_buttons` (batch 39), so the spec builds all of them from one template
  (`list_window`):
  - entry buttons, the back button, mark mode, mark page, delete, page - and page +;
  - entries select page * size + i; in mark mode they toggle the mark, otherwise they open the entry.

  The windows differ in what an entry opens. List A loads the entry and opens window 0x16. List C
  copies the entry's record number (halfword SYS+0xFA6 + 2 * entry) to 0x4091F396, selects that
  record and loads the generator samples.
- **Button groups 0x20A and 0x5C.** `buttons_20a_handler` has:
  - a slider: SYS+0x654 = 100 * (x - lo) / (hi - lo), with the touch x clamped to the slider
    limits;
  - toggles of SYS+0x652, +0x653, +0x655;
  - buttons that back up the settings and open the window sets 0x22..0x24 and 0x26..0x28.

  `buttons_05c_handler` toggles SYS+0x3C, resets the word pair 0x570 / 0x578, and steps the byte
  SYS+0x41 (or +0x105) within 0..SYS+0x1BA - 1. "Brightness" for SYS+0x654 would be a guess; the
  names stay neutral.
- **Numpad keys.** `numpad_key_buttons` maps the 16 buttons 0xA20..0xAB6 to the key codes
  3..0xC, 1, 2, 0xD..0x10 of `numpad_key` (batch 37).
- **Self-tests.** `gen_selftest_504b8` and `gen_selftest_50688` put the generator (block
  0x42E3F984) into one or two settings: amplitude 1000, or +/-2500. For each setting they:
  - apply it, then run two acquisitions;
  - compare the s64 results SYS+0x8F8 and SYS+0x958, divided by 1000 (`__aeabi_ldivmod`), with a
    window: within +/-399, or at least 1500 / below -1499;
  - store the pass flags in bytes 0x42E68B07..0x42E68B10.

  The tests stub the acquisition and feed values right at the thresholds. They also record the
  settings each step applies, because the second step overwrites them.
- **Test harness.** Two new helpers in `sweep.py`. `one_btn` releases a single random button of
  the group. `slider40` picks x right at the slider limits. Both make the stock edge cases
  frequent enough for the 80-case mutant runs.

## Self-tests, touch-target test, window touch dispatch, generator FPGA writer (batch 41)

Spec: `tools/platform_spec/zj_selftests_dispatch.py`, 5 functions. Tests: `tools/emu/tests/sweep.py`.

- **Self-tests.** `gen_selftest_50bac` and `gen_selftest_50ddc` join the self-tests of batch 40.
  Both store their pass flags in the result block 0x42E68AE0.
  - `gen_selftest_50bac` measures, per channel, the spread (max - min) of the capture bytes at
    0x42DF76D4, summed over 30 acquisitions. It does this with the output setting SYS+0x46 /
    SYS+0x10A at 0 and at 1. A channel passes when the spread with setting 1 is below 0.9 times
    the spread with setting 0.
  - `gen_selftest_50ddc` compares 5 * (result / 1000) with 4 * 1000.

  What each test checks physically is DERIVED from the settings it writes. It is not
  HW-CONFIRMED.
- **Touch test.** `touch_targets_test` draws five 80 x 80 squares: the four corners and the
  centre of the 1024 x 600 screen. It polls the GT911 until every square has been touched, and
  repaints each square as it is touched. The tests feed touches right at the edges of the squares
  and always end with the five centres, so the loop terminates.
- **Window touch dispatch.** `window_buttons_dispatch(id)` is the switch that routes a touch to
  the window's button handler. Ids 1..42 map to the handlers rewritten in batches 35-40, with no
  handler for id 4. Every id is then passed to `ui_window_touch_handle`. This table links the
  window ids to their handlers.
- **Generator FPGA writer.** `gen_fpga_settings_write` sends the generator block 0x42E3F984 to the
  FPGA:
  - the sample clock: min(limit, rate * points), multiplied by 10 below 2500, with a x10 flag;
    it goes to the MS5351 clock 1;
  - two 16-bit words, the point count - 2 and a mode flag;
  - every sample as index (selector 0x3F) + value (0x40), with a strobe on selector 0x3E.

  It is skipped while byte 0x42E68BC7 is set.

## Output-level and capture-lane self-tests, numpad parser, measurement limits (batch 42)

Spec: `tools/platform_spec/zk_selftests_numpad.py`, 4 functions. Tests: `tools/emu/tests/sweep.py`.

- **Self-tests.** Their purpose is DERIVED from the settings they write; neither is HW-CONFIRMED.
  - `gen_selftest_4f89c` sets four output configurations. Each is centred on SYS+0xE with
    offsets +/- 2 * SYS+0x20 and has the channels on or off. It checks the measured words SYS+0xB34
    and SYS+0xB50 against e * 1.3 and e * 0.7 (doubles).
  - `selftest_4f5e4` steps the FPGA 0x2A value (SYS+0x44) 900 times from 3000 with a 7.777...
    step, and records one byte of each of the eight capture lanes per step. A lane passes when
    every bit is seen both set and clear, and every pair of neighbouring bits is seen different.
    That looks like a test of the eight data lines between the ADC and the FPGA.
- **Numpad text parser.** `numpad_value_parse(unit)` turns the numpad text into a double. The text
  is an optional '-', digits, then a '.' that switches to decimals. Then:
  - the result is scaled by the numpad mode and the unit key: x1, x1000, x1e6 or x10, or the old
    value is kept;
  - it is stored to the target pointers as a double, as an s64 clamped to -0x7FFFFFFF..0x7FFFFFFF,
    and as the low word;
  - the callback runs.

  The lower limit is -0x7FFFFFFF, not INT32_MIN. The literal `#96, #6` is 0x80000001, which was
  first misread here as 0x80000000.

  `numpad_key` (batch 37) now passes its key to this function, as the stock code does in r0.
- **Measurement limits.** `measure_values_clamp` is the numpad callback of the measurement window:
  - the four s64 values go to 0..1e9, before and after `window_64_recalc`;
  - the two doubles go to -60..260 and 1..260.

  The upper limit is tested against 260 plus one ulp, so a value of exactly 260 is left as it is.
- **Disassembly helper.** The disassembly helper used for the specs now also decodes the
  conditional `vldr` literals (vldrge, vldrmi).

## Trace segment scaling, level-column graphs, channel 1 level search (batch 43)

Spec: `tools/platform_spec/zl_trace_scale_columns.py`, 4 functions. Tests: `tools/emu/tests/sweep.py`.

- **Trace scaling.** `trace_scale_segment(tr, start, len)` is the core of `trace_span_full` /
  `trace_span_window`. It turns raw u16 samples into screen rows with fixed-point arithmetic:
  y = base + ((off + ((x - off - pos) * gain >> 10)) * mul >> 12), clamped to min..max.
  - The gain, position and offset come from channel set 0 or 1 (SYS+0xE4.. or SYS+0x1A4..).
  - mul / base / range select the full screen, or one part of the split screen (SYS+0x2AA): the
    upper two thirds (mul 0xAAB) or the lower third (mul 0x555).
  - The division by 3 appears as the usual `smull` by 0x55555556 and `* 0xAAAB >> 17`.
- **Level-column graphs.** `level_columns_draw` and `level_columns_draw_b` paint the columns of
  the two level graphs with a colour ramp (tables 0x42E56C30 and 0x42E583A0), then copy the
  buffer to the screen (stride 1022 pixels).

  In `level_columns_draw` the start pixel of each column does not move with the step. Each step
  repaints the same M+0x184 pixels from the bottom row, so only the last colour is left. The C
  keeps this; whether it is a firmware bug is OPEN.
- **Level search.** `auto_level_search` puts channel 1 into a fixed state, then steps the level
  byte SYS+0x41 down (to 0) or up (to 7) until the measured word SYS+0xB30 is between 0.2 and 0.9
  times the screen height. After that it prepares the FFT input and scrolls the two
  level-column buffers up by M+0x184 rows.
- **Skipped.** 0x40060BA4 and 0x40060DC8 stay unrewritten. For a mode word M+0xF4 above 3 they
  use registers that were never set (r0..r2 and fp from the caller, and stack slots), like the
  functions skipped earlier for the same reason.

## Text and number rendering (batch 44)

Spec: `tools/platform_spec/zm_text_numbers.py`, 6 functions. Tests: `tools/emu/tests/sweep.py`.
This batch was chosen because any new screen or app needs it: with it, every function needed to
put text and numbers on the screen is in C.

| Function | Signature (DERIVED names) | What it does |
|---|---|---|
| `text_draw` 0x4002CE7C | `void (u32 x, u32 y, const char *utf8)` | draws a string in the current font, stores width + 1 in u16 0x42E68BE4 |
| `text_width` 0x4002D164 | `u32 (const char *utf8)` | width in pixels, without drawing |
| `number_draw` 0x4002D82C | `void (u32 x, u32 y, s64 v, u8 dec, s8 sign_mode)` | prints a fixed-point number |
| `number_text_width` 0x4002D2D4 | `u32 (s64 v, u32 dec, u32 sign_mode)` | width of the same text |
| `font_glyph_index` 0x4002DD70 | `u16 (u32 code)` | character to glyph index for font type 1 |
| `glyph_alpha_draw` 0x4002EE78 | `void (u32 x, u32 y, const u8 *glyph)` | anti-aliased (alpha-mask) glyph |

- **Current font and state.** The font descriptor is the word 0x42E68C74 (set by
  `word_42e68c74_set`). Byte +0 is the font type: 1 uses `font_glyph_index` and `font_char_draw`;
  8 is the digits-only font (`char_to_digit_glyph`, `font_char_tint_draw`). Other state: glyph
  widths at u16 +0x262 + 2 * glyph, fixed width u16 +8, spacing u16 0x42E68BE2
  (`half_42e68be2_set`), centring byte 0x42E68BE0 (`byte_42e68be0_set`).
- **UTF-8.** Strings are decoded as UTF-8 (1 to 4 bytes); only the low 16 bits of the code are
  used. In font type 1, a character with no glyph is drawn with the 14 x 14 font
  (`glyph_14x14_draw`, code from table u16 0x42CF64B4[code]) and advances 14 pixels.
  - An invalid lead byte (0x80..0xBF, 0xF8..0xFF) gives code 0. The stock code then keeps the
    length of the previous character; for the first character that is register r9 of the caller.
    The C starts at 1, and the tests never start a string with such a byte.
- **Glyph table.** `font_glyph_index` covers 186 codes:
  - 'A'..'Z' = 0..25 and 'a'..'z' = 26..51;
  - Cyrillic = 52..99;
  - Ä Ñ ñ Ö Ü ß ä ö ü ẞ = 100..109;
  - '0'..'9' = 110..119;
  - punctuation = 120..134.

  Latin vowels with accents or pinyin tones (à á ã è é ê ā ě ǎ …) map to the plain letter. The
  stock code is a 186-case switch. The C table was generated by running the stock function on
  every 16-bit input in the emulator; the test also checks 32-bit inputs.
- **Numbers.** `number_draw` and `number_text_width` take 16 decimal digits with 64-bit divisions.
  - The integer part keeps its units digit and drops the zeros above it; `dec` digits follow a
    '.' when `dec` is not 0.
  - Sign mode: 0 prints '-' only, 1 prints '-' or '+', any other value prints no sign.
  - Example: v = -12345, dec = 2, mode 0 gives "-123.45".
  - `dec` must be 0..15: the digit array has 16 entries.
  - The wrappers `func_4002d82c_flag0/1` and `func_4002d2d4_d0` fix the sign mode.
- **Alpha glyphs.** `glyph_alpha_draw` reads u16 big-endian width and height, then one coverage
  byte per pixel. Coverage >= 0xF6 stores the solid colour u16 0x42E68660, coverage < 3 is
  skipped, and anything else blends the colour bytes 0x42E6865D..F into the screen pixel. The
  coverage is first scaled by 255 - byte 0x42E6865C and lowered by u16 0x42E68BE6.

## SD-card update check, flash read and read-modify-write (batch 45)

Spec: `tools/platform_spec/zn_sd_update.py`, 3 functions. Tests: `tools/emu/tests/sweep.py`.

| Function | Signature (DERIVED names) | What it does |
|---|---|---|
| `sd_update_check` 0x4004F134 | `void (void)` | looks for an update `.iso` in the SD root; called once by `main` (0x40034700) |
| `persistence_read` 0x40043620 | `int (u32 dev, u32 addr, u8 *buf, u32 len)` | SPI flash read; 0 or -1 |
| `persistence_write_read_modify_write` 0x40043874 | `void (u32 dev, u32 addr, const u8 *buf, u32 len)` | SPI flash write that keeps the rest of each 4 KB sector |

- **What `sd_update_check` does** (STATIC-CONFIRMED from the code):
  1. `f_opendir("0:")`, then `f_readdir` until an error or an empty name. A name matches when
     the text after its **first** '.' is exactly `iso` in lower case and at least one character
     comes before the dot (`fw.iso` and `DPOS350P_TEST.iso` match; `x.ISO`, `a.b.iso` and `.iso`
     do not). On a match the directory is closed and the file is opened for reading. The scan
     then goes on, but `f_readdir` on the closed directory fails, so it stops.
  2. The size (low 32 bits only) must be at least 1000 bytes, and the first 4 KB, read into
     0x41E7B9C8, must start with `DPOS350P` (string at 0x40093EF0).
  3. The rest of the file (from offset 0x1000) is read into 0x41E7B9C8 over the header. Its last
     36 bytes are checked against the footer described in `ISO_FORMAT.md`: 9 big-endian sums.
  4. All nine match: the 4 KB buffer 0x42E59B10 is cleared, written to SPI flash 0xA8000
     (`persistence_write_read_modify_write(0, 0xA8000, buf, 0x1000)`), and
     `wdt_system_reset` restarts the device. The file is kept.
  5. Any check fails (too small, wrong header, wrong footer): the file is **deleted**
     (`f_unlink`).
- **Footer confirmed on the stock file.** `build/isochk45.py` runs the same check on
  `build/DPOS350P.iso`: the header is `DPOS350P` and all nine sums match. The format that
  `tools/dposiso.py` packs is therefore the one the firmware accepts (STATIC-CONFIRMED).
- **Safety note.** A `*.iso` in the SD root is acted on at every start: a valid one starts an
  update and an invalid one is deleted. `DPOS350P_TEST.iso` has a valid footer and a matching
  name, so copying it to the SD root would install it. It must never be put on the card.
- **OPEN.** Who reads the flash sector at 0xA8000 after the reset, and how the update is
  installed. 0xA8000 occurs only as this one immediate in the application, so it is probably
  read by the boot code, which is not part of this image. Without that code the full update
  path cannot be reproduced.
- **`persistence_read`**: command 0x03 + 3 address bytes, or 0x13 + 4 address bytes when the
  word 0x42E68CD4 (4-byte addressing) is set. When the word 0x42E68CC4 is 1, the command is 0x3B
  / 0x3C (dual-output fast read); when it is positive, a dummy byte follows the address. After
  the read the word is set back to 0, so the mode lasts for one read only.
- **`persistence_write_read_modify_write`**: per 4 KB sector, it reads the sector into
  0x42E5E010.
  - When every target byte is 0xFF, it programs the new bytes directly.
  - Otherwise it erases the sector, merges the new bytes into the copy and programs the whole
    sector.
  - The first chunk runs to the end of the sector; later chunks are up to 0x1000 bytes.

## Dialogs, menus, XY grid, markers and time cursors (batch 46)

Spec: `tools/platform_spec/zo_dialogs.py`, 14 functions. Tests: `tools/emu/tests/sweep.py`.
These are complete screens, so they show how a screen is put together.

- **How a screen is built** (STATIC-CONFIRMED from the code):
  1. The box comes from the window rectangle table 0x42E64520 (u16 x, y, w, h per window).
  2. Set the font (`word_42e68c74_set(0x42E658D8)`), the character spacing
     (`half_42e68be2_set`) and the colour (`draw_colour_set`, colours from a table such as
     SYS+0x3DD54C or SYS+0x3DD550).
  3. Draw with `rect_fill`, then texts with `text_shadow_draw(x, y, s, 1, 2, 0)`. A text comes
     from the UI table, whose x/y bytes +0x194/+0x195 nudge it for the current language.
  4. Add a touch area with `touch_button_set_rect(x, y, w, h, 0x42E61FF8 + slot)`. When the
     slot byte +1 is set, the button is drawn again in the "pressed" colour.
- **UI texts.** `build/uitext46.py` runs the stock filler 0x40045974 in the emulator and lists
  every entry with its Chinese and English text (output in `build/uitext46.txt`, not committed).
  Entries used here:
  - 11 'USB connect' and 67 'Return';
  - 91 'Notice !', 92..93 'Please unplug all signal' / 'proble and USB first !', 95 'Confirm';
  - 102..103 'Restore to factory' / 'settings ?';
  - 106 'Warning !', 107..108 'All saved data will' / 'be clear !';
  - 118 'Delete selected', 120 'items';
  - 126..127 'Please connect all channels' / 'to the signal generator !'.

| Function | Window | What it draws |
|---|---|---|
| `ui_delete_confirm_164` / `_180` / `_12c` | +0x164 / +0x180 / +0x12C | 'Delete selected <n> items ?' (n = u16 SYS+0x88BE) and 'Confirm' |
| `ui_unplug_notice_19c` / `_252` | +0x19C / +0x252 | 'Notice !' unplug probes and USB, 'Confirm' |
| `ui_factory_reset_confirm` | +0x1AA | 'Notice !' restore factory settings ?, 'Confirm' |
| `ui_clear_data_warning` | +0x1AA | 'Warning !' all saved data will be cleared, 'Confirm' |
| `ui_connect_generator_notice` | +0x20C | 'Notice !' connect all channels to the generator, 'Confirm' |
| `ui_usb_connect_screen` | +0x1C6 | 'USB connect' screen with 'Return' and a fixed picture |
| `ui_language_menu_draw` | +0x84 | four rows: Chinese, English, Russian, Portuguese; selected row = SYS+0x7AC (1..4) |
| `ui_colour_buttons_draw` | +0x18E | four 64 x 150 buttons, each with an image in its own colour |

- **Trace pieces:**
  - `xy_grid_draw`: the XY-mode square (side u16 SYS+0xA, centred in the trace area);
    grid lines every u16 SYS+0x20 pixels when SYS+0x655 is set, 40 ticks 12.6 pixels apart
    on the centre lines, and the border.
  - `trace_markers_draw`: the channel markers '2' (left edge, height from word SYS+0x120,
    clamped to the area) and '1' (top edge, x from word SYS+0x60).
  - `time_cursors_draw`: the time cursors C1 / C2. The state is at SYS+0x3DA52C: enable byte
    +0, channel byte +4, positions +8 and +0xC. It draws dotted lines, the 'C1' / 'C2' markers
    and a 'T-CH1' / 'T-CH2' label between them.
- **Generated gates.** `func_40024c60_unless_t17` and `func_40028364_unless_t17`
  (`w_small_sweep.py`) now call the new names.

## Automatic measurements, SI scaling, measurement panel, manual cursors (batch 47)

Specs: `tools/platform_spec/zp_measure.py`, 5 functions. Tests: `tools/emu/tests/sweep.py`.
All facts here are STATIC-CONFIRMED from the code and checked in the emulator (EQUIVALENT).
What the values mean on the real probe is DERIVED and was not measured on the device.

- **`measure_channel(ch)`** (0x40032E0C) computes the 12 measurements of channel 0 or 1 from
  the capture buffer (word SYS+0x7B4 + 4*ch, sample count u16 SYS+0x7BC).
  - Channel 0 / channel 1:

    | Data | Channel 0 | Channel 1 |
    |---|---|---|
    | enable byte | SYS+0x3E | SYS+0x102 |
    | offset (s32) / scale (s64) | SYS+0x60 / SYS+0x50 | SYS+0x120 / SYS+0x110 |
    | raw block (u32: max, min, range, mid, glitches, -, hysteresis) | SYS+0xB28 | SYS+0xB44 |
    | results, 12 x s64 | SYS+0x8E8 | SYS+0x948 |
    | sums for the average, 12 x s64 | SYS+0xA68 | SYS+0xAC8 |
    | averages, 12 x s64 (what the screen shows) | SYS+0x9A8 | SYS+0xA08 |
    | counter / limit (u16) / "average ready" byte | SYS+0x86A / SYS+0x86E / SYS+0x872 | SYS+0x86C / SYS+0x870 / SYS+0x873 |

  - Result index (measurement number - 1) and unit:

    | # | Label | Meaning | Unit |
    |---|---|---|---|
    | 1 | MAX | highest level | uV |
    | 2 | MIN | lowest level | uV |
    | 3 | AVG | mean | uV |
    | 4 | RMS | root mean square | uV |
    | 5 | VP | peak (MAX or MIN, whichever is larger in size) | uV |
    | 6 | VPP | MAX - MIN | uV |
    | 7 | F | frequency = 1e15 / period | mHz |
    | 8 | T | period | ps |
    | 9 | T+ | high time | ps |
    | 10 | T- | low time | ps |
    | 11 | Dt+ | duty cycle, high | 1/10000 (100.00 %) |
    | 12 | Dt- | 10000 - Dt+ | 1/10000 |

  - The samples are split into W32(SYS+0x558) interleaved banks. Edges are found against the
    trigger level S16(SYS+0x27E) when SYS+0x27C is this channel, else against the middle level;
    a jump larger than the hysteresis word (raw block +0x18) counts as a glitch. A bank with at
    most one edge each way is "invalid" and uses all its samples for RMS and mean.
  - The time base is Q64 SYS+0x590 divided by u16 SYS+0x1C, the level scale divides by u16
    SYS+0x20. Each call adds the results to the sums; when the counter reaches the limit the
    averages are written, the sums cleared and the "ready" byte set.
- **`value_si_scale(kind, -, v, &out, &dec, &prefix)`** (0x400301E0) chooses the divisor,
  decimals and prefix index (0..4 = u, m, -, k for volts; mHz, Hz, kHz, MHz, GHz; ps, ns, us,
  ms, s). Kinds 1..6 truncate, kinds 7..10 round half up. Trailing zero decimals are dropped.
- **`measure_item_draw(x, y, ch, kind)`** (0x40030AF8) draws one readout: grey label, a sign for
  MAX / MIN / AVG / VP, the number in font 0x42E66718 and the unit. Colour 0xFFFF00 for channel 0,
  0x00FFFF for channel 1. The stock unit strings are "KHz", "pS", "nS", "uS", "mS", "S".
- **`measure_panel_draw()`** (0x40030F8C): the list of chosen measurements is 24 s16 at
  SYS+0x880 (0 ends the list, a negative item means channel 1). Six per row, rows going up from
  y = 0x206, boxes in 0x44333333 (alpha 0x44).
- **`manual_cursors_draw()`** (0x4003117C): the H1/H2 time cursors (enable SYS+0x802,
  x positions SYS+0x808 / SYS+0x80C) and the V1/V2 level cursors (enable SYS+0x803,
  y positions SYS+0x810 / SYS+0x814), dotted grey lines with markers.
- **For a new app.** To add a measurement readout: enable the channel, put the item number in
  the list at SYS+0x880 and let the stock loop call `measure_channel`; the average at
  SYS+0x9A8 / SYS+0xA08 + 8*(n-1) is the value to show.
- **OPEN:**
  - 0x400300E8 (remove an item from the measurement list) shifts the list starting from an
    uninitialised register when the item is not found; not rewritten.
  - `measure_item_draw`: for |v| >= 1e14 the stock code leaves the decimals and prefix
    uninitialised; here they start at 0. Kinds outside 1..12 read the caller's registers and are
    not tested.

## Saved files: names, previews, saved trace, record list (batch 48)

Spec: `tools/platform_spec/zq_files.py`, 7 functions. Tests: `tools/emu/tests/sweep.py` and, for
the whole-screen preview, the new suite `tools/emu/tests/heavy.py` (6 cases, about 2 s each).
The code facts are STATIC-CONFIRMED and checked in the emulator (EQUIVALENT). What a buffer is
used for is DERIVED from the callers; the files themselves were not opened from a device.

- **File names** (`screenshot_path_make(n)` 0x40034B78, `waveform_path_make(n)` 0x40035BC4).
  - They write the path of entry n to the path buffer SYS+0x88C0 (0x4054D728): `/Screenshots/<n>.bmp`
    or `/Waveform/<n>.wav`, with n in decimal and no leading zeros.
  - The directory strings are at 0x4008F82C and 0x4008FFB4.
  - From n = 65536 on, only the '/' is written and the name is left unterminated (stock behaviour).
  - The list code (`file_exists_*`, `list_*_prune_missing`, `list_*_page_thumbs`, the save
    functions) calls these before `f_open`.
- **Previews (thumbnails).** Both build an RGB565 picture at SYS+0x109684 (0x4064E4EC): u16 width,
  u16 height, then the pixels from SYS+0x109688. The save code writes that block into the file,
  and the file list shows it again (`list_*_page_thumbs` copies width x height halfwords from it).
  - `screenshot_thumb_make` (0x40034E94) copies the scanned-out framebuffer 0x41FB8FE8 (1022 x 600)
    to 0x42210D20. It then box-filters the copy with 6 x 5 samples every 5.035 x 4.348 pixels,
    which gives a 203 x 138 preview.
  - `waveform_thumb_make` (0x40035EE0) reads the trace area (SYS+4 .. SYS+0xA) of the framebuffer
    directly, with 5 x 4 samples every 4.685 x 3.645 pixels. Both ends of the area are included.
  - Coordinates are in 1/1000 pixel. Each channel is (sum x 1e6) / step x / step y, clamped to
    31 / 63 / 31. The stock code does the divisions with reciprocal multiplications, and the C keeps
    them.
- **Saved trace** (`saved_wave_trace_build` 0x40036864).
  - State at SYS+0x3DA530: buffer index byte, first and end column, and their difference.
  - The saved sample buffers are 0x927C0 bytes each, from 0x411AF3E8.
  - Each column is the mean of W32(SYS+0x5F8) samples, divided by W32(SYS+0x1C0) and turned
    upside down against the trace height. The result goes to the u16 column array at
    SYS+0x3DA548.
- **Record list** at SYS+0xD744.
  - Layout: u16 SYS+0x758 slots of W32(SYS+0x3DA544) u16 cells, each cell holding one byte.
  - The id of a slot is the low bytes of cells 0 and 1; a free slot has id 0.
  - `record_append` (0x40036DE4) writes into the first free slot: the id from u16 SYS+0xF9C,
    then the length word SYS+0x3DA53C, then that many cells copied from SYS+0x3DA548.
  - `record_clear` (0x40037020) finds a slot by its id and calls `copy_u16(slot, 0, size)`, which
    copies from address 0 over the slot. It was probably meant as a fill; the stock code does it
    this way.
  - OPEN: if no slot matches, both functions use a register the caller left behind (r2 or r3)
    as the index. The C takes it as an extra argument, so the behaviour is the same at the ABI
    level. What the callers have in that register was not traced.
- **For a new app.**
  - To save a picture the way the stock code does: call `screenshot_thumb_make`, then
    `screenshot_path_make(n)`, and write the file. `func_4005c3f0` (not yet rewritten) writes the
    .bmp.
  - The preview block at SYS+0x109684 is the format the file browser shows.

## Screenshot .bmp, trace zoom, list C preview, number text (batch 49)

Spec: `tools/platform_spec/zr_bmp.py`, 4 functions. Tests: `tools/emu/tests/sweep.py`.
The code facts are STATIC-CONFIRMED and checked in the emulator (EQUIVALENT). The .bmp layout
is read from the code; no file written by the device was opened.

- **`bmp_save(path)`** (0x4005C3F0) builds the whole file in the buffer 0x41D3E3A8 and writes it
  with one `f_write` of 0x13D620 (1,300,000) bytes, passing a null byte-count pointer. It returns 1
  on success.

  | Offset | Content |
  |---|---|
  | 0x00 | 'BM', file size 0x13D620, pixel data offset 0xDEA8 |
  | 0x0E | BITMAPINFOHEADER: 40, width 1022, height 600, 1 plane, 16 bits, compression 3 (BI_BITFIELDS), image size field 0xBB800 (not the real size, 1022 x 600 x 2 = 0x12B6A0) |
  | 0x36 | masks R 0xF800, G 0x07E0, B 0x001F, then a zero word |
  | 0x46 | **preview**: u16 width, u16 height, then the RGB565 pixels (the block made by `screenshot_thumb_make`) |
  | 0xDEA8 | the 600 rows of the screen copy at 0x42210D20, bottom row first (rows of 2044 bytes) |

  - This explains `list_*_page_thumbs` (batch 30), which reads the preview back from the file.
  - It also explains the loader that reads 0x12B6A0 bytes from offset 0xDEA8.
  - A normal BMP viewer skips the gap before 0xDEA8, so the files open on a PC (DERIVED).
- **`simple_integer_index_mapping_nearest_neighbour(src, center, w, m, shift)`** (0x4005C678).
  The name comes from an earlier project, but the code does linear interpolation, not nearest
  neighbour. It is the horizontal zoom of a trace:
  - n = w x 1.2 x k / m source samples around src[center] (k = W32(SYS+0x554)) are stretched to
    m x n / k samples in the buffer 0x416431E8, in 8-bit fixed point;
  - w of those samples, centred and moved back by `shift`, are copied back over src;
  - nothing happens when k >= m.
- **`list_c_record_preview(a)`** (0x4003754C) turns the selected list C record into a preview
  curve:
  - the record is decimated to at most 193 points, centred in the zoom buffer, then stretched;
  - the minimum is subtracted and the values are scaled to 8 + 122 x v / range;
  - the result goes to the preview block at SYS+0x109684 (count, then values).
- **`number_text_88c0(n)`** (0x40037160) writes n in decimal to SYS+0x88C0, using the same digit
  code as the path helpers of batch 48.
- **Not tested** (the stock code divides by these values): k = 0 in the zoom, and a record length
  of 0 in the preview.

## Value + unit drawers, Bode plot grid and cursors (batch 50)

Spec: `tools/platform_spec/zs_bode.py`, 6 functions. Tests: `tools/emu/tests/sweep.py`.
The code facts are STATIC-CONFIRMED and checked in the emulator (EQUIVALENT). Reading the block
0x42E180C8 as the Bode plot is DERIVED from the labels ("deg", Hz, the log10 minor lines); the
screen was not seen on the device.

- **Value + unit drawers.** All take (x, y, sg, double d). The number goes through
  `number_draw` (flag 1 when the low byte of sg is set, else flag 0; DERIVED: the sign is shown),
  and the unit is drawn at x + the last drawn width (u16 0x42E68BE4) + 4.

  | Function | Input | Shown as |
  |---|---|---|
  | `phase_deg_draw` (0x40038788) | degrees, clamped to ±180 | 2 decimals below 10, 1 below 100, then 0; unit "deg" |
  | `voltage_uv_draw` (0x40038974) | microvolts | "uV" below 1000, "mV" (no decimals) below 10^6, "V" with 2 / 1 decimals below 10^7 / 10^8; nothing above |
  | `level_value_draw` (0x40038C00) | any, clamped to ±999 | 2 / 1 / 0 decimals as above, no unit (`level_label_db` / `level_label_dbmv` add "dB" / "dBmV") |
  | `frequency_draw` (0x40038E40) | Hz | "Hz", "KHz", "MHz" (2 / 1 / 0 decimals), "GHz" (2 decimals) below 10^10; nothing above |

  - The values are cut to an integer number of thousandths first (degrees, levels) or of units
    (voltage, frequency), truncating toward zero.
  - `frequency_draw` with the low byte of its third argument set draws nothing and returns the
    width the text would take (number width + unit width + 4); this is how the callers centre or
    right-align labels.
  - Its rounding is odd: after dividing, it adds 1 when the last *kept* digit is 5 or more
    (1050 Hz shows as 1.06 KHz). Stock behaviour, kept.
- **Bode plot block** (0x42E180C8). u16 +0 x, +2 y, +4 width, +6 height, +8 decade width, +0xA
  number of decades, +0xC row height, +0xE number of rows. Doubles: +0x10 / +0x18 gain top /
  bottom (dB), +0x20 / +0x28 phase top / bottom (degrees), +0x8D08 / +0x8D10 log10 of the start /
  end frequency. Byte +0xAC60 = cursors on, words +0xAC64 / +0xAC68 = cursor x positions.
- **`bode_grid_draw`** (0x40038374), skipped while touch byte +0x17 is set:
  - clears the area, draws a double 0x444444 outline, points touch button 0x42E62B3A at the plot;
  - a 3-pixel line at each decade, minor lines at x + decade width × log10(k) for k = 2..9, and
    the row lines;
  - gain labels on the left (x = 2), phase labels on the right (x = 943), one per row line, the
    first and last moved 7 pixels inwards;
  - 4 frequency labels 10^(start + i × (end − start) / decades) under the plot. The first is left
    aligned, the last right aligned, the others centred on their decade line. Only 4 labels are
    drawn whatever the number of decades.
- **`bode_cursors_draw`** (0x40037C3C), when the cursors are on:
  - two dashed 0xBBBBBB vertical lines and a tag "C1" / "C2" with a small triangle on each;
  - three text rows under the plot, 17 pixels apart: "C1 = ", "C2 = " and "DC = " (the
    difference), each with the frequency (0xBBBBBB), the gain in dB (0xFF8800) and the phase
    (0x0088FF). The values are the doubles computed by `trace_cursor_values`. The C1 and C2 rows
    use the signed number format, the difference row does not.
- **For new apps:** a frequency-response app can reuse the block layout, `bode_grid_draw` and
  the cursor panel as they are, and `frequency_draw` / `voltage_uv_draw` are ready-made unit
  formatters (DERIVED: no app was written yet).

## Bode plot measurement: sweep, point, phase, calibration, scale, curves (batch 51)

Spec: `tools/platform_spec/zt_bode_sweep.py`, 6 functions. Tests: `tools/emu/tests/sweep.py`.
The code facts are STATIC-CONFIRMED and checked in the emulator (EQUIVALENT). That input A is the
reference (the generator output looped back) and input B the device under test is DERIVED from
gain = B / A; nothing was measured on the device.

- **Block 0x42E180C8, more fields** (see batch 50 for the plot geometry):

  | Offset | Content |
  |---|---|
  | +0x47 | sweep running (cleared when the touch byte +0x15 stops it) |
  | +0x4C / +0x50 | generator amplitude / offset words for the sweep (copied to 0x42E3F984+0x2F00 / +0x2F04) |
  | +0x54 / +0x58 / +0x5C | start / end frequency in Hz, number of steps n (n + 1 points) |
  | +0x60 | linear step (end − start) / n |
  | +0x68[i] | gain ratio B / A of point i (doubles) |
  | +0x3EE8[i] | gain in dB, 20 log10 of the ratio (doubles) |
  | +0x7D68[i] | phase in degrees (s16) |
  | +0x8D08 / +0x8D10 / +0x8D18 | log10 of start, end, span |
  | +0x8D20[i] / +0x9CC0[i] | pixel rows of the gain / phase curve (u16) |
  | +0xACF8 / +0xAD00 | amplitude of input A / B at the current point (doubles); B is divided by the calibration ratio |
  | +0xAD08 / +0xAD10 | calibration ratio and phase offset used for the current point |
  | +0xAD18 | set during calibration: no event service between points |
  | +0xAD19 / +0xAD1A | input A / B reached an amplitude of 5000 at least once in the sweep |

- **`bode_sweep_run`** (0x4003A300): for i = 0..n measures at f = 10^(log10 start + i × span / n)
  (log spacing), shows "NN%" at (903, 510), and stores ratio, dB and phase. If one of the inputs
  never reached 5000, all points are reset to ratio 1, 0 dB, phase 0. At 100% the text is not
  rewritten (the previous "NN%" stays), stock behaviour.
- **`bode_point_measure(f)`** (0x4003A044):
  - sets the generator frequency, picks the input coupling (bytes SYS+0x46 / +0x10A = 2 below
    5 MHz, else 1; reading 2 as AC is OPEN) and the time base SYS+0x588 so that the record covers
    a few periods;
  - looks up the calibration entry in record D (0x42E554B0) for f;
  - then repeats acquisitions, stepping the range bytes SYS+0x41 (A) / +0x105 (B) while the
    amplitude words SYS+0xB30 / +0xB4C are above 0.8 or below 0.3 of the full scale (u16 SYS+0xA).
    When a range fits, or cannot move any more, the 64-bit amplitude (SYS+0x910 / +0x970) is
    taken.
- **`bode_phase_measure(i)`** (0x40039C08): rising crossings of the trigger level (u16 SYS+0xE) in
  the two capture buffers (0x42AC7CF4 and +0x62250), at least 8 samples apart. The phase is
  360 × (mean A→B delay) / (mean period of both inputs) minus the calibration offset, wrapped into
  −180..180 degrees. The outer 50 steps at both ends of the record are skipped.
- **`bode_calibrate`** (0x40039EB8): measures every frequency of record D's list (filled by
  `trace_defaults_37b48`: 1 to 50 MHz in 200 kHz steps) with the calibration neutral. The
  measured ratio and phase become the new calibration, saved with `settings_d_save` only when both
  inputs had signal.
- **`bode_autoscale`** (0x4003963C) fits gain and phase into 0.8 of the plot height. The gain
  scale is at most 25 pixels per dB (when it would reach 26) and the phase scale at most 3
  pixels per degree, with a spread of at least 5 degrees. Rows are kept in 3..height, and the
  axis ranges at +0x10..+0x28 are set to match.
- **`bode_curves_draw`** (0x40039254) draws both curves as vertical segments between neighbouring
  points, a legend box ("dB", "deg") and, when an input had no signal, a centred message: UI text
  129 (no signal on either input), 130 (only B had signal) or 131 (only A had signal). Reading the
  three texts as "no signal" messages is DERIVED; their content was not checked.
- **For new apps:** a frequency-response app can call `bode_sweep_run`, `bode_autoscale`,
  `bode_grid_draw`, `bode_curves_draw` and `bode_cursors_draw` in this order with its own start,
  end and step values in the block. `bode_point_measure` is a general "set the generator to f and
  read both amplitudes with auto-range" routine (DERIVED: no app was written yet).

## Defaults, FPGA parameter sync, generator calibration, self-test (batch 52)

Spec: `tools/platform_spec/zu_defaults.py`, 5 functions. Tests: `tools/emu/tests/sweep.py`.
The code facts are STATIC-CONFIRMED and checked in the emulator (EQUIVALENT).

- **`acq_defaults_reset`** (0x4003EB40, 17 callers) puts the acquisition state back to its
  defaults and then calls `runtime_state_commit_force`:
  - time-base index SYS+0x588 = 0x17, +0x598 = 0x16, the 64-bit words +0x570 / +0x578 = 0;
  - range bytes +0x41 / +0x105 = 6, coupling bytes +0x46 / +0x10A = 0;
  - the trigger level copies +0x27E, +0x60, +0x120 = u16 +0xE;
  - cursor words +0x808 = width / 5, +0x80C = 4 × width / 5, +0x810 = height / 4,
    +0x814 = 3 × height / 4 (width / height = u16 SYS+8 / +0xA);
  - the saved-trace state at SYS+0x3DA52C (batch 48) to 0, width / 3 and 2 × width / 3;
  - touch byte +0x16 = 1.
- **`system_defaults_init`** (0x4003E738) is the power-on defaults list: 135 constant stores
  and no reads. The C was generated from an emulator trace of the stock code (the store
  addresses and values, in order) and is checked like every other function. Values worth
  knowing:
  - screen 950 × 503 (u16 SYS+8 / +0xA), trigger level 251 (u16 +0xE), sample shift 6
    (word +0x1BC), marker word SYS+0x77C = 0xA55A5AA5;
  - generator limits in 0x42E3F984 + 0x2F10..+0x2F5C (for example 50,000,000 at +0x2F14);
  - the Bode plot block 0x42E180C8: x 70, y 48, 867 × 480, decade 289 pixels, 3 decades,
    rows of 80 pixels, 6 rows.
- **`fpga_params_6e4_sync`** (0x4003F160) sends the parameters SYS+0x6E4..+0x6E8 that differ from
  their cached copies at SYS+0xBB0..+0xBB4, updating each copy first:

  | Parameter | Path to the FPGA |
  |---|---|
  | bytes +0x6E4 / +0x6E5 / +0x6E6 | `fpga_param_updater(0x21 / 0x22 / 0x23, 1, word, byte)`, `smec_commit(0xC1 / 0xC2 / 0xC3)`, selector 4 / 5 / 6 + the byte |
  | byte +0x6E7 | selector 2 + the byte |
  | word +0x6E8 | selector 0xD + four bytes, high first |

  After any change it pulses selector 7 (1, then 0) and waits 2 ms. The word passed to
  `fpga_param_updater` is an unaligned 32-bit read at the copy (stock behaviour). What the five
  parameters control is OPEN.
- **`gen_output_calibrate`** (0x4004EF10) calibrates the generator output:
  - it measures the mean input (s64 SYS+0x8F8 / 1000) for three DAC settings (G+0x2F38 / +0x2F3C
    = 0x206C / 0x22C4, 0x206C / 0x1068, 1000 / 0xE42; G = 0x42E3F984), giving a, b, c;
  - k0 = (4700 (b − c) + 550 (b − a)) / 34,310,000, k1 = (a − b) / 4700,
    k2 = a − 8300 k0 − 8900 k1;
  - the three floats go to G+0x2F64..+0x2F6C (record G) and to the backup copy, then the record
    is saved and the state restored.

  The constants look like resistor values of the output stage, but that is OPEN.
- **`gen_selftest_50898`** (0x40050898) joins the self-tests of batches 40 and 41. It applies
  five generator / capture settings (25 Hz .. 10 kHz) and checks per channel that the
  peak-to-peak of the capture window is at most 10 × word SYS+0x1C0. The flags go to bytes
  0x42E68AE0 + 0x35 (A), + 0x36 (B) and + 0x34 (both).
- **Harness note.** Straight-line functions that only write constants can now be turned into C
  from an emulator trace (`build/trace52.py`, not committed: it hooks the memory writes). The
  result is still verified by the normal EQUIVALENT test, with random memory before the call.

## Trace horizontal position, zoom and gather (batch 53)

Spec: `tools/platform_spec/zv_trace_pos.py`, 4 functions. Tests: `tools/emu/tests/sweep.py`.
These are four of the trace-chain callees listed in batch 22. The code facts are
STATIC-CONFIRMED and checked in the emulator (EQUIVALENT). The meaning of each field
(for example "horizontal position") is DERIVED from how the code uses it.

- **Two parameter sets.** `mode_not_0_or_1(tr)` selects the set. The first set is used when it
  returns 0, the second one otherwise:

  | Meaning (DERIVED) | First set | Second set |
  |---|---|---|
  | position reference, s64 | SYS+0x570 | SYS+0x578 |
  | divisor, s64 | SYS+0x590 | SYS+0x5A0 |
  | samples per column (z) | SYS+0x5F8 | SYS+0x610 |
  | samples limit for the zoom | SYS+0x5FC | SYS+0x614 |
  | scale | SYS+0x600 | SYS+0x618 |
  | step | SYS+0x604 | SYS+0x61C |
  | view offsets (left, right) | SYS+0x608 / +0x60C | SYS+0x620 / +0x624 |

  The current position is the s64 at SYS+0x5E0. The segment count is u16 SYS+0x5F4. Segment
  k uses the start SYS+0x2C4[k], the base SYS+0x38C[k] and the length SYS+0x454[k]. The trace
  buffers are u16 arrays at 0x411AF3E8 + tr × 0x927C0, and the display buffers are at
  0x413F92E8 + tr × 0x927C0.
- **`trace_view_bounds`** (0x4003BB6C) gives the visible column range. For the other modes it
  calls `view_bounds_update` (0x40042280). With the first set:
  - lo = offset 0x608 + u16 SYS+0xC, clamped to 0..width (width = u16 SYS+8);
  - hi = offset 0x60C + u16 SYS+0xC − 1, clamped to 0..width − 1;
  - hi is raised to lo;
  - when the record SYS+0x5D4 is shorter than SYS+0x5F8, the range is also cut to the centred
    part, width × 0x5D4 / 0x5F8 wide.

  The results go to u16 SYS+0x10 (lo), +0x12 (hi) and +0x14 (hi − lo).
- **`trace_shift_samples`** (0x4003C158) handles a change of horizontal position:
  - q = (position − reference) × scale / divisor / step, in signed 64-bit;
  - the two view offsets go down by q;
  - every segment's samples shift by q × z, inside the segment. A positive shift copies
    forward, a negative one copies backward.
- **`trace_zoom_segments`** (0x4003C314) runs when the zoom factor SYS+0x554 is below the
  samples limit:
  - rem = ((reference − position) × z / divisor) mod step;
  - each view offset becomes rem + offset × limit / zoom;
  - each segment goes through `index_map_if_above` (batch 27) with its base, its u16 length,
    the limit and rem.

  It also calls `mode_not_0_or_2` and ignores the result. That is stock behaviour.
- **`trace_segments_gather`** (0x4003C5CC) fills the display buffer for columns start ..
  start + n:
  - each column and segment gets 2z + 1 values;
  - the source index is k × SYS+0x2B0 + z × column + j + (base − start − z × width / 2);
  - the index is clamped to 0..0x493E0, except when z equals SYS+0x5D4: then there is no offset
    and no clamp.

  The stock code keeps the per-segment offsets in a stack array of 101 entries.
- **Use for new features.** These four functions, together with the batch 22 chain, are the
  whole path from the capture buffer to the screen columns. A new horizontal mode (for example
  a different zoom) can reuse the parameter sets above.

## FFT, Hann window, trigger crossings and amplitude normalization (batch 54)

Spec: `tools/platform_spec/zw_fft_trigger.py`, 5 functions. Tests: `tools/emu/tests/sweep.py`.
The code facts are STATIC-CONFIRMED and checked in the emulator (EQUIVALENT). The names of the
quantities (trigger level, edge, window) are DERIVED from how the code uses them.

- **`fft_radix2`** (0x400665A8, also called from 0x40060DC8) is an in-place radix-2
  decimation-in-time FFT over n complex floats `{re, im}`:
  - first the bit-reversal swaps, then `dcache_clean_inv_all`;
  - then log2(n) stages. Each stage starts the twiddle at u = 1 and multiplies it by
    w = cos(pi / half) - i sin(pi / half) with `cmul` after each butterfly group;
  - between stages, when byte SYS+0x54E is set and byte 0x42E68668+5 is clear, it calls
    `runtime_event_service`. If that sets byte 0x42E68BB0+0x15, the FFT stops early and clears
    byte 0x42E68668+4. This keeps the touch screen responsive during long transforms.

  n must be a power of two, at least 2. For n < 2 or other sizes the stock loops never end, so
  the tests use only powers of two.
- **`fft_hann_spectrum`** (0x40041BA0, called by `zoom_views_update`) feeds the FFT view. It runs
  for each channel that is on (CH1: bytes SYS+0x3E and +0x42; CH2: +0x102 and +0x106), with
  n = SYS+0x7F8:

  | | input floats | complex buffer | magnitudes |
  |---|---|---|---|
  | CH1 | 0x42E49EB4 | 0x42E2DB08 | 0x42E4BEB4 |
  | CH2 | 0x42E4DEB4 | 0x42E31B08 | 0x42E4FEB4 |

  - x[i] = in[i] x (0.5 - 0.5 cos(2 pi i / (n - 1))), a Hann window, in double precision, stored
    as a float with a zero imaginary part;
  - then `fft_radix2(n, x)` and `cvec_magnitude(x, out, n / 2)`.

  The FFT is called even when n is 0, and then never returns.
- **`final_per_channel_normalization`** (0x4003FE14) evens out the amplitude of trace ch
  (SYS+0x7B4[ch]):
  - it runs only when the u64 SYS+0xB78 (CH2: +0xBA0) is at least
    (0x8D7F250 / SYS+0x524) x byte SYS+0x1B1;
  - the trace from index 10 to SYS+0x7BC - 10 is cut into half-periods at each crossing of the
    middle level m (SYS+0xB6C; CH2 +0xB94);
  - a half-period with mean >= m whose peak is more than (top - m) / 3 above m is stretched so
    that the peak lands on top (SYS+0xB60; CH2 +0xB88): v = m + (v - m)(top - m) / (peak - m);
  - a half-period below m is handled the same way with the bottom reference SYS+0xB64 (CH2
    +0xB8C);
  - the last sample of each half-period is left as it is (stock).
- **Trigger crossings.** The level is SYS+0x1C0 x (s16) SYS+0x27E, the trigger channel is
  byte SYS+0x27C, and byte SYS+0x27B selects the falling edge.
  - `moving_average_and_crossing_alignment_search` (0x40040114) runs when bytes SYS+0x274,
    +0x275 and +0x279 are all set:
    - with h = u16 SYS+0x282, s = SYS+0x558 x h x 3 and t = byte SYS+0x1B1 x SYS+0x62C x h, it
      copies n = 2s + 2t + 1 samples from index SYS+0x2A0 - t - s to 0x416431E8;
    - it smooths them with an s-point moving average;
    - from the middle it searches left and right for the first sample k with
      `d[k] < level <= d[k+1]` (falling: `d[k] > level >= d[k+1]`);
    - the distances are limited to SYS+0x298. u16 SYS+0x29C gets the nearer one (negative on the
      left; the right one on a tie), or 0 when there is none.
  - `trigger_window_crossing` (0x400403AC, two calls in `full_acquisition_entry`) works directly
    on trace ch, but only when ch is the trigger channel and byte SYS+0x275 is set:
    - centre c = s64 SYS+0x570 / s64 SYS+0x590 + SYS+0x7BC / 2;
    - window c +- 6 x SYS+0x55C / SYS+0x554;
    - a crossing at k compares k - 1 with k + 1. Rising: before < level <= after, or
      before = level < after. Falling is the mirror;
    - u16 SYS+0x29E = the offset of the nearer crossing from c (the right one on a tie). When
      only the right one is found, the left distance counts as the window edge. When none is
      found nothing is stored.
- **Not converted: the sample-rate table of 0x4003F2F4** (with its tail 0x4003F704). It is read
  from the disassembly (STATIC-CONFIRMED) but left as assembly:
  - key = ((u64 0x405453A0) x (SYS+0x524 / byte SYS+0x1B1)), times 100 / u16 SYS+0x1C unless byte
    SYS+0x588 is 11, then halved;
  - a 64-bit switch (helper 0x400657FC, 24 entries) sets the FPGA parameters of batch 52:
    byte +0x6E4 = 64 - a, byte +0x6E5 = 64 - b, bytes +0x6E6 / +0x6E7 and word +0x6E8:

  | key | +0x6E4 | +0x6E5 | +0x6E6 | +0x6E7 | +0x6E8 |
  |---:|---:|---:|---:|---:|---:|
  | 50,000 | 54 | 63 | 32 | 4 | 49999 |
  | 125,000 | 54 | 63 | 32 | 4 | 19999 |
  | 250,000 | 54 | 63 | 32 | 4 | 9999 |
  | 500,000 | 54 | 63 | 32 | 4 | 4999 |
  | 1,250,000 | 54 | 63 | 32 | 4 | 1999 |
  | 2,500,000 | 54 | 63 | 32 | 4 | 999 |
  | 5,000,000 | 54 | 63 | 32 | 4 | 499 |
  | 12,500,000 | 54 | 63 | 32 | 4 | 199 |
  | 25,000,000 | 54 | 63 | 32 | 4 | 99 |
  | 50,000,000 | 54 | 63 | 32 | 4 | 49 |
  | 125,000,000 | 54 | 63 | 32 | 4 | 19 |
  | 250,000,000 | 54 | 63 | 32 | 4 | 9 |
  | 500,000,000 | 54 | 63 | 32 | 4 | 4 |
  | 1,250,000,000 | 54 | 63 | 32 | 4 | 1 |
  | 2,500,000,000 | 54 | 63 | 32 | 4 | 0 |
  | 5,000,000,000 | 54 | 63 | 32 | 2 | 0 |
  | 12,500,000,000 | 56 | 63 | 32 | 1 | 0 |
  | 25,000,000,000 | 60 | 63 | 48 | 1 | 0 |
  | 40,000,000,000 | 54 | 60 | 56 | 1 | 0 |
  | 50,000,000,000 | 62 | 63 | 56 | 1 | 0 |
  | 125,000,000,000 | 60 | 59 | 62 | 1 | 0 |
  | 250,000,000,000 | 62 | 59 | 63 | 1 | 0 |
  | 400,000,000,000 | 63 | 60 | 63 | 1 | 0 |
  | 500,000,000,000 | 63 | 59 | 63 | 1 | 0 |

  For keys up to 2,500,000,000 the word +0x6E8 is 2,500,000,000 / key - 1, which looks like a
  decimation divider (DERIVED; what the bytes do is OPEN). For any other key the stock code
  falls into a second switch on a stack word it never wrote, and stores leftover register
  values. C cannot reproduce that, so the function stays assembly.

## Zoom resampling, cubic spline and the intensity view (batch 55)

Spec: `tools/platform_spec/zx_resample_persist.py`, 7 functions. Tests: `tools/emu/tests/sweep.py`.
The code facts are STATIC-CONFIRMED and checked in the emulator (EQUIVALENT). The method names
(nearest, spline, sinc) are DERIVED from the arithmetic.

- **`workspace_copy_and_resampling`** (0x4005C7BC, from `channel_post_process`) zooms one channel
  while the current factor k = SYS+0x554 is below the target n:
  - the len samples centred on half are copied to the workspace 0x41A13AE8. CH2 gets an extra
    tail of 15 / 7 / 4 / 2 / 1 samples for time-base index SYS+0x588 = 12..16;
  - the method comes from byte SYS+0x51C (CH1) / +0x51D (CH2):

    | Byte | Method | Function |
    |---|---|---|
    | 0 | nearest neighbour | `simple_integer_index_mapping_nearest_neighbour` (batch 49) |
    | 1 | cubic spline | `floating_point_interpolation_resampling` |
    | other | windowed sinc | `fixed_point_weighted_or_filtered_resampling` |

  - the result is copied back. For CH2 it is shifted by the tail.
- **Common to both resamplers.** They take (ws, half, len, m, s64 shift):
  - c = (len × k / m) × 12 / 10 input samples starting c/2 before half;
  - cnt = m × c / k outputs, written to 0x416431E8;
  - then len outputs, centred and moved back by shift, are copied to ws from half − len/2.
- **`floating_point_interpolation_resampling`** (0x4005D048):
  - the inputs become floats (v − 128 L) / (128 L) at 0x42DBCD54, with L = SYS+0x1C0;
  - knots x = j go to 0x42DA94D4, and the positions j × k / m go to 0x42DD05D4;
  - `spline_resample` writes 0x42DE3E54, and the outputs go back to u16 (y + 1) × 128 L,
    with negative values set to 0.
- **`spline_resample`** (0x4005CF78) builds the spline with 0x4005C95C, which is still assembly
  (flags 0, end slopes 0.0, coefficients b / c / d at 0x42D6EB54 / 0x42D823D4 / 0x42D95C54).
  It then evaluates `spline_eval` for each position.
- **`spline_eval`** (0x4005CEA4):
  - it keeps the last segment index and bisects only when x leaves that segment;
  - it returns a + dx (b + dx (c + dx d)).

  A NaN knot makes the bisection loop forever (stock behaviour).
- **`fixed_point_weighted_or_filtered_resampling`** (0x4005D2E4):
  - output o is the sum of x[j] × w(p − 10000 j), with p = o × (10000 / (m / k));
  - w(0) = 10000 and w(t) = `sine_table_lookup(t)` / t. The table at 0x42D564B4 is filled at run
    time, so this is a sinc kernel in fixed point (DERIVED);
  - the sum is divided by 10000, and negative results are set to 0.
- **Intensity (persistence) view.** The hit counts are u16, column-major, at
  0x400954A4 + tr × 0x12BE70. Pixels go to the frame buffer *(0x40095498), 1022 pixels per row,
  from (u16 SYS+4 + 1, u16 SYS+6).
  - Palette: 0x42E45864 (byte SYS+0x653 = 0) or 0x42E47B8C. The offset is 6000 bytes in the
    single-trace mode (byte SYS+0x652), otherwise 3000 × `mode_not_0_or_2(tr)`.
  - `trace_intensity_draw` (0x4005DDD4):
    - it draws a non-zero count v as colour 2 + sqrt(S) × S × 22 / 1000 + ((v / D) × 100 + 1) / 110,
      with S = byte SYS+0x654 and D = SYS+0x7B0;
    - in the per-trace mode with SYS+0x653 set it also divides SYS+0x7B0 by 3 and stores it. The
      intensity therefore decays every frame (DERIVED).
  - `trace_intensity_draw_window` (0x4005DF5C, with its piece 0x4005E0AC) draws the columns
    SYS+0x700 .. +u16 SYS+0x6F8 as colour 16 + (v / D) × 1000 / 1333, and then calls
    `fb_present_rect`. It does this only when SYS+0x6FC >= SYS+0x708.
- The spline coefficient builder 0x4005C95C is `spline_coeffs` (batch 56).

## Spline coefficients, intensity palettes and hits, peak hold, FFT spurs (batch 56)

Spec: `tools/platform_spec/zy_spline_spurs.py`, 7 functions. Tests: `tools/emu/tests/sweep.py`.
The code facts are STATIC-CONFIRMED and checked in the emulator (EQUIVALENT). The names (spline,
palette, spur, peak hold) are DERIVED from the arithmetic.

- **`spline_coeffs`** (0x4005C95C), the builder that `spline_resample` calls. Arguments
  (n, end1, end2, x, y, b, c, d, iflag, slope1 in s0, slope2 in s1):
  - *iflag = 1 when n <= 1 (unsigned compare) and *iflag = 2 when the x are not strictly
    increasing; nothing else is written then;
  - n = 2 gives the straight line (c = d = 0);
  - otherwise it is the classic tridiagonal cubic spline: h in d, the diagonal in b, the
    right-hand side in c. The ends match the third derivative (n > 3; zero for n = 3), or use
    the slope slope1 / slope2 when end1 / end2 = 1. `spline_resample` passes 0, 0, 0.0, 0.0;
  - on return y[i] + b dx + c dx^2 + d dx^3 is the curve on [x[i], x[i+1]], the form that
    `spline_eval` evaluates. The return register is not set.
- **Intensity view, the two missing pieces:**
  - `intensity_palettes_init` (0x4005DB9C) builds the RGB565 palettes used by
    `trace_intensity_draw`:
    - palette A 0x42E45864: three 1500-entry parts filled with yellow 0xFFE0, cyan 0x7FF and
      green 0x7E0, each starting with a 64-step ramp from black;
    - palette B 0x42E47B8C: a 192-step blue - cyan - green - yellow - red ramp over a red
      fill, copied to +3000 and +6000 bytes;
    - two u32 tables, 0x42E56C30 (the 271-entry levels colour table of `levels_shade_draw`,
      written to 1021 words, past its 1000-word fill) and 0x42E583A0 (460 words).
  - `trace_intensity_accumulate(tr)` (0x4005EDC0) adds a capture to the hit counts of trace
    tr. It runs only when u16 SYS+0x14 is set:
    - for u16 SYS+0x5F4 segments of W32 SYS+0x2B0 points, the points j = 10 .. points - 11
      are read as u16 pairs at 0x411AF3E8 / +0x927C0;
    - each point adds +5 to a 3 x 3 block of cells and +15 to its centre. The column is
      (H16 SYS+8 - H) / 2 + (a >> s), the row ((L x H - b) >> s) - 1, with H = u16 SYS+0xA,
      s = SYS+0x1BC, L = SYS+0x1C0;
    - then the largest count inside the plot area, divided by 200, becomes the divisor
      SYS+0x7B0 that `trace_intensity_draw` uses.

    With fewer than 10 points per segment the stock loop runs about 2^32 times.
- **`peak_hold_decimate(buf, n, step)`** (0x40061608, step in d0): in place, output k is the
  maximum (at least -28671) of the s16 samples from the previous end to (u32)(k x step).
- **`levels_max_panel_draw`** (0x40062A18) draws the "Max = " line of the level view at
  block 0x42E68668 +0xB0 / +0xB4: the frequency (s32 +8), the level +0x10 in dBmV and the
  voltage +0x18 x 1000. It is skipped while touch byte 0x42E68BB0+0x17 is set.
- **FFT spurs**, for the mode word 0x42E68668+0xF4 = 0..3. The lists are at 0x42E3C000:

  | Mode | Values (float) | Bin indices (u16) | Mean | Count (u16) | Room |
  |---|---|---|---|---|---|
  | 0 | +0x2340 | +0x37F4 | +0x14 | +6 | 200 |
  | 1 | +0x1D00 | +0x34D4 | +0x10 | +4 | 400 |
  | 2 | +0x12D8 | +0x2FC0 | +0xC | +2 | 650 |
  | 3 | +0x18 | +0x2660 | +8 | +0 | 1200 |

  The room column is the distance to the next list (DERIVED). The stock checks no bound, and it
  leaves its pointers unset for a mode above 3.
  - `fft_spur_capture` (0x40060BA4) works on the spectrum 0x42D364B4:
    - with half = n / 2 (n = +0xF8), it stores the mean of bins 2 .. half - 1;
    - every bin above 3 x the mean is recorded as its index and 2 x its value. The compare
      is on (s64)(x x 10000), through the firmware's `__aeabi_f2lz`.
  - `fft_spur_cancel` (0x40060DC8):
    - it applies a Hann window to the n floats at 0x42D164B4, runs `fft_radix2` and
      `cvec_magnitude` into 0x42D364B4, and sets byte +4;
    - unless byte +0x190 is set, it subtracts each recorded spur from its bin. A bin that
      falls below the mean is set to the mean.

    With n = 0, `fft_radix2` never returns (stock).
  - So the device can learn a reference spectrum and remove its fixed spurs from later
    captures (DERIVED; which screen offers it is OPEN).

## Intensity accumulators with smoothing, level-view layout and sample window (batch 57)

Spec: `tools/platform_spec/zz_intensity_layout.py`, 4 functions. Tests: `tools/emu/tests/sweep.py`.
The code facts are STATIC-CONFIRMED and checked in the emulator (EQUIVALENT). The names are
DERIVED from the arithmetic.

- **Intensity view, the other two accumulators** (with `trace_intensity_accumulate` of batch 56).
  Both stamp the samples of a trace into its hit counts and then smooth each column:
  - **Stamping.** Each column x takes (2 x points + 1) samples per segment, read at
    0x413F92E8 + tr x 0x927C0:
    - the row is (L x H - v) >> s, clamped to H - 3. It is H / 2 - 3 with SYS+0x2AA set (the
      split screen) in the first window;
    - the cells row - 1 / row / row + 1 get L - f / 0x40 / f, with f = (L x H - v) & (L - 1).
      This is a sub-pixel split of the sample between two rows (DERIVED);
    - the covered rows are flagged in 0x426BFF94 (0x4B0 bytes per column). Their lowest - 1 and
      highest + 1 are kept in 0x42E63D24 and 0x42E63528.
  - **Then each covered cell** gets 5 x segments + 20.
  - **Smoothing.** Passes s = shift .. 1 run a moving sum of 2^s cells over each covered run. It
    is written back 2^(s-1) cells behind as sum >> s.
  - **The window.** The first window is u16 SYS+0x10 / +0x12, with points W32 SYS+0x5F8 and
    gate u16 SYS+0x14. With `mode_not_0_or_1(tr)` it is SYS+0x16 / +0x18 / +0x610 / +0x1A.
  - `trace_intensity_accumulate_lines` (0x4005E12C):
    - it clears all column flags (0x95B50 u16), and calls `dcache_clean_inv_all` before the
      fill;
    - it uses up to 4 passes, fewer for short runs. With touch byte 0x42E68BB0+0x15 and byte
      0x42E61FF8 = 1, the pass count is 0x927C0 (SYS+0x1B0 = 2) or 0x124F80 divided by the
      summed run lengths, at most 4;
    - the divisor SYS+0x7B0 becomes max / 75.
  - `trace_intensity_accumulate_window` (0x4005E838) is the capture-window version:
    - it runs only when SYS+0x6FC >= SYS+0x708, on the columns SYS+0x700 .. + u16 SYS+0x6F8 - 1;
    - it always uses four passes, and sets SYS+0x7B0 = 45 x SYS+0x5F8 / 50.
  - With 0 segments and a non-empty window, the stock pass-count loop never ends (the run length
    is negative).
- **`levels_view_layout`** (0x400612FC) sets up the level (spectrum) view in block 0x42E68668:
  - the FFT size +0xF8 is 0x1000 / 0x2000 / 0x4000 / 0x8000 for mode +0xF4 = 0..3;
  - it sets the sample pointer 0x42E68CB0 = 0x42CC64B4 + 2 x size;
  - the plot box +0xB0 .. +0xD0 and the panel boxes +0xD4 .. +0xF0 come from the bytes +1 / +2
    (the two side panels, DERIVED);
  - the frequency axis comes from the s64 sample rate SYS+0x538 (DERIVED from the arithmetic):
    bin width +0x100 = rate / 1000 / size, top +0x108 = rate / 1000 / 2, centre +0x118 =
    (top + bin) / 2, span +0x110 = top - bin.
- **`levels_samples_window`** (0x40061A70) fits the visible frequency range to the plot width
  w = +0xB8:
  - the first and last index come from +0x120 / +0x128: i = ((f - bin) x size / 2) / span;
  - with more samples c than pixels, it calls `peak_hold_decimate(src, w, c / w)`;
  - otherwise it centres the samples (minus 0x8000) in the zoom workspace 0x416431E8 at index
    500000, with 100 copies of each end sample. It zooms them with the nearest-neighbour
    resampler (m = w x SYS+0x554 / c below 3 x SYS+0x554) or the spline resampler, then puts
    them back (plus 0x8000) through 0x4191F8A8;
  - the result is copied to the start of the sample buffer.

## Level-view frame and markers, numeric keypad (batch 58)

Spec: `tools/platform_spec/zza_levels_ui.py`, 3 functions. Tests: `tools/emu/tests/sweep.py`.
The code facts are STATIC-CONFIRMED and checked in the emulator (EQUIVALENT). The names and the
meaning of the fields are DERIVED from what is drawn.

- **`levels_graph_draw`** (0x40062448) draws the frame of the level (spectrum) view, from block
  0x42E68668. It does nothing while touch byte 0x42E68BB0+0x17 is set.
  - It clears the box at 0x42E6473A, draws two outlines (colour 0x444444) around the plot box
    +0xB0 / +0xB4 / +0xB8 / +0xC0, and makes the plot a touch button (record 0x42E62BD0).
  - The grid: +0xC8 - 1 vertical lines every +0xC4 pixels, and +0xD0 - 1 horizontal lines every
    +0xCC pixels.
  - The level labels at x = 5 (colour 0x888888): top +0x148 minus i x step +0x140, for
    i = 0 .. +0xD0. It adds 'Ref' under the first one unless both side panels are on.
  - The frequency labels under the plot: +0x120 + j x (+0x128 - +0x120) / +0xC8, for
    j = 0 .. +0xC8. They are centred on the grid lines, the first is left aligned and the last is
    right aligned. Each label is measured first (`frequency_draw` with measure 1).
  - For each side panel on (bytes +1 and +2): a colour bar of +0xE0 / +0xF0 rows at x = 10 .. 0x28,
    from the tables 0x42E56C30 (index 1000 .. 0) and 0x42E583A0 (index 512 .. 0), and the same
    frequency labels under the panel (+0xD4 / +0xE4 left, +0xD8 / +0xE8 top).
- **`levels_markers_draw`** (0x400606A0) runs while the markers flag +0x58 is set:
  - two dashed vertical lines (colour 0x88FFFFFF) at x offsets +0x5C / +0x60 in the plot, and a flag
    labelled C1 / C2 (colour 0xBBBBBB) on top of each one;
  - a readout panel at the bottom left of the plot with the rows 'C1 = ', 'C2 = ', 'DC = '. Each row
    has the frequency (s64 +0x68 / +0x70 / +0x78), the level in dBmV (+0x80 / +0x88 / +0x90) and
    the voltage (+0x98 / +0xA0 / +0xA8). The rows are 0x11 pixels apart.
- **`numpad_draw`** (0x4005FF88) draws the numeric keypad in the box at 0x42E646F4 (x, y, w, h as
  u16):
  - a dark frame with the title string 0x42E68A8B;
  - 4 rows of 4 keys, each 0x5A x 0x3C and 0x5D x 0x3F apart. The keys are
    `7 8 9 F / 4 5 6 F / 1 2 3 F / F 0 . F`, where F is a function key (colour 0x202020; the
    digits are 0x303030);
  - each key is a touch button (records 0x42E62A18 .. 0x42E62AAE). It is filled with the colour
    W32 0x409223B4 when pressed;
  - the mode byte 0x42E68A89 (1 .. 6) disables keys 11 / 12 / 15, picks the glyphs of keys 7, 11
    and 15, and shows '+-' on key 12 in modes 2 .. 4. Key 3 always has the same glyph (0x40087B94,
    likely backspace; OPEN).

## Time-base step table and divisors, Bode curve zoom (batch 59)

Spec: `tools/platform_spec/zzb_timebase_bode.py`, 3 functions. Tests: `tools/emu/tests/sweep.py`.
The code facts are STATIC-CONFIRMED and checked in the emulator (EQUIVALENT). The names and units
are DERIVED.

- **`timebase_step_table_init`** (0x4003A680) fills the 36 s64 entries at SYS+0xBD0 with the
  1-2-5 series 1, 2, 5, 10 .. 5e13. `timebase_period_load` reads them. Entry 11 (SYS+0xC28) is
  5,000,000 instead of 5,000 in this build: STATIC-CONFIRMED. Whether that time-base step is ever
  selected is OPEN.
- **`timebase_divisors_compute`** (0x4003B834), called by `acquisition_rearm`:
  - P = 1e15 / period (SYS+0x590) x u16 SYS+0x1C, and k = ceil(SYS+0x5A8 / P);
  - the sample count SYS+0x5FC = k x W32 0x405453BC x P / SYS+0x5A8 (signed);
  - SYS+0x5F8 = min(k, 10 x SYS+0x5D4);
  - SYS+0x600 = the smallest j for which SYS+0x5FC x j is a multiple of W32 0x405453BC, and
    SYS+0x604 = the quotient (1 when the count equals that word);
  - the repeat SYS+0x5F4 = u16 SYS+0x5D0, or 1 when SYS+0x3C or SYS+0x665 is set. SYS+0x544 =
    SYS+0x5F8 x the repeat (u16);
  - per channel: the scales SYS+0xE4 / +0x1A4 = (s64 +0x58 / +0x118 << 10) / (+0x50 / +0x110),
    the spans +0xE8 / +0x1A8 = +0x64 - +0x60 / +0x124 - +0x120, and the bytes SYS+0x650 / +0x651
    = ceil of the ratio, rounded up from tenths and at least 1;
  - with SYS+0x28 set and SYS+0x3C clear it clears SYS+0x570 .. +0x57F and the byte SYS+0x580;
    otherwise it sets SYS+0x580 = 1.
- **`bode_curves_zoom`** (0x40039918) fits the gain and phase curves of the Bode view (u16 rows
  0x42E180C8 + 0x8D20 / + 0x9CC0, W32 +0x5C points) to the plot width u16 +4:
  - each curve is centred at index 10000 of the workspace 0x416431E8, with 100 copies of each end
    sample;
  - it is resampled with m = +4 x SYS+0x554 / +0x5C (kept in +0x40): nearest neighbour below
    3 x SYS+0x554, else the spline. Then it is copied back;
  - every row is clamped to 3 .. +6 - 3 and flipped to +6 - row (screen rows grow downwards).

## Automatic trigger level, SMEC byte sync, settings record C (batch 60)

Spec: `tools/platform_spec/zzc_trigger_smec_rec.py`, 3 functions. Tests: `tools/emu/tests/sweep.py`.
The code facts are STATIC-CONFIRMED and checked in the emulator (EQUIVALENT). The names and the
purpose are DERIVED.

- **`trigger_level_auto`** (0x4006382C) picks the trigger level u16 SYS+0x27E:
  - the candidates are L = u16 SYS+0x632 levels, min + i x range / (L + 1) for i = 1 .. L. Min and
    range are the words +4 / +8 of SYS+0xB28, or of SYS+0xB44 when SYS+0x27C is set. L is used up
    to 10: the counters are 10 words on the stack, and a larger L would overwrite the frame;
  - the samples are u16 at W32 SYS+0x7B4 (or +0x7B8) + 0x14, with SYS+0x558 streams interleaved.
    Each stream has (u16 SYS+0x7BC - 0x14) / SYS+0x558 samples, each shifted right by SYS+0x1BC.
    With fewer than 0x14 bytes, the u16 count wraps and the scan runs over 65,000 samples;
  - each level counts its crossings, rising (prev < level <= cur) or falling
    (cur <= level < prev);
  - it takes the level with the fewest crossings, or the middle one (L / 2) when the middle count
    x 4 is below the fewest x 5.
- **`smec_byte_params_sync`** (0x40063CD0) handles the 8 bytes SYS+0x674 .. +0x67B. For each one
  that differs from its copy at SYS+0xBB8 + i:
  - it updates the copy;
  - it sends SMEC job 0x19 + i (`fpga_param_updater`, size 1; the unused word argument is an
    unaligned load for odd i), commits job 0xB9 + i, and writes the byte to FPGA selector 0xA6 + i.

  It always ends by writing 0 and then 1 to selector 0x11. That looks like a latch strobe
  (DERIVED). What the 8 bytes control is OPEN.
- **`record_c_fields`** (0x40043B0C) is the field list of settings record C, the FFT spur lists at
  0x42E3C000:
  - the serializer cursor is set to 0;
  - then the four u16 counts +6 / +4 / +2 / +0 and the four float means +0x14 / +0x10 / +0xC /
    +8;
  - then 200 / 400 / 650 / 1200 pairs of float value and u16 bin, from +0x2340 / +0x1D00 /
    +0x12D8 / +0x18 and +0x37F4 / +0x34D4 / +0x2FC0 / +0x2660 (the FFT spur modes 0 .. 3, see above).

## ADC lanes for the sample rate, UI layout table, settings field lists (batch 61)

Specs: `tools/platform_spec/zzd_adc_lanes_layout.py` and `zze_settings_fields.py`, 5 functions.
Tests: `tools/emu/tests/sweep.py`. The code facts are STATIC-CONFIRMED and checked in the emulator
(EQUIVALENT). The names and the meaning of the fields are DERIVED.

- **`adc_lanes_config_for_rate`** (0x40063F04) sets up the 8 ADC lanes for the sample rate
  U64 SYS+0x538 (in mSa/s, as in batch 57):
  - the lane masks SYS+0x674 .. +0x67B, the bytes that `smec_byte_params_sync` (batch 60) sends
    to FPGA selectors 0xA6 .. 0xAD. They are 4-of-8 bit patterns (0xF0, 0x3C, 0x0F, 0xC3, 0x78,
    0x1E, 0x87, 0xE1), which look like the clock phases of each lane (DERIVED);
  - the per-lane delays SYS+0x67C .. +0x683, copied from one 8-byte group of table A
    (SYS+0x684, the 64-byte record A). An unknown rate gets 0x40 in every lane;
  - the per-lane phases SYS+0x6D4 .. +0x6DB (cleared) and the lane → bank map SYS+0x6DC .. +0x6E3
    (see batch 17).

  | SYS+0x1B0 | Rate (MSa/s) | Table A group | Bank map |
  |---|---|---|---|
  | 1 | 1000 / 950 | 0 / 1 | 2 6 3 7 0 4 1 5 |
  | 1 | 475 | 2 | 2 6 3 7 0 4 5 1 |
  | 1 | 237.5 / other | 3 / none | 2 6 7 3 0 4 5 1 |
  | not 1 | 475 / 380 / 237.5 / 118.75 / other | 4 / 5 / 6 / 7 / all 0x40 | 2 3 0 1 6 7 4 5 |

- **`ui_layout_init`** (0x4003CF88) fills the UI layout table at 0x42E64520 with 222 stores:
  - mostly fixed u16 boxes (x, y, w, h) and flag bytes; the keypad box +0x1D4 and the level-view
    box +0x21A are among them;
  - copies of u16 SYS+4 .. +0xA, of the Bode box 0x42E180C8 .. +6, and of the level plot box
    0x42E68668 + 0xB0 / +0xB4 / +0xB8 / +0xC0.

  The C keeps the stock store order. It was produced from two stock runs with different memory
  (`tools/gen_store_list.py`): equal values are constants, the others are copies of the address read.
- **Settings field lists**, the records of the batch 15 serializer. The call sequences were taken
  from stock runs (`tools/gen_call_list.py`) and folded into loops:
  - `slot_fields` (0x40043F54), a user slot: 121 fields. They are the acquisition and display
    state of SYS, generator words at 0x42E3F984 + 0x2EFA, the level-view settings
    (0x42E68668 + 1 .. 3, + 0x58 .., + 0xF4, the eight s64 at + 0x120, + 0x188 / + 0x18C), the
    Bode sweep settings (0x42E180C8 + 0x44 / + 0x46 / + 0x4C .. / + 0x60) and 0x42E22D28 .. + 8;
  - `settings_image_fields` (0x400448B4), the main settings image, from cursor 0x32. It holds the
    SYS fields from + 4 up, u16 SYS+0x5D0 triples of words + 0x2C4 / + 0x38C / + 0x454, the block
    from + 0x800, and B8 SYS+0x8E4 u16 values from + 0x880. Both counts are re-read on every pass,
    so a decode that loads a count uses it at once;
  - `calibration_process_serialized_fields` (0x40044450, Hack name): 50 rows of the 32 word
    columns of the table 0x42E53A0C (+ 0x190 + 0xC8 x column + 4 x row), then 20 rows of four
    word columns (+ 0x50 / + 0xF0 / + 0xA0 / + 0x140). SIM.md already notes the 32 columns.

## Split-window marker, three-row menu, screenshot viewer frame (batch 62)

Spec: `tools/platform_spec/zzf_ui_menus.py`, 3 functions. Tests: `tools/emu/tests/sweep.py`.
The code facts are STATIC-CONFIRMED and checked in the emulator (EQUIVALENT). The names and the
meaning of the boxes are DERIVED.

- **`split_window_marker_draw`** (0x40042520) marks the zoom window on the plot. It does nothing
  when touch byte 0x42E68BB0+0x17 is set or SYS+0x2AA is 0.
  - The window has width u16 SYS+0x24 and centre SYS+0x26. The plot box is SYS+4 / +6 / +8 / +0xA
    (x, y, w, h).
  - The left edge is clamped to [1, w-2] and the right edge to [0, w-1].
  - The plot outside the window is dimmed with colour 0xCCFFFFFF, over a third of the plot height.
  - Each edge is drawn as three vertical lines, 0xAAAAAA / 0x555555 / 0x222222 from the outside in.
  - A stock quirk: the left edge is clamped against w-2, not against x+w-2.
- **`menu_three_rows_draw`** (0x4001E8E0) draws a menu box with three rows of 0x3C. The box is in
  the layout table at 0x42E64520 + 0xAE .. + 0xB4.
  - The background uses theme colour SYS+0x3DD550, the text +4 and a pressed row +8.
  - Each row has an image (0x4008A038 + 0x170 x row), the label of text-table entry 110 + row
    with its per-language x / y offsets (bytes + 0x194 / + 0x195 of the 0x1A0-byte entry), and the
    icon 0x40019C7C at x + 0x14A.
  - Each row is also a touch button: records 0x42E61FF8 + 0x340 / + 0x34A / + 0x354.
- **`screenshot_view_frame_draw`** (0x40025DB4) draws the frame of the window at layout + 0x148.
  The body (minus 0x5A) is touch button + 0x80E.
  - The title bar is 0x1E high, 0x202020, with a 7-line gradient under it (colours k << 29). It
    holds image 0x4008A038 and text entry 110, the same as row 0 of the menu above.
  - At x + 0x3ED, right-aligned, it shows the file name that `screenshot_path_make` builds from the
    current entry of the list at SYS+0xFA4 (the text at SYS+0x88C1). This is why we read the
    window as the screenshot viewer (DERIVED).
  - The bottom bar is at y + 0x21C, 0x3C high, with a gradient above it. It has three separators
    and four touch buttons, + 0x818 / + 0x822 / + 0x82C / + 0x836, which are shaded with theme
    colour SYS+0x3DD54C while pressed.
  - The buttons are labelled with text entries 67, 115, 116 and 117, each centred in 0xFD.
  - A stock quirk: the separators, buttons and labels use 0x21C - y, while the bar itself uses
    y + 0x21C. The two agree only when y is 0.

## Configuration menus, zoom ratio and zoom window (batch 63)

Spec: `tools/platform_spec/zzg_config_menus.py`, 3 functions. Tests: `tools/emu/tests/sweep.py`.
The code facts are STATIC-CONFIRMED and checked in the emulator (EQUIVALENT). The names are
DERIVED. The menu meanings come from the English labels of the text table (`build/uitext46.txt`).

- **`config_menu_draw`** (0x40028740) draws the configuration menu (layout + 0x68 .. + 0x6E). It
  has three rows: 'Save configuration', 'Read configuration' and 'Power-on configuration' (text
  entries 83 .. 85). The images are at 0x4008B2E8 + 0x170 x row, and the touch buttons are
  0x42E61FF8 + 0x106 / + 0x110 / + 0x11A.
  - A pressed row becomes the selection, stored in layout byte + 0x67 as row + 1.
  - The selected row is shaded with theme colour + 8.
- **`config_slot_menu_draw`** (0x40028AB4) draws the list beside that menu (layout + 0x76): five
  rows 'Configuration 1' .. '5' (entries 86 .. 90) with buttons + 0x124 + 0xA x row.
  - A row is shaded while pressed.
  - It is also shaded when the selection is 3 ('Power-on configuration') and the row is the
    power-on slot SYS+0x7C0. This matches the slot loaded at start-up (batch 15).
- The three-row menu of batch 62 (`menu_three_rows_draw`) is the file browser menu: 'Picture
  browser', 'Waveform browser' and 'Capture browser' (entries 110 .. 112). The window of
  `screenshot_view_frame_draw` has the 'Picture browser' title, and its buttons are 'Return',
  'Delete', 'Last page' and 'Next page'.
- **`zoom_ratio_window_update`** (0x4004201C) is called from `acquisition_rearm`. It does nothing
  when `zoom_mode_check()` is set; otherwise it first runs `zoom_entry_select`. In the steps below
  C = u64 SYS+0x5A0, A = s64 SYS+0x5A8, D = SYS+0x554 and step = s64 SYS+0x590.
  - P = (10^15 / C) x u16 SYS+0x1C, and n = A / P rounded up (signed division).
  - SYS+0x610 = min(n, 10 x SYS+0x5D4), and u16 SYS+0x54A = that x u16 SYS+0x5F4.
  - q = n x D x P / A goes to SYS+0x614. The ratio q : D is reduced: k is the smallest value with
    q x k divisible by D, SYS+0x618 = k and SYS+0x61C = q x k / D (both are 1 when q = D).
  - The view position s64 SYS+0x578 is clamped so that its distance from SYS+0x570 stays within
    (plot width / 2) x (C - step) on one side and (plot width / 2) x (step - C) on the other. The
    plot width is u16 SYS+8.
  - Then it sets the zoom window that `split_window_marker_draw` (batch 62) draws:
    - centre u16 SYS+0x26 = (SYS+0x570 - SYS+0x578) / step + width / 2;
    - width u16 SYS+0x24 = C x width / step, at least 6.
  - The symbol table splits this function at 0x40042024, 0x400421A0 and 0x40042200. The spec
    entry covers all the pieces.
- **Still assembly:** 0x4003F2F4, the sample-rate table (see the note just before the batch 55 section).
  Its default case reads a stack word that is never written and the caller's registers, so it
  stays as it is.

## Bode and level-view toolbars and settings bars (batch 64)

Spec: `tools/platform_spec/zzh_toolbars.py`, 4 functions. Tests: `tools/emu/tests/sweep.py`.
The code facts are STATIC-CONFIRMED and checked in the emulator (EQUIVALENT). The names are
DERIVED. The labels are the English entries of the text table (`build/uitext46.txt`).

All four bars work the same way:
- they start with a black box from the layout table;
- then comes a row of 0x2C-high touch buttons, the first at x + 3;
- the toolbar theme is SYS+0x3DD530: + 0 is the fill, + 4 the icon, + 8 the text and + 0x1C a
  pressed button;
- the touch records are at 0x42E61FF8 + offset, and the byte after each record is "pressed".

The two toolbars have buttons 0xA6 wide, every 0xAA. Each has an icon at (+ 0x49, + 2) and a
centred label. The stock code sets a per-button colour and replaces it at once with the theme
colour; the C keeps both calls.

- **`bode_toolbar_draw`** (0x4002A184) draws the frequency-response view toolbar (layout
  + 0x1E2, records + 0xAC0 ..):
  - 'Return', 'Calibration', 'Screenshot', 'Cursor';
  - 'Single' or 'Loop', chosen by byte 0x42E180C8 + 0x44;
  - a run indicator (icon 0x400857FC), red while + 0x45 is set and green otherwise.
- **`levels_toolbar_draw`** (0x4002AE60) draws the level-view toolbar (layout + 0x21A, records
  + 0xB56 ..):
  - 'Return', 'Function', 'Screenshot', 'Cursor';
  - an unlabelled icon 0x40085AF4 in 0x0088FF;
  - the run indicator, which follows byte 0x42E68668 + 0. Its background is 0xA7 wide, but its
    touch area is 0xA6.

The two settings bars show each button's grey label at (+ 0xF, + 5) and its value in 0xCCCCCC at
(+ 0xF, + 0x19).

- **`bode_sweep_bar_draw`** (0x4002A7F4) draws the Bode sweep settings (layout + 0x1F0, six
  buttons of 0xA6, records + 0xB06 ..). It calls `trace_cursor_limits()` first.

  | Button | Value | Drawn by |
  |---|---|---|
  | 'Amplitude' | 1000 x word + 0x4C | `voltage_uv_draw` |
  | 'Offset' | 1000 x word + 0x50 | `voltage_uv_draw` |
  | 'Start frequency' | word + 0x54 | `frequency_draw` |
  | 'Stop frequency' | word + 0x58 | `frequency_draw` |
  | 'Frequency count' | word + 0x5C, as an integer | 0x4002D6F8 |
  | 'Movement' | 'Fast move' / 'Slow move' by byte + 0x46 | text |

- **`levels_param_bar_draw`** (0x4002B43C) draws the level-view settings (layout + 0x228, seven
  buttons of 0x8E every 0x92, records + 0xB92 ..). The 'Movement' button is 0x8D wide.

  | Button | Value | Drawn by |
  |---|---|---|
  | 'Reference' | double + 0x148 | `level_label_dbmv` |
  | 'Attenuation' | double + 0x140 | `level_label_dbmv` |
  | 'Start frequency' | s64 + 0x120 | `frequency_draw` |
  | 'Stop frequency' | s64 + 0x128 | `frequency_draw` |
  | 'Center freq' | s64 + 0x130 | `frequency_draw` |
  | 'Freq width' | s64 + 0x138 | `frequency_draw` |
  | 'Movement' | 'Fast move' / 'Slow move' by byte + 3 | text |

  The values are fields of block 0x42E68668.

These tables give the meaning of the Bode and level-view settings words that `slot_fields`
(batch 61) saves.

## FFT settings menu, plot grid and right-hand menu column (batch 65)

Spec: `tools/platform_spec/zzi_fft_grid.py`, 3 functions. Tests: `tools/emu/tests/sweep.py`.
The code facts are STATIC-CONFIRMED and checked in the emulator (EQUIVALENT). The names are
DERIVED. The menu meanings come from the English labels of the text table (`build/uitext46.txt`).

- **`fft_settings_menu_draw`** (0x4002BC40) draws the FFT settings menu (layout + 0x244 .. + 0x24A).
  It has four rows.
  - 'FFT length' (text entry 137) has four option buttons with the texts '4K', '8K', '16K' and
    '32K'. The word 0x42E6875C is the selected index (0 .. 3). The selected option is filled with
    theme colour + 0x10 and has black text. The touch records are 0x42E61FF8 + 0xBE2 + 0xA x i.
  - 'Waterfall_display' (entry 138) and '3D display' (entry 139) each have an ON/OFF switch. The
    switch states are the bytes 0x42E68669 and 0x42E6866A. The records are + 0xC0A and + 0xC14.
  - 'Calibration' (entry 68) has the arrow icon and record + 0xC1E.
- **`plot_grid_draw`** (0x4002C7AC) draws the grid of the main plot. The plot box is x0 = u16
  SYS+4, y0 = SYS+6, width SYS+8 and height SYS+0xA.
  - When the grid byte SYS+0x655 is set, it draws in colour 0x303030:
    - vertical lines every u16 SYS+0x1C;
    - horizontal lines every u16 SYS+0x20;
    - 40 ticks, 5 pixels wide, across the vertical centre line, every 12.6 pixels;
    - 50 ticks from centre - 1 to centre + 3 on the horizontal centre line, every 19 pixels.
  - It always draws the frame in colour 0x505050.
  - When the split-screen byte SYS+0x2AA is set, the plot is cut into two parts.
    - The upper part is h/3 + 1 high, with a line step of SYS+0x20 / 3 and ticks every 4.2.
    - The lower part starts at y0 + h/3 + 1 and is 2h/3 + 1 high, with a line step of
      2 x SYS+0x20 / 3 and ticks every 8.4.
    - The tick steps are the floats 0x40866667 and 0x41066667. These are one bit above the nearest
      floats to 4.2 and 8.4, so the C writes them as hex floats.
    - The upper frame has no bottom line.
- **`side_menu_column_draw`** (0x4001A708) draws the right-hand menu column (layout + 0x22 /
  + 0x24, 0x40 wide). It has eight buttons at y + 0, 0x4B, 0x96, 0xE1, 0x127, 0x175, 0x1BF and
  0x20A.
  - Each button has an icon image and the touch record 0x42E61FF8 + 0x5C + 0xA x i.
  - A record byte + 1 equal to 1 shades the button with theme SYS+0x3DD54C.
  - The first icon is red when SYS+0x3C is set and green when not.
  - The fifth icon depends on SYS+0x664: image 0x400869CC in blue when set, 0x400866D4 in orange
    when not.
  - Which view each button opens is OPEN. The column only draws; the touch handling is elsewhere.

## Display menu, high-resolution menu and main toolbar (batch 66)

Spec: `tools/platform_spec/zzj_display_menus.py`, 3 functions. Tests: `tools/emu/tests/sweep.py`.
The code facts are STATIC-CONFIRMED and checked in the emulator (EQUIVALENT). The names are
DERIVED. The menu meanings come from the English labels of the text table (`build/uitext46.txt`).

- **`main_toolbar_draw`** (0x40019DB8) draws the bottom toolbar of the oscilloscope view (layout
  + 0x14 / + 0x16).
  - It has nine buttons, 0x66 x 0x2C each and 0x6A apart, with the labels 'System', 'Function',
    'Measure', 'Browser', 'Cursor X', 'Cursor Y', 'Save wave', 'Screenshot' and 'Zoom' (text
    entries 0 .. 8).
  - The icons are 0x40088208 + 0x170 x i.
  - The touch records are 0x42E61FF8 + 2 + 0xA x i. A pressed button is filled with toolbar
    theme + 0x1C.
- **`display_menu_draw`** (0x4001CE30) draws the display settings menu (layout + 0x92 .. + 0x98).
  It has seven rows, labelled with text entries 16 .. 22.
  - 'Waveform light' shows the value byte SYS+0x654 with a '%' and a slider.
  - 'Color temperature', 'X-Y mode display' and 'Background gird' are switches for the bytes
    SYS+0x653, SYS+0x652 and SYS+0x655. SYS+0x655 is the grid flag that `plot_grid_draw` (batch
    65) reads, so this is now STATIC-CONFIRMED.
  - 'Capture output', 'Frequency response analyzer' and 'Spectrum analyzer' (the last one with
    ' (CH1)') have an arrow icon, so they open other views.
  - The touch records are 0x42E61FF8 + 0x20A + 0xA x row.
- **`hires_menu_draw`** (0x40021630) draws the 'High resolution mode' menu (layout + 0xE6 ..
  + 0xEC).
  - It has nine options for the byte SYS+0x660: 'OFF' / '8 bit' (option 0, wide), then '9 bit' ..
    '16 bit' (strings 0x400903D8 + 8 x i) in two rows of four.
  - The selected option is filled with theme colour + 0x10.
  - The touch records are 0x42E61FF8 + 0x55C + 0xA x i.
  - What the extra bits do in the acquisition path (averaging or oversampling) is OPEN.

## CH1 and CH2 settings menus (batch 67)

Spec: `tools/platform_spec/zzk_channel_menus.py`, 2 functions. Tests: `tools/emu/tests/sweep.py`.
The code facts are STATIC-CONFIRMED and checked in the emulator (EQUIVALENT). The names are
DERIVED. The menu meanings come from the English labels of the text table (`build/uitext46.txt`).

`ch1_menu_draw` (0x4001EBE4) and `ch2_menu_draw` (0x4001F5A8) are the same code with other
offsets. In the C they share one static function, `channel_menu`.

| | CH1 | CH2 |
|---|---|---|
| layout box | + 0xBC | + 0xCA |
| settings in SYS | + 0 | + 0xC4 |
| touch records | 0x42E61FF8 + 0x35E .. | + 0x64 |
| enable label | entry 25 | entry 26 |

The CH1 settings bytes, with their rows and options:

| Row | Byte | Control |
|---|---|---|
| 'Channel enable' | SYS+0x3E | ON/OFF switch |
| 'Proble attenuation' | SYS+0x40 | options 0 '1X', 1 '10X', 2 '100X' |
| 'Coupling mode' | SYS+0x3F | options 0 'DC', 1 'AC' |
| 'FFT display' | SYS+0x42 | ON/OFF switch |
| 'Bandwidth limit' | SYS+0x47 | options 0 'Full 350M', 1 '150M', 2 '20M' |

For CH2 the bytes are the same + 0xC4: SYS+0x102, 0x104, 0x103, 0x106 and 0x10B.

- The selected option is filled with theme colour + 0x10 and has black text.
- A pressed row or option is shaded with theme + 8.
- The two texts '1X' and '10X' are inside the stock function. The C uses string literals, so the
  test stubs record these texts by content.

## Time-base menu and measurement selection menu (batch 68)

Spec: `tools/platform_spec/zzl_timebase_menu.py`, 2 functions. Tests: `tools/emu/tests/sweep.py`.
The code facts are STATIC-CONFIRMED and checked in the emulator (EQUIVALENT). The names are
DERIVED. The menu meanings come from the English labels of the text table (`build/uitext46.txt`).

- **`timebase_menu_draw`** (0x4001FF74) draws the time-base menu (layout + 0xD8 .. + 0xDE).
  - The title is 'Main time base' (text entry 34). When the split-screen byte SYS+0x2AA is set,
    it is 'Zoom time base' (entry 35) instead.
  - The current setting is the byte SYS+0x588 (main) or SYS+0x598 (zoom).
  - There are 31 settings: 0x2A = 50 s, 0x29 = 20 s, 0x28 = 10 s, and so on in 1-2-5 steps down to
    0x0C = 5 ns. The time-base index is therefore 0x0C (5 ns) .. 0x2A (50 s).
  - 50 s is a wide button in the title row. The other 30 settings are 0x50 x 0x28 buttons in five
    rows of six.
  - The touch records are 0x42E61FF8 + 0x426 (50 s) and + 0x430 + 0xA x (0x29 - setting).
- **`measure_select_menu_draw`** (0x4001D608) draws the measurement selection menu (layout + 0xA0
  .. + 0xA6), with one block of twelve buttons for each channel.
  - The blocks are 'CH1 parameter measurement' (entry 23) and 'CH2 parameter measurement' (entry
    24). The buttons are 'Vmax', 'Vmin', 'Vavg', 'Vrms', 'Vp', 'Vpp', 'F', 'T', 'T+', 'T-', 'Du+'
    and 'Du-'.
  - Measurement k (1 .. 12) of channel c is shown as selected when `value_in_table_405456e8(c, k)`
    is set. That is, k (CH1) or -k (CH2) is in the list of 24 shown measurements at 0x405456E8.
    The measurement codes are therefore +-1 .. +-12 in this order.
  - The touch records are 0x42E61FF8 + 0x250 + 0xA x (12 c + k - 1).
  - Stock quirk: the last two buttons are centred by the width of 'Dt+' / 'Dt-' but draw 'Du+' /
    'Du-'. 'Du' is probably the duty cycle (DERIVED). The C keeps both texts.
  - The CH2 block is drawn without the `half_42e68be2_set(2)` that comes before the CH1 buttons.

## Not yet rewritten (next batches)

- Remaining clock users: `sys_mmu_init` and 0x400429E4 (CP15, not emulated), and 0x4003E00C..0x4003E314 (with TCON).
- (USB stack: done, see USB.md.)
- The display engine and TCON code (DE2 0x01000000, functions of up to 5 KB).
- The next region (0x4004B0A0 onwards: POST and factory flow).
