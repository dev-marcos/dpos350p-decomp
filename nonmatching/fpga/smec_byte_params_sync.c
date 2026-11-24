/*
 * smec_byte_params_sync -- stock 0x40063CD0 (564 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * For each of the 8 bytes SYS+0x674..+0x67B that differs from its copy SYS+0xBB8+i: updates the copy, sends SMEC job 0x19+i (fpga_param_updater, size 1), commits 0xB9+i and writes the byte to FPGA selector 0xA6+i. Then writes 0 and 1 to selector 0x11.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"
#include "shared/access.h"

#define SYS 0x40544E68u

void smec_byte_params_sync(void)
{
    u32 i, v;
    for (i = 0; i < 8; i++) {
        v = B8(SYS + 0x674 + i);
        if (B8(SYS + 0xBB8 + i) != v) {
            B8(SYS + 0xBB8 + i) = (unsigned char)v;
            fpga_param_updater(0x19 + i, 1, W32(SYS + 0xBB8 + i), v);   /* the word load is unaligned for odd i */
            smec_commit(0xB9 + i);
            fpga_selector_command_write(0xA6 + i);
            fpga_data_byte_write(B8(SYS + 0xBB8 + i));
        }
    }
    fpga_selector_command_write(0x11);
    fpga_data_byte_write(0);
    fpga_selector_command_write(0x11);
    fpga_data_byte_write(1);
}
