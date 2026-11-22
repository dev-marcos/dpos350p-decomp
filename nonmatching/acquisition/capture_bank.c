/*
 * capture_bank -- stock 0x400406C8 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * capture_block(i, 0x42DF76D4 + i x 0x2134) (signed 16-bit product).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"

void capture_bank(u32 i)
{
    capture_block(i, 0x42DF76D4u + (int)(short)i * 0x2134);
}
