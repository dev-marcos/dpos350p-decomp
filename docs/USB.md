# USB device stack: origin (build 2025-06-19)

Research done before rewriting the USB functions (about 0x4006AC00..0x4006C400). FNIRSI did not
write this stack from scratch. Evidence levels as in CONTRIBUTING.md.

## Findings

| # | Claim | Level | Evidence |
|---|---|---|---|
| 1 | The core of the stack is **EasyUSB** by VeiLiang (github.com/VeiLiang/EasyUSB, MIT, last commit a605b8d, 2020-07-03): `usb_phy/f1c100s/usb_dev.c` plus the CDC and HID class files. | STATIC-CONFIRMED | 72 EasyUSB strings occur verbatim in the image, typos included: "unsupprot USB_REQ_*", "not suppost", "usb_device_write_data faild! usb not connect!!", "CONFIG_USB_GADGET_DUALSPEED: USBC_TS_MODE_HS", "handle_ep0: ep0 setup end". Per file: usb_dev.c 13/18 strings, usb_cdc.c 18/27, usb_hid.c 19/30. |
| 2 | `usb_device_write_data` keeps EasyUSB's `Timeout = 10000000`. | STATIC-CONFIRMED | The literal 0x00989680 sits in the same pool (0x4006B778) as the "usb_device_write_data len:" and "... Time out!" strings. |
| 3 | The low-level HAL (`USBC_*`, `usb_phy.c`) is Allwinner's sunxi_usb BSP, ported to bare metal. | DERIVED | EasyUSB's usb_phy.c makes 552 `USBC_*` references. The same API appears in the Allwinner linux-3.10 BSP (`drivers/usb/sunxi_usb`), which is GPL-2.0. The FNIRSI-1013D analysis by pecostm32 names the same functions (`USBC_SelectActiveEp`, `USBC_Dev_Ctrl_IsSetupEnd`, ...). |
| 4 | FNIRSI added a Mass Storage class (CBW/CSW, SCSI READ(10)/WRITE(10), GET_MAX_LUN), a UVC descriptor, a device-type switch, and the PHY clock helpers `usb_phy_open/close_clock_Device`. | STATIC-CONFIRMED (strings) / OPEN (source) | 26 USB strings are not in EasyUSB: "Valid CBW 1..4", "SCSI_Invalid_Cmd", "ERR:read10 address out of range!", "usb_mass_get_cfg_descriptor", "usb_uvc_get_cfg_descriptor", "usb init...", ... No public source found yet. |
| 5 | The same code base was used in the FNIRSI-1013D (F1C100s). | DERIVED | Its original firmware has the same strings: pecostm32/FNIRSI-1013D-Hack, `Software reverse engineering/C analysis/USB`. |
| 6 | The device is set up with IRQ 0x67 (103, V3s USB OTG) through `irq_register`. | STATIC-CONFIRMED | 0x4006B6F8: `mov r0,#0x67; bl irq_register`. |

Related note (clock code, not USB):
- The structure of `sys_clock_init` follows xboot's `sys-clock.c` (Jianjun Jiang, GPL-2.0+; the
  F1C100s copy is in EasyUSB `boot/f1c100s/`). Both write PLL_VIDEO `0x?1004107`, then
  PLL_PERIPH, then AHB/APB `0x3180`.
- The FNIRSI version is a V3s variant: different values, CPU frequency as an argument, LOCK wait
  without timeout, MBUS. Not a verbatim copy. DERIVED.

## Consequences for the rewrite

- EasyUSB is the reference for function boundaries, names, argument order and control flow,
  as R0.13b was for FatFs. The stock image stays authoritative: every function is still checked
  in tools/emu (return value, memory, ordered MMIO log). FNIRSI's changes are kept as they are.
- The reference clone lives in `third_party/easyusb`, which is git-ignored. Clone it with:

  `git clone https://github.com/VeiLiang/EasyUSB third_party/easyusb`

- Licence: EasyUSB declares MIT, but the parts derived from the Allwinner BSP may carry GPL-2.0.
  The files in `nonmatching/usb/` are transcriptions of the stock binary that only name the
  reference. No reference source is copied into the repository.
- Names taken from EasyUSB get evidence level DERIVED (`source=EasyUSB:<file>`), never
  "confirmed".

## Sources

- https://github.com/VeiLiang/EasyUSB
- https://github.com/VeiLiang/BoloRTT (`bsp/f1c/drivers/usb_dev.c`, GPL-2.0, same usb_dev core)
- https://github.com/pecostm32/FNIRSI-1013D-Hack (1013D USB analysis)
- https://whycan.com/t_1648.html (Allwinner sunxi_usb BSP, base of the bare-metal port)

## Rewrite status (Phase 5, batch 7)

All 179 functions of the stack are in `nonmatching/usb/<layer>/<name>.c` (`usbc`, `core`,
`mass_storage`, `cdc`, `hid`, `uvc`, `phy`) and EQUIVALENT under tools/emu: same return value,
memory, ordered MMIO log and stub calls. On `main` they are generated from `tools/gen_usb.py`
and `tools/usb_spec/*.py`; on the `readability` branch the C files are the source.

| Layer | Functions | Stock range |
|---|---:|---|
| `USBC_*` HAL (registers, FIFO, endpoints, ISCR, PHY) | 110 | 0x4006C108..0x4006CF5C |
| `usb_dev.c` core (EP0 state machine, IRQ, init) | 26 | 0x4006AA28..0x4006B774 |
| Requests, descriptors, CDC/HID echo | 22 | 0x4006CFB0..0x4006DDE4 |
| Mass storage: bulk-only transport and SCSI | 21 | 0x4006B95C..0x4006C0C8, 0x4006D888..0x4006DD7C |

### Findings

FNIRSI changes to EasyUSB:
- EP0 IN replies keep the caller's buffer (`usb_tx_data`, 0x42E68D08), not the copy in
  `usb_tx_buf`.
- `usb_config_ep_in/out` dropped the 4 KiB FIFO bound check and always use double buffering.
- `sunxi_udc_enable` passes the bit numbers 0, 1, 2, 5 as interrupt masks. The resulting mask
  is 0x07, so DISCONNECT (0x20) is never enabled.
- `USBC_Dev_IsEpStall` discards the stall bit, so it always returns 0. This EasyUSB bug is
  kept, and the "ep0 stall" path is therefore dead.
- `usb_phy_write` does a 32-bit read and an 8-bit write, so the PHY bit address (bits 15:8)
  never reaches the PHY.

The mass storage class is FNIRSI code:
- CBW/CSW handling, and a SCSI table covering TEST UNIT READY, REQUEST SENSE, INQUIRY,
  MODE SENSE 6/10, START STOP, PREVENT ALLOW, READ FORMAT CAPACITIES, READ CAPACITY,
  READ(10), WRITE(10) and VERIFY.
- LUN 0/1. A LUN above 1 gets NOT READY / MEDIUM NOT PRESENT.
- Sense data has a single global buffer, and the LUN argument is ignored.
- The max-LUN byte is read as an unaligned word.

Overlapping entries (armcc tail sharing). The Phase 2 analyser now recognises these:
- 0x4006C4CC, 0x4006C4DC and 0x4006C76C are `mov r0,#0` entries that fall into the store at
  the next address.
- 0x4006D2D8 loads `&current_usb_type` and falls into 0x4006D2DC.
- The CBW decoder at 0x4006BB6C is reached only by a tail branch, so the map keeps it inside
  `usb_mass_out_handler`.

### Not covered

- The 10 000 000-poll timeout of `usb_device_write_data`: it is too long to emulate.
- `usb_get_device_descriptor` for an unknown class type: it uses an unset register (OPEN).
- The disk glue (`usb_mass_disk_read/write`, 0x4006A964 / 0x4006A978): it is outside the USB
  files and is stubbed in the tests.
