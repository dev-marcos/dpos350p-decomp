/*
 * zoom_index_set -- stock 0x400424B8 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SYS+0x598 = v, then 0x400423B8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"

void zoom_index_set(u32 v)
{
    SYS8(0x598) = v;
    zoom_index_fit();
}
