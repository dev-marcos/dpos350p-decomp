/*
 * disk_mount -- stock 0x40049B80 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * f_mount(&fs, "0:", 1).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

int disk_mount(void)
{
    return f_mount(g_fatfs0, STR_DRIVE0, 1);
}
