/*
 * simple_integer_index_mapping_nearest_neighbour -- stock 0x4005C678 (324 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Stretches a trace by linear interpolation (the name comes from an earlier project; the code interpolates). k = W32(SYS+0x554), m = the target scale; nothing happens when k >= m. n = (int)(w * 1.2 * k / m) source samples centred on src[center] are stretched to L = m * n / k samples in the buffer 0x416431E8 (8-bit fixed-point steps), then copy_u16 copies w of them, centred and moved back by `shift`, to src[center - w / 2]. The stock code divides by k with udiv, so k = 0 gives 0 there; that case is not tested.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void copy_u16(unsigned short *d, const unsigned short *s, u32 n);
#define SYS 0x40544E68u
#define W32(a) (*(volatile u32 *)(a))

void simple_integer_index_mapping_nearest_neighbour(u32 src_a, u32 center_a, u32 w, u32 m, u32 shift_a, u32 unused)
{
    volatile unsigned short *src = (volatile unsigned short *)src_a;
    int center = (int)center_a, shift = (int)shift_a;
    u32 k = W32(SYS + 0x554), t, acc, sl;
    volatile unsigned short *out = (volatile unsigned short *)0x416431E8u, *p, *o;
    unsigned short *q;
    int n, L, i, pos, prev, last, cur, diff;
    if (k >= m)
        return;
    n = (int)((double)w * 1.2 * (double)k / (double)m);
    p = src + (center - n / 2);
    q = (unsigned short *)(src + (center - ((int)w >> 1)));
    L = (int)((m * (u32)n) / k);
    last = *p++;
    out[0] = (unsigned short)last;
    i = 1;
    prev = 0;
    pos = (int)m;
    if (L >= 2) {
        do {
            t = k * (u32)i;
            cur = *p;
            acc = (t - (u32)prev) << 8;
            diff = cur - last;
            o = out + i;
            while (pos >= (int)t) {
                sl = acc / m;
                t += k;
                i++;
                *o++ = (unsigned short)(last + ((int)(sl * (u32)diff) >> 8));
                acc += k << 8;
            }
            last = cur;
            prev = pos;
            p++;
            pos += (int)m;
        } while (i < L);
    }
    copy_u16(q, (const unsigned short *)((u32)(out + (L - (int)(unsigned short)w) / 2) - 2u * (u32)shift),
             (unsigned short)w);
}
