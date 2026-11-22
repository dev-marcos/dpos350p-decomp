/*
 * fpga_counters_init -- stock 0x40063BC0 (116 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * For the 8 counters i: clears SYS+0x674+i and the last value SYS+0xBC0+i, selector 0x96 + 2i = 1, then 130 pairs (1, 0) to selector 0x97 + 2i.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

void fpga_counters_init(void)
{
    int i, k;
    for (i = 0; i < 8; i++) {
        SYS8(0x674 + i) = 0;
        SYS8(0xBC0 + i) = 0;
        fpga_selector_command_write((0x96 + 2 * i) & 0xFF);
        fpga_data_byte_write(1);
        fpga_selector_command_write((0x97 + 2 * i) & 0xFF);
        for (k = 0x82; k; k--) {
            fpga_data_byte_write(1);
            fpga_data_byte_write(0);
        }
    }
}
