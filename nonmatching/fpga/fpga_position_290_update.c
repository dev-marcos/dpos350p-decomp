/*
 * fpga_position_290_update -- stock 0x40063798 (148 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Reads FPGA flag 0x18 (fpga_cmd18_read). Flag set: word SYS+0x290 = (13-bit value of selector 0x14) - SYS+0x28C, plus SYS+0x520 when negative. Else: 0 when u64 SYS+0x30 < SYS+0x28C, otherwise (u64 SYS+0x30 - SYS+0x28C) mod SYS+0x520 (the remainder of __aeabi_uldivmod).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

void fpga_position_290_update(void)
{
    u32 r;
    fpga_cmd18_read();
    if (SYS8(0x279)) {
        u32 a, v;
        fpga_selector_command_write(0x14);
        a = fpga_data_byte_read();
        v = fpga_data_byte_read() | (a << 8);
        r = (v & 0x1FFF) - SYS32(0x28C);
        if ((int)r < 0)
            r += SYS32(0x520);
    } else {
        unsigned long long t = *(volatile unsigned long long *)(0x40544E68u + 0x30);
        u32 x = SYS32(0x28C);
        if (t < x)
            r = 0;
        else
            r = (u32)((t - x) % SYS32(0x520));
    }
    SYS32(0x290) = r;
}
