/*
 * memcpy -- stock 0x40035724 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * b __aeabi_memcpy.
 */
#include "hw/platform.h"
#include "drivers/sdmmc.h"
#include "libc.h"
#include "stock_asm.h"
#include "util.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void *__aeabi_memcpy_raw(void *d, const void *s, unsigned int n) __asm__("__aeabi_memcpy");

#define FB_PIXELS 0x95B50u                     /* 1022 * 600 */
#define fb_front  ((unsigned short *)0x41FB8FE8u)
#define fb_back   ((unsigned short *)0x41BFBF68u)

void *memcpy(void *d, const void *s, unsigned int n) { return __aeabi_memcpy_raw(d, s, n); }
