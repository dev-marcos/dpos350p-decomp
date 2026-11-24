/*
 * fb_clear_current -- stock 0x40034454 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clear the buffer whose address is stored at 0x40095498.
 */
#include "hw/platform.h"
#include "drivers/sdmmc.h"
#include "stock_asm.h"
#include "ui/draw.h"
#include "util.h"
#include "shared/framebuffer.h"

void fb_clear_current(void) { fill_u16(*(unsigned short **)0x40095498u, 0, FB_PIXELS); }
