/*
 * zoom_index_add -- stock 0x4004245C (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SYS+0x598 += d, then 0x400423B8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "sys_info.h"

void zoom_index_add(u32 d)
{
    SI(zoom_index) = SI(zoom_index) + d;
    zoom_index_fit();
}
