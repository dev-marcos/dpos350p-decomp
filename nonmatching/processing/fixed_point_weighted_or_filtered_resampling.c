/*
 * fixed_point_weighted_or_filtered_resampling -- stock 0x4005D2E4 (372 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Windowed-sinc zoom of the u16 workspace ws around index half: k = SYS+0x554, c = (len x k / m) x 12 / 10 input samples starting c/2 before half, cnt = m x c / k outputs; output o sums x[j] x w(p - 10000 j) over the c inputs, with p = o x (10000 / (m / k)) and w(0) = 10000, w(t) = sine_table_lookup(t) / t, then divides by 10000 (negative results to 0) into 0x416431E8; finally (u16) len samples, centred and moved back by the 64-bit stack argument, are copied to ws from half - len/2.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define F32(a) (*(volatile float *)(a))

int sine_table_lookup(int x);
void copy_u16(unsigned short *d, const unsigned short *s, u32 n);

#define OUT16 0x416431E8u

void fixed_point_weighted_or_filtered_resampling(u32 ws, u32 half, u32 len, u32 m, long long shift)
{
    u32 k = W32(SYS + 0x554), step, cnt, src, dst, l16, out, ph, acc, o;
    long long t;
    int c, j, ph2, w, r;
    t = (long long)((unsigned long long)len * k) / (long long)m;
    step = 10000u / (m / k);
    c = (int)((long long)((unsigned long long)t * 12) / 10);
    src = ws + half * 2 - ((u32)(c + (int)((u32)c >> 31)) & ~1u);
    cnt = (u32)c * m / k;
    l16 = len & 0xFFFF;
    dst = ws + half * 2 - (l16 & ~1u);
    if ((int)cnt > 0) {
        out = OUT16;
        ph = 0;
        for (o = cnt; o; o--) {
            acc = 0;
            if (c > 0) {
                ph2 = (int)ph;
                for (j = 0; j < c; j++) {
                    if (ph2 == 0)
                        w = 10000;
                    else
                        w = sine_table_lookup(ph2) / ph2;
                    acc += (u32)w * H16(src + 2 * (u32)j);
                    ph2 -= 10000;
                }
            }
            r = (int)acc / 10000;
            if (r < 0)
                r = 0;
            H16(out) = (unsigned short)r;
            out += 2;
            ph += step;
        }
    }
    copy_u16((unsigned short *)dst,
             (const unsigned short *)(OUT16 + ((cnt - l16 + ((cnt - l16) >> 31)) & ~1u) - ((u32)shift << 1)), l16);
}
