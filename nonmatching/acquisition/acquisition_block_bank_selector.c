/*
 * acquisition_block_bank_selector -- stock 0x4006338C (164 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * FPGA selector 0x15 with the bank of block i, only while word SYS+0x554 < SYS+0x55C (unsigned): with divisor SYS+0x1B0 = 1 the byte i & 7; else with trigger source SYS+0x27C = 0 the byte 2 x (i mod 4), otherwise 2 x (i mod 4) + 1 (i mod 4 computed as i - ((i + bit 31) & ~3)).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "drivers/fpga_bus.h"
#include "sys_info.h"

#define SYS 0x40544E68u

void acquisition_block_bank_selector(u32 i)
{
    u32 q = i - ((i + (i >> 31)) & ~3u), v;

    if (SI(f_554) >= SI(f_55c))
        return;
    fpga_selector_command_write(0x15);
    if (SI(f_1b0) == 1)
        v = i & 7;
    else if (SI(trigger_channel) == 0)
        v = (q & 0x7F) << 1;
    else
        v = 1 + 2 * q;
    fpga_data_byte_write(v & 0xFF);
}
