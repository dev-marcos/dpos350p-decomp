/*
 * fpga_params_0e_13_write -- stock 0x400634F0 (224 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * cursor_27e_clamp, then FPGA parameters 0x0E..0x13 for the channel chosen by SYS+0x27C (0: SYS+0xE0, SYS+0x60, slot SYS+0x1C4[SYS+0x41], words +0x18 / +0x38 of 0x42E53A0C; else SYS+0x1A0, SYS+0x120, slot [SYS+0x105], words +0x14 / +0x34), parameter 0x10 being the 2-byte SYS+0x27E; then smec_commit(0xB8) and fpga_cmd17_write.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fpga.h"
#include "processing.h"
#include "sys_info.h"

#define PB32(o) (*(volatile u32 *)(0x42E53A0Cu + (o)))

void fpga_params_0e_13_write(void)
{
    u32 a, b, c, d, e, w;
    cursor_27e_clamp();
    if (SI(trigger_channel) == 0) {
        a = SI(f_0e0);
        b = SI(ch1_offset);
        d = PB32(0x18);
        e = PB32(0x38);
        c = *(volatile u32 *)(0x40544E68u + 0x1C4 + 4 * SI(ch1_vdiv_index));
    } else {
        a = SI(f_1a0);
        b = SI(ch2_offset);
        d = PB32(0x14);
        e = PB32(0x34);
        c = *(volatile u32 *)(0x40544E68u + 0x1C4 + 4 * SI(ch2_vdiv_index));
    }
    fpga_param_updater(0xE, 4, a, a);
    fpga_param_updater(0xF, 4, b, b);
    w = SI(trigger_level) | ((u32)SI(f_280) << 16);      /* the stock reads the word at SYS+0x27E unaligned */
    fpga_param_updater(0x10, 2, w, (u32)(int)(short)SI(trigger_level));
    fpga_param_updater(0x11, 4, c, c);
    fpga_param_updater(0x12, 4, d, d);
    fpga_param_updater(0x13, 4, e, e);
    smec_commit(0xB8);
    fpga_cmd17_write();
}
