/*
 * trigger_level_auto -- stock 0x4006382C (524 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Picks the trigger level u16 SYS+0x27E: the L = u16 SYS+0x632 candidate levels min + i x range / (L + 1) (i = 1..L; min / range = words +4 / +8 of SYS+0xB28, or SYS+0xB44 with SYS+0x27C set) are scored by the number of crossings (rising or falling) of the samples (W32 SYS+0x7B4 / +0x7B8 + 0x14, interleaved by SYS+0x558, u16 SYS+0x7BC - 0x14 per stream, each >> SYS+0x1BC); it takes the level with the fewest, or the middle one when its count x 4 is below the fewest x 5.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "util.h"
#include "sys_info.h"
#include "shared/access.h"

#define SYS 0x40544E68u

/* register shift as the core does it: the low byte of n, 32 and more give 0 */
static u32 shr(u32 v, u32 n)
{
    n &= 0xFF;
    return n >= 32 ? 0 : v >> n;
}

void trigger_level_auto(void)
{
    u32 cnt[10];
    u32 st, n, base, lvb, seg, k, L, i, sh, idx, mn, c;
    int prev, cur, lv;
    volatile unsigned short *p, *q;
    st = SI(bank_count);
    n = SI(capture_samples) & 0xFFFF;
    if (SI(trigger_channel)) {
        base = SI(ch2_buffer);
        lvb = SYS + 0xB44;
    } else {
        base = SI(ch1_buffer);
        lvb = SYS + 0xB28;
    }
    fill_u32(cnt, 0, 10);
    seg = (u32)(((int)n - 0x14) / (int)st) & 0xFFFF;
    for (k = 0; (int)k < (int)st; k++) {
        p = (volatile unsigned short *)(base + 0x14 + 2 * k);
        sh = SI(sample_shift);
        prev = (int)shr(*p, sh);
        if ((int)seg <= 1)
            continue;
        q = p + st;
        for (c = seg - 1; c; c--) {
            cur = (int)shr(*q, sh);
            L = SI(f_632);
            for (i = 0; (int)i < (int)L;) {
                i++;
                lv = (int)(i * W32(lvb + 8) / (L + 1) + W32(lvb + 4));
                if (prev < lv) {
                    if (cur >= lv)
                        cnt[i - 1]++;
                } else if (lv < prev && lv >= cur) {
                    cnt[i - 1]++;
                }
            }
            prev = cur;
            q += st;
        }
    }
    L = SI(f_632);
    mn = 0xFFFFFFFFu;
    idx = 0;
    for (i = 0; (int)i < (int)L; i++) {
        if (cnt[i] < mn) {
            idx = i;
            mn = cnt[i];
        }
    }
    if ((unsigned long long)cnt[L >> 1] * 4 < (unsigned long long)mn * 5)
        idx = L >> 1;
    SI(trigger_level) = (unsigned short)((idx + 1) * W32(lvb + 8) / (L + 1) + W32(lvb + 4));
}
