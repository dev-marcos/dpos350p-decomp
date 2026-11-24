/*
 * peak_hold_decimate -- stock 0x40061608 (112 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * In-place peak decimation of n s16 samples: output k = the maximum (at least -28671) of the samples start .. (u32)pos, with pos = k x step (d0) and start the previous end; nothing when n = 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "shared/access.h"

#define SYS 0x40544E68u

void peak_hold_decimate(volatile short *buf, u32 n, double step)
{
    double pos = 0.0;
    u32 start = 0, end, cnt;
    volatile short *out = buf, *p;
    int mx;
    if (n == 0)
        return;
    do {
        end = (u32)pos;
        mx = -28671;
        cnt = end - start + 1;
        p = buf + start;
        while (cnt) {
            if (*p >= mx)
                mx = *p;
            p++;
            cnt--;
        }
        *out++ = (short)mx;
        start = end;
        pos += step;
    } while (--n);
}
