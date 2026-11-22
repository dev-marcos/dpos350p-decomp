/*
 * fpga_counter_step -- stock 0x40063C34 (124 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Counter ch: when the target SYS+0x67C+ch differs from the last value SYS+0xBC0+ch, writes the direction (1 = down) to selector 0x96 + 2ch and |difference| pairs (1, 0) to selector 0x97 + 2ch, then keeps the target as the last value.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

void fpga_counter_step(u32 ch)
{
    int d;
    u32 cur = SYS8(0x67C + ch), old = SYS8(0xBC0 + ch);
    if (cur == old)
        return;
    d = (int)cur - (int)old;
    fpga_selector_command_write((2 * ch + 0x96) & 0xFF);
    if (d < 0) {
        d = -d;
        fpga_data_byte_write(1);
    } else {
        fpga_data_byte_write(0);
    }
    fpga_selector_command_write((2 * ch + 0x97) & 0xFF);
    for (; d > 0; d--) {
        fpga_data_byte_write(1);
        fpga_data_byte_write(0);
    }
    SYS8(0xBC0 + ch) = SYS8(0x67C + ch);
}
