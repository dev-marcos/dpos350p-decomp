/*
 * trigger_window_crossing -- stock 0x400403AC (564 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Only when byte SYS+0x275 is set and ch is the trigger channel (byte SYS+0x27C). Around the centre c = (s64 SYS+0x570 / s64 SYS+0x590) + SYS+0x7BC / 2 of trace SYS+0x7B4[ch], within c +- 6 x SYS+0x55C / SYS+0x554, it searches left and right for the nearest sample k where the trace crosses the level SYS+0x1C0 x (s16) SYS+0x27E between k - 1 and k + 1 (rising: before < level <= after, or before = level < after; falling with byte SYS+0x27B set, mirrored). u16 SYS+0x29E = the offset of the nearer crossing from c (the right one on a tie). When only the right one is found, the left distance is the window edge; when none is found nothing is stored.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define S64(a) (*(volatile long long *)(a))
#define U64(a) (*(volatile unsigned long long *)(a))

#define T16(b, k) ((int)H16((b) + 2u * (u32)(k)))

static int crossing(u32 b, int k, int lvl, u32 edge)
{
    int a = T16(b, k - 1), c = T16(b, k + 1);
    if (edge == 0)
        return (a < lvl && c >= lvl) || (a == lvl && c > lvl);
    return (a > lvl && c <= lvl) || (a == lvl && c < lvl);
}

void trigger_window_crossing(u32 ch)
{
    int lvl = (int)(W32(SYS + 0x1C0) * (u32)(int)(short)H16(SYS + 0x27E));
    u32 buf = W32(SYS + 0x7B4 + 4 * ch), w, edge;
    int found = 0, c, lo, hi, l, r, k, v;
    long long q;
    if (!B8(SYS + 0x275))
        return;
    if (ch != B8(SYS + 0x27C))
        return;
    q = S64(SYS + 0x570) / S64(SYS + 0x590);
    c = (int)((u32)q + (W32(SYS + 0x7BC) >> 1));
    w = (W32(SYS + 0x55C) * 6) / W32(SYS + 0x554);
    lo = (int)((u32)c - w);
    hi = (int)(w + (u32)c);
    edge = B8(SYS + 0x27B);
    l = lo;
    r = hi;
    for (k = c; lo < k; k--)
        if (crossing(buf, k, lvl, edge)) {
            found = 1;
            l = k;
            break;
        }
    for (k = c; k < hi; k++)
        if (crossing(buf, k, lvl, edge)) {
            r = k;
            goto done;
        }
    if (!found)
        return;
done:
    v = r - c;
    if (c - l < v)
        v = l - c;
    H16(SYS + 0x29E) = (unsigned short)v;
}
