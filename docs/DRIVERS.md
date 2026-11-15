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

## Not yet rewritten (next batches)

- Remaining clock users: `sys_mmu_init` and 0x400429E4 (CP15, not emulated), and 0x4003E00C..0x4003E314 (with TCON).
- (USB stack: done, see USB.md.)
- The display engine and TCON code (DE2 0x01000000, functions of up to 5 KB).
- The next region (0x4004B0A0 onwards: POST and factory flow).
