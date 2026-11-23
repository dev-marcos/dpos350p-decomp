/*
 * moving_average_and_crossing_alignment_search -- stock 0x40040114 (656 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Only when bytes SYS+0x274, +0x275 and +0x279 are all set. With the trigger channel c = byte SYS+0x27C, h = u16 SYS+0x282, s = SYS+0x558 x h x 3 and t = byte SYS+0x1B1 x SYS+0x62C x h: copies n = 2s + 2t + 1 samples of trace SYS+0x7B4[c] from index SYS+0x2A0 - t - s to 0x416431E8, smooths them with an s-point moving average (result stored s - 1 places back, when s != 0), then searches from the middle (n/2 - s/2) to the left and to the right for the first crossing of the level SYS+0x1C0 x (s16) SYS+0x27E (rising, or falling when byte SYS+0x27B is set). The distances are limited to SYS+0x298; u16 SYS+0x29C = the nearer one (negative for the left one), the right one when both are equal, 0 when none.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "util.h"
#include "sys_info.h"

#define SYS 0x40544E68u
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))

#define AVG 0x416431E8u

void moving_average_and_crossing_alignment_search(void)
{
    u32 ch = SI(trigger_channel), buf = W32(SYS + 0x7B4 + 4 * ch), h, s, t, lvl, n, sum, l, lim, old;
    int mid, k, fl = 0, fr = 0, dl = 0, dr = 0, off = 0;
    if (!SI(f_274) || !SI(f_275) || !SI(f_279))
        return;
    h = SI(f_282);
    s = SI(f_558) * h * 3;
    lvl = SI(acq_geometry_1c0) * (u32)(int)(short)SI(trigger_level);
    t = SI(repeat_factor) * (SI(acq_geometry_62c) * h);
    l = SI(f_2a0) - t - s;
    n = 2 * s + 2 * t + 1;
    copy_u16((unsigned short *)AVG, (const unsigned short *)(buf + 2 * l), n);
    if (s != 0 && (int)n > 0) {
        sum = 0;
        for (l = 0; (int)l < (int)n; l++) {
            sum += H16(AVG + 2 * l);
            if ((int)l >= (int)(s - 1)) {
                old = H16(AVG + 2 * (l + 1 - s));
                H16(AVG + 2 * (l + 1 - s)) = (unsigned short)((int)sum / (int)s);
                sum -= old;
            }
        }
    }
    mid = (int)n / 2 - (int)s / 2;
    if (SI(trigger_edge) == 0) {
        for (k = mid; k > 0; k--)
            if ((int)H16(AVG + 2 * k) < (int)lvl && (int)H16(AVG + 2 * k + 2) >= (int)lvl) {
                fl = 1;
                dl = mid - k;
                break;
            }
        for (k = mid; k < (int)n; k++)
            if ((int)H16(AVG + 2 * k) < (int)lvl && (int)H16(AVG + 2 * k + 2) >= (int)lvl) {
                fr = 1;
                dr = k - mid;
                break;
            }
    } else {
        for (k = mid; k > 0; k--)
            if ((int)lvl < (int)H16(AVG + 2 * k) && (int)lvl >= (int)H16(AVG + 2 * k + 2)) {
                fl = 1;
                dl = mid - k;
                break;
            }
        for (k = mid; k < (int)n; k++)
            if ((int)lvl < (int)H16(AVG + 2 * k) && (int)lvl >= (int)H16(AVG + 2 * k + 2)) {
                fr = 1;
                dr = k - mid;
                break;
            }
    }
    lim = SI(f_298);
    if (lim < (u32)dl)
        dl = (int)lim;
    if (lim < (u32)dr)
        dr = (int)lim;
    if (fl & fr)
        off = dl >= dr ? dr : -dl;
    else if (fl)
        off = -dl;
    else if (fr)
        off = dr;
    SI(f_29c) = (unsigned short)off;
}
