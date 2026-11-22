/*
 * copy_u16 -- stock 0x40035728 (28 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Copy n halfwords.
 */
#include "hw/platform.h"

#define FB_PIXELS 0x95B50u                     /* 1022 * 600 */
#define fb_front  ((unsigned short *)0x41FB8FE8u)
#define fb_back   ((unsigned short *)0x41BFBF68u)
void copy_u16(unsigned short *d, const unsigned short *s, u32 n);
void fill_u16(unsigned short *d, unsigned short v, u32 n);
void fill_u32(u32 *d, u32 v, u32 n);
void dcache_clean_inv_all(void);
void dcache_clean_inv_all_t(void);

void copy_u16(unsigned short *d, const unsigned short *s, u32 n)
{
    while (n) { *d++ = *s++; n--; }
}
