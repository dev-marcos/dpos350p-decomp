/*
 * trace_shift_samples -- stock 0x4003C158 (428 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When the horizontal position changed: q = (SYS+0x5E0 - base) x scale / div / step (64-bit signed; mode 0/1: base SYS+0x570, scale +0x600, div SYS+0x590, step +0x604; other modes: base SYS+0x578, scale +0x618, div SYS+0x5A0, step +0x61C), the view offsets (+0x608 / +0x60C, or +0x620 / +0x624) are moved back by q and the stored samples of trace tr (buffer 0x411AF3E8 + tr x 0x927C0) are shifted by s = q x the samples per step (+0x5F8 or +0x610): for each of the u16 SYS+0x5F4 segments (start SYS+0x2C4[k], length SYS+0x454[k]) the samples move down by s (s > 0) or up by -s (s < 0), keeping the segment.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "processing.h"
#include "sys_info.h"
#include "shared/access.h"

#define SYS 0x40544E68u
#define P(o) (SYS + 0x5F4 + (o))
#define TRBUF(ch) (0x411AF3E8u + (ch) * 0x927C0u)

void trace_shift_samples(u32 tr)
{
    long long d = SI(f_5e0), q, s;
    volatile unsigned short *buf, *src, *dst;
    u32 n, k, st, ln;
    int cnt;
    if (!mode_not_0_or_1(tr)) {
        q = (long long)((unsigned long long)(d - SI(position_ref)) * W32(P(0xC))) / S64(SYS + 0x590);
        q = q / (long long)W32(P(0x10));
        W32(P(0x14)) -= (u32)q;
        W32(P(0x18)) -= (u32)q;
        s = (long long)((unsigned long long)q * W32(P(4)));
    } else {
        q = (long long)((unsigned long long)(d - SI(view_position)) * W32(P(0x24))) / SI(zoom_period);
        q = q / (long long)W32(P(0x28));
        W32(P(0x2C)) -= (u32)q;
        W32(P(0x30)) -= (u32)q;
        s = (long long)((unsigned long long)q * W32(P(0x1C)));
    }
    if (s == 0)
        return;
    n = H16(P(0));
    if (n == 0)
        return;
    buf = (volatile unsigned short *)TRBUF(tr);
    for (k = 0; k < n; k++) {
        st = W32(SYS + 0x2C4 + 4 * k);
        ln = W32(SYS + 0x454 + 4 * k);
        if (s > 0) {
            cnt = (int)(ln - (u32)s);
            if (cnt > 0) {
                dst = buf + st;
                src = buf + (st + (u32)s);
                do
                    *dst++ = *src++;
                while (--cnt);
            }
        } else {
            cnt = (int)((u32)s + ln);
            if (cnt > 0) {
                dst = buf + (ln + st);
                src = buf + ((u32)s + ln + st);
                do
                    *dst-- = *src--;
                while (--cnt);
            }
        }
    }
}
