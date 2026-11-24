/*
 * fpga_position_290_update -- stock 0x40063798 (148 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Reads FPGA flag 0x18 (fpga_cmd18_read). Flag set: word SYS+0x290 = (13-bit value of selector 0x14) - SYS+0x28C, plus SYS+0x520 when negative. Else: 0 when u64 SYS+0x30 < SYS+0x28C, otherwise (u64 SYS+0x30 - SYS+0x28C) mod SYS+0x520 (the remainder of __aeabi_uldivmod).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"
#include "sys_info.h"

void fpga_position_290_update(void)
{
    u32 r;
    fpga_cmd18_read();
    if (SI(f_279)) {
        u32 a, v;
        fpga_selector_command_write(0x14);
        a = fpga_data_byte_read();
        v = fpga_data_byte_read() | (a << 8);
        r = (v & 0x1FFF) - SI(trigger_offset_1c);
        if ((int)r < 0)
            r += SI(record_length);
    } else {
        unsigned long long t = *(volatile unsigned long long *)(0x40544E68u + 0x30);
        u32 x = SI(trigger_offset_1c);
        if (t < x)
            r = 0;
        else
            r = (u32)((t - x) % SI(record_length));
    }
    SI(fpga_position) = r;
}
