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

#define FB_PIXELS 0x95B50u                     /* 1022 * 600 */
#define fb_front  ((unsigned short *)0x41FB8FE8u)
#define fb_back   ((unsigned short *)0x41BFBF68u)

void fb_clear_current(void) { fill_u16(*(unsigned short **)0x40095498u, 0, FB_PIXELS); }
