# Whole-system simulator (prototype)

`tools/sim/` runs a complete application image on an emulated V3s (Unicorn, Cortex-A7)
from `Reset_Handler` (0x4008CB28). Every peripheral access without a model is logged together
with the function that made it, so the log shows what to model next.

```
python3 -m tools.sim.sim [--image build/extract/app.bin] [--insns 600M] [--sd card.img] [--trace-mmio]
```

`screenshot(sim, path)` in `tools/sim/sim.py` renders the DE2 UI layer to a PNG.

## Status (stock ROM)

The stock image reaches the power-on self test and draws it on the emulated display:

| POST line | Meaning | Result |
|---|---|---|
| 磁盘初始化 | disk (SD card) init | 成功 (OK) |
| FPGA检测 | FPGA identity | 成功 |
| 加密芯片检测 | security chip | 成功 |
| ADC检测 | ADC / acquisition | waits: needs interrupts + CSI capture |

## Peripheral models (`tools/sim/devices.py`)

| Block | Address | Model | Evidence |
|---|---|---|---|
| CCU | 0x01C20000 | every PLL reports LOCK | DRIVERS.md |
| Timers / AVS | 0x01C20C00 | AVS counters advance on every read (delays end quickly) | DRIVERS.md |
| UART0..2 | 0x01C28000.. | TX captured (USR always "not full") | stock putc |
| SPI0 + SPI NOR | 0x01C68000 | streaming exchange (XCH first, then TX FIFO), W25Q-style flash, 2 MiB area used: settings at 0x1EB000..0x200000; blank flash means defaults are written back | stock sys_spi_cpu_transfer |
| SD/MMC0 + SDHC card | 0x01C0F000 | commands, PIO FIFO and IDMAC descriptors; raw image (default: blank 64 MiB) | stock SD driver |
| FPGA on PE bus | PE0/PE2/PE3/PE8..15 | pin-level strobe protocol; 0x01 identity = A5 A5 5A 5A, 0x60 security chip = present, jobs 0x64..0x6B always ready with result 0 | Hack FPGA_PROTOCOL.md, SMEC.md; stock code |
| DE2 display | 0x01100000 | framebuffer read only: UI channel 1 (0x01104000), RGB565, 1022x600, pitch 2044, at 0x41FB8FE8 | registers written by the stock code |
| Low address space | 0x00000000..0x00FFFFFF | zero-filled memory | see "Firmware quirks" |

Assumptions:
- **SD long responses.** The stock driver asks for a long response on ACMD41 and reads the OCR
  from RESP3. The model copies a short reply into all four response words.
- **FPGA job results.** The job engine (0x64..0x6B) returns 0 for every job. The real transform
  is OPEN.

## Firmware quirks found with the simulator

- **Copy from a NULL pointer.** While creating `/Captures/data.cap`, the firmware calls
  `copy_u16(dst, NULL, n)`. This copies from address 0, which does not fault on the SoC.
- **Halt on a failed check.** A failed POST check halts the firmware (`b .`) after drawing
  异常 in red.
- **Different status selector.** This build polls job status on selector 0x64. The Hack notes
  describe 0x65.

## Next steps

1. Add interrupts: GIC distributor/CPU interface (0x01C81000/0x01C82000) and IRQ entry into the
   CPU. These are needed for the timer tick, CSI and USB.
2. Model CSI (0x01CB4000) frame capture, so the FPGA's sample stream lands in RAM and raises
   "frame done". This is where the virtual probe signal will enter.
3. Touch (GT911 on bit-banged I2C, PG6/PG7) and the buttons.
4. Live display window and input.
5. Run the same scenario on three images, as asked:
   - the stock ROM;
   - the image rebuilt from assembly (byte-identical, so it must behave the same);
   - a TEST image with the C rewrites linked in place of the assembly.
