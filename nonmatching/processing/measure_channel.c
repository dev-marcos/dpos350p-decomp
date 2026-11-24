/*
 * measure_channel -- stock 0x40032E0C (4684 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Measurements of channel ch (0 or 1; any other value runs as channel 1 without its enable check) from the sample buffer word SYS+0x7B4 + 4 * ch (u16 samples, SYS+0x558 interleaved banks, count u16 SYS+0x7BC). Raw block M (SYS+0xB28 / SYS+0xB44, words): max, min, max - min and middle, divided by SYS+0x1C0; +0x10 glitch count per bank; +0x18 hysteresis limit. Result block O (SYS+0x8E8 / SYS+0x948, 12 x s64): 0 max and 1 min voltage, 2 mean and 3 RMS (x 100 then rescaled), 4 the larger of |max| and |min| with its sign, 5 peak-to-peak, 6 frequency = 1e15 / period, 7 period, 8 and 9 the two pulse widths, 10 duty cycle and 11 its complement (x 10000). Voltage = (raw - offset word SYS+0x60 / SYS+0x120) * scale s64 SYS+0x50 / SYS+0x110 / u16 SYS+0x20. Crossings of the trigger level s16 SYS+0x27E (the middle level when SYS+0x27C is another channel or the level is outside min..max) give the period in samples x 1e6, converted with the time base s64 SYS+0x590 and u16 SYS+0x1C. O is added to the sums at SYS+0xA68 / SYS+0xAC8; after u16 SYS+0x86E / SYS+0x870 acquisitions (counter u16 SYS+0x86A / SYS+0x86C) the averages go to SYS+0x9A8 / SYS+0xA08, the sums are cleared and the byte SYS+0x872 / SYS+0x873 is set.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "libc.h"
#include "libm.h"
#include "processing.h"
#include "sys_info.h"
#include "shared/access.h"

#define SYS 0x40544E68u
#define S16(a) (*(volatile short *)(a))
#define Q64(a) (*(volatile long long *)(a))
#define LDIV(a, b) __aeabi_ldivmod((long long)(a), (long long)(b))
#define MUL64(a, b) ((long long)((unsigned long long)(a) * (unsigned long long)(b)))

/* Scale v down by 10 until it is at most 1e6, moving the factor into the divisor k (while k > 1)
   or the multiplier m (once k is 1). The first step always divides k (as in the stock code). */
static long long dec_first(long long v, long long *k, long long *m)
{
    v = LDIV(v, 10);
    *k = LDIV(*k, 10);
    while (v > 1000000) {
        v = LDIV(v, 10);
        if (*k == 1)
            *m = MUL64(*m, 10);
        else
            *k = LDIV(*k, 10);
    }
    return v;
}

static long long dec_more(long long v, long long *k, long long *m)
{
    do {
        v = LDIV(v, 10);
        if (*k == 1)
            *m = MUL64(*m, 10);
        else
            *k = LDIV(*k, 10);
    } while (v > 1000000);
    return v;
}

/* (v * timebase) / SYS+0x1C / banks / k * m, with both v and the time base first scaled down. */
static long long to_time(long long *v, long long *k, long long *m, int banks)
{
    long long tb, r;
    if (*v > 1000000)
        *v = dec_first(*v, k, m);
    tb = Q64(SYS + 0x590);
    if (tb > 1000000)
        tb = dec_more(tb, k, m);
    r = LDIV(MUL64(*v, tb), (long long)SI(f_01c));
    r = LDIV(r, (long long)banks);
    r = LDIV(r, *k);
    return MUL64(r, *m);
}

void measure_channel(u32 ch_arg)
{
    static const unsigned char order[12] = {0, 1, 2, 3, 5, 4, 6, 7, 8, 9, 10, 11};
    unsigned char ch = (unsigned char)ch_arg;   /* the stock code reads the byte back from the stack */
    u32 th = (u32)(int)S16(SYS + 0x27E);
    u32 buf = W32(SYS + 0x7B4 + 4 * ch), n = (unsigned short)SI(capture_samples);
    u32 cntp, flagp, avg, out, acc, mb, limit, mx, mn, d, maxs, mins, k;
    volatile u32 *mr;
    volatile long long *o, *a, *v;
    long long off, scale, vmax, vmin, amax, amin, per = 0, wb = 0, wa = 0, rms = 0, mean = 0;
    long long k1 = 1000000, m1 = 1, k2 = 1000000, m2 = 1, k3 = 1000000, m3 = 1, t, w, div;
    int banks, seg, b, glitch = 0, bad = 0, i;

    if (ch == 0) {
        if (SI(ch1_enabled) == 0)
            return;
        off = (long long)(int)SI(ch1_offset);
        scale = Q64(SYS + 0x50);
        flagp = SYS + 0x872;
        cntp = SYS + 0x86A;
        limit = SI(ch1_avg_limit);
        avg = SYS + 0x9A8;
        out = SYS + 0x8E8;
        acc = SYS + 0xA68;
        mb = SYS + 0xB28;
    } else {
        if (ch == 1 && SI(ch2_enabled) == 0)
            return;
        off = (long long)(int)SI(ch2_offset);
        scale = Q64(SYS + 0x110);
        flagp = SYS + 0x873;
        cntp = SYS + 0x86C;
        limit = SI(ch2_avg_limit);
        avg = SYS + 0xA08;
        out = SYS + 0x948;
        acc = SYS + 0xAC8;
        mb = SYS + 0xB44;
    }
    banks = (int)SI(bank_count);
    mr = (volatile u32 *)mb;
    o = (volatile long long *)out;
    a = (volatile long long *)acc;
    v = (volatile long long *)avg;

    /* raw min / max over the samples 10 .. n - 11 */
    mx = 0;
    mn = 0xFFFFFFFFu;
    if ((int)(n - 10) > 10) {
        volatile unsigned short *p = (volatile unsigned short *)(buf + 0x14);
        for (k = n - 20; k; k--, p++) {
            u32 s = *p;
            if (s >= mx)
                mx = s;
            if (s <= mn)
                mn = s;
        }
    }
    mr[1] = mn;
    mr[3] = (mn + mx) >> 1;
    mr[2] = mx - mn;
    mr[0] = mx;
    d = SI(acq_geometry_1c0);
    maxs = mx / d;
    mr[0] = maxs;
    {
        u32 lo = mr[1], mid = mr[3];
        mins = lo / SI(acq_geometry_1c0);
        mr[1] = mins;
        mr[2] = (mx - mn) / SI(acq_geometry_1c0);
        mr[3] = mid / SI(acq_geometry_1c0);
    }
    vmax = LDIV(MUL64((long long)maxs - off, scale), (long long)SI(grid_step));
    o[0] = vmax;
    vmin = LDIV(MUL64((long long)mins - off, scale), (long long)SI(grid_step));
    o[1] = vmin;
    o[5] = vmax - vmin;
    if (SI(trigger_channel) != ch)
        th = mr[3];
    if (maxs < th || th < mins)
        th = mr[3];
    amax = vmax < 0 ? -vmax : vmax;
    amin = vmin < 0 ? -vmin : vmin;
    o[4] = amax < amin ? vmin : vmax;

    seg = (unsigned short)((int)(n - 20) / banks);
    for (b = 0; b < banks; b++) {
        u32 base = (u32)b + 10, shift = SI(sample_shift);
        int hia = 0, loa = 0xFFFF, hib = 0, lob = 0xFFFF, fa = 0, fb = 0;
        int na = 0, nb = 0, firsta = 0, lasta = 0, firstb = 0, lastb = 0;
        int suma = 0, sumb = 0, cnta = 0, cntb = 0, usea = 0, from, to, cnt, sq, sm;
        int prev = (int)(H16(buf + 0x14 + 2 * (u32)b) >> shift);
        volatile unsigned short *p;
        if (seg <= 1)
            goto invalid;
        for (i = 1; i < seg; i++) {
            u32 s;
            if (prev >= hia)
                hia = prev;
            s = H16(buf + 2 * ((u32)banks * (u32)i + base)) >> shift;
            if (prev <= loa)
                loa = prev;
            if (prev >= hib)
                hib = prev;
            if (prev <= lob)
                lob = prev;
            if (prev < (int)th) {
                if ((int)th <= (int)(unsigned short)s) {
                    if (fa == 0)
                        firsta = i;
                    na++;
                    fa = 1;
                    lasta = i;
                    if (fb) {
                        cnta++;
                        suma = i + suma - lastb;
                    }
                    if (mr[6] < (u32)(hia - loa + 1)) {
                        loa = 0xFFFF;
                        glitch++;
                        hia = 0;
                    }
                    i += 2;
                }
            } else if ((int)th < prev && (int)th >= (int)(unsigned short)s) {
                if (fb == 0)
                    firstb = i;
                nb++;
                fb = 1;
                lastb = i;
                if (fa) {
                    cntb++;
                    sumb = i + sumb - lasta;
                }
                if (mr[6] < (u32)(hib - lob + 1)) {
                    lob = 0xFFFF;
                    glitch++;
                    hib = 0;
                }
                i += 2;
            }
            prev = (unsigned short)s;
        }
        if (na <= 1 && nb <= 1)
            goto invalid;
        if (na >= nb) {
            per += LDIV((long long)(lasta - firsta) * 1000000, (long long)(na - 1));
            wb += LDIV((long long)sumb * 1000000, (long long)cntb);
            wa = wb < per ? per - wb : 0;
            usea = 1;
        } else {
            per += LDIV((long long)(lastb - firstb) * 1000000, (long long)(nb - 1));
            wa += LDIV((long long)suma * 1000000, (long long)cnta);
            wb = wa < per ? per - wa : 0;
        }
        if (bad) {
            to = seg;
            from = 0;
        } else if (usea) {
            from = firsta;
            to = lasta;
        } else {
            to = lastb;
            from = firstb;
        }
        goto rms_part;
invalid:
        bad = 1;
        mr[4] = 0;
        to = seg;
        from = 0;
rms_part:
        sq = 0;
        sm = 0;
        p = (volatile unsigned short *)(buf + 2 * ((u32)banks * (u32)from + base));
        if (to >= from) {
            u32 sh = SI(sample_shift);
            int j = from;
            do {
                int x = (int)(*p >> sh) - (int)(u32)off;
                j++;
                p += banks;
                sq += x * x;
                sm += x;
            } while (to >= j);
        }
        cnt = to - from + 1;
        rms = (long long)((double)rms + sqrt((double)(sq / cnt)) * 100.0);
        mean += (long long)(100 * sm / cnt);
    }
    mr[4] = (u32)(glitch / banks);
    o[3] = LDIV(LDIV(LDIV(MUL64(rms, scale), (long long)SI(grid_step)), 100), (long long)banks);
    o[2] = LDIV(LDIV(LDIV(MUL64(mean, scale), (long long)SI(grid_step)), 100), (long long)banks);

    t = to_time(&per, &k1, &m1, banks);
    o[7] = t;
    w = to_time(&wb, &k2, &m2, banks);
    o[8] = w;
    o[9] = to_time(&wa, &k3, &m3, banks);
    o[6] = t != 0 ? LDIV(1000000000000000LL, t) : 0;
    div = w > 10000000000LL ? 10000 : 1;
    {
        long long duty = LDIV(MUL64(LDIV(w, div), 10000), LDIV(t, div));
        o[11] = 10000 - duty;
        o[10] = duty;
    }
    a[0] += vmax;
    for (i = 1; i < 12; i++)
        a[order[i]] += o[order[i]];
    k = (unsigned short)(H16(cntp) + 1);
    if ((int)limit > (int)k) {
        H16(cntp) = (unsigned short)k;
        return;
    }
    H16(cntp) = (unsigned short)k;
    B8(flagp) = 1;
    for (i = 0; i < 12; i++)
        v[order[i]] = LDIV(a[order[i]], (long long)(int)limit);
    for (i = 0; i < 12; i++)
        a[i] = 0;
    H16(cntp) = 0;
}
