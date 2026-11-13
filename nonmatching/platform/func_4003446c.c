/*
 * fb_clear_all -- stock 0x4003446C (116 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clear the four 16-bit screen buffers and the two 32-bit buffers (0x40CFF978, 0x40F576B0), then present.
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

void fb_present(void);

void fb_clear_all(void)
{
    fill_u16(fb_front, 0, FB_PIXELS);
    fill_u16(fb_back, 0, FB_PIXELS);
    fill_u16((unsigned short *)0x420E4E84u, 0, FB_PIXELS);
    fill_u16((unsigned short *)0x4233CBBCu, 0, FB_PIXELS);
    fill_u32((u32 *)0x40CFF978u, 0, FB_PIXELS);
    fill_u32((u32 *)0x40F576B0u, 0, FB_PIXELS);
    fb_present();
}
