/*
 * memset_w -- stock 0x400356E4 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * memset(dst, value, n): b __aeabi_memset (0x4005C604) with the length and value registers swapped.
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

void *__aeabi_memset_entry(void *d, unsigned int n, int v);

void *memset_w(void *d, int v, unsigned int n) { return __aeabi_memset_entry(d, n, v); }
