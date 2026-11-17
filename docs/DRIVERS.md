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
  | C | list 0x40043B0C (not rewritten) | SYS+0x7F0 | 0x4000 |
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
  callees that are still assembly (0x4005E10C, 0x4003C0AC, 0x4003C158, 0x4003C314, 0x4003BB6C,
  0x4003C5CC, 0x4005E12C, 0x4005DDD4, …).
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
  | slot 1..5 (SYS+0x7C0) | field list 0x40043F54 (still assembly) | SYS+0x7C8 + 4 × (slot − 1) | 0x1000 |
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

## Not yet rewritten (next batches)

- Remaining clock users: `sys_mmu_init` and 0x400429E4 (CP15, not emulated), and 0x4003E00C..0x4003E314 (with TCON).
- (USB stack: done, see USB.md.)
- The display engine and TCON code (DE2 0x01000000, functions of up to 5 KB).
- The next region (0x4004B0A0 onwards: POST and factory flow).
