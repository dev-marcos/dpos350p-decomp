/*
 * lcd_readl -- stock 0x4003DFFC (8 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Register read (copy in the LCD source file).
 */
#include "hw/platform.h"
#include "drivers/ccu.h"
#include "drivers/display.h"
#include "drivers/timer.h"
#include "libc.h"
#include "util.h"
#include "shared/drivers.h"

typedef struct { u32 de_base, tcon_base, width, height, bpp, fb; } disp_t;   /* 0x42E68B68 */

u32 lcd_readl(u32 addr) { return REG32(addr); }
