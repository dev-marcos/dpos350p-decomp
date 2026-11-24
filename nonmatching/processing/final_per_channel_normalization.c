/*
 * final_per_channel_normalization -- stock 0x4003FE14 (464 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Only when the u64 (ch 0: SYS+0xB78, else SYS+0xBA0) is at least (0x8D7F250 / SYS+0x524) x byte SYS+0x1B1. Works on the u16 trace SYS+0x7B4[ch] from index 10 to SYS+0x7BC - 10, split into half-periods at each crossing of the middle level m (SYS+0xB6C / +0xB94); the top / bottom references are SYS+0xB60 / +0xB64 (ch 1: +0xB88 / +0xB8C). For each half-period above m (mean >= m) whose peak exceeds m by more than (top - m) / 3, the samples are stretched so that the peak reaches top: v = m + (v - m) x (top - m) / (peak - m); below m the same with the bottom reference. The last sample of each half-period is not rescaled (stock).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "sys_info.h"
#include "shared/access.h"

#define SYS 0x40544E68u
#define U64(a) (*(volatile unsigned long long *)(a))

void final_per_channel_normalization(u32 ch)
{
    u32 buf = W32(SYS + 0x7B4 + 4 * ch), top, bot, end, sum, i, st, k;
    int mid, a, b, a3, b3, mx, mn, v, up, dn, cnt, avg;
    unsigned long long d, lim;
    if (ch == 0) {
        top = SI(f_b60);
        bot = SI(f_b64);
        mid = (int)SI(f_b6c);
        d = (unsigned long long)SI(f_b78) | ((unsigned long long)SI(f_b7c) << 32);
    } else {
        top = SI(f_b88);
        bot = SI(f_b8c);
        mid = (int)SI(f_b94);
        d = U64(SYS + 0xBA0);
    }
    lim = (unsigned long long)(0x8D7F250u / SI(f_524)) * SI(repeat_factor);
    if (d < lim)
        return;
    end = SI(f_7bc) - 10;
    a = (int)(top - (u32)mid);
    b = (int)((u32)mid - bot);
    if ((int)end <= 10)
        return;
    b3 = b / 3;
    a3 = a / 3;
    sum = 0;
    mx = 0;
    mn = 0xFFFFFF;
    st = 10;
    for (i = 10; (int)i < (int)end; i++) {
        v = H16(buf + 2 * i);
        if (v >= mx)
            mx = v;
        if (v <= mn)
            mn = v;
        sum += (u32)v;
        if (v < mid) {
            if ((int)H16(buf + 2 * i + 2) >= mid)
                goto seg;
        } else if (mid < v) {
            if (mid >= (int)H16(buf + 2 * i + 2))
                goto seg;
        }
        if (i + 1 != end)
            continue;
    seg:
        up = (int)((u32)mx - (u32)mid);
        dn = (int)((u32)mid - (u32)mn);
        cnt = (int)(i - st);
        avg = (int)sum / (cnt + 1);
        if (avg < mid) {
            if (b3 < dn && (int)st < (int)i)
                for (k = st; cnt; cnt--, k++)
                    H16(buf + 2 * k) = (unsigned short)((u32)mid + (u32)((int)(((u32)H16(buf + 2 * k) - (u32)mid) * (u32)b) / dn));
        } else {
            if (a3 < up && (int)st < (int)i)
                for (k = st; cnt; cnt--, k++)
                    H16(buf + 2 * k) = (unsigned short)((u32)mid + (u32)((int)(((u32)H16(buf + 2 * k) - (u32)mid) * (u32)a) / up));
        }
        sum = 0;
        mx = 0;
        mn = 0xFFFFFF;
        st = i;
    }
}
