/*
 * value_si_scale -- stock 0x400301E0 (2328 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Scales a measurement for display. kind is the measurement number (1..12, see measure_item_draw); v is in the base unit (uV for 1..6, mHz for 7, ps for 8..10, 1/10000 for 11..12). Kinds 1..6 truncate and kinds 7..10 round half up (rounding is skipped when v is negative); both pick, from |v|, the divisor, the number of decimals *dec and the prefix index *prefix (0..4 = u / m / unit / k for volts, mHz / Hz / kHz / MHz / GHz, ps / ns / us / ms / s). From |v| >= 1e14 nothing is changed. Other kinds set *dec to 2 (0 when v is 0) and leave *prefix. Finally up to four trailing zero digits are dropped while *dec > 0. *out gets the scaled value.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"

static long long q(long long v, long long d)
{
    return v / d;
}

/* rounded: add d when the remainder is at least the stock threshold (half + 1, 6 for d = 10) */
static long long qr(long long v, long long d, long long half)
{
    if (v % d >= half)
        v += d;
    return v / d;
}

void value_si_scale(u32 kind, u32 unused, long long v, long long *out, int *dec, int *prefix)
{
    long long a = v < 0 ? -v : v;
    int d, i;
    (void)unused;
    if (kind >= 1 && kind <= 6) {
        if (a < 1000) {
            *dec = 0; *prefix = 0;
        } else if (a < 10000) {
            v = q(v, 10); *dec = 2; *prefix = 1;
        } else if (a < 100000) {
            v = q(v, 100); *dec = 1; *prefix = 1;
        } else if (a < 1000000) {
            v = q(v, 1000); *dec = 0; *prefix = 1;
        } else if (a < 10000000) {
            v = q(v, 10000); *dec = 2; *prefix = 2;
        } else if (a < 100000000) {
            v = q(v, 100000); *dec = 1; *prefix = 2;
        } else if (a < 1000000000) {
            v = q(v, 1000000); *dec = 0; *prefix = 2;
        } else if (a < 10000000000LL) {
            v = q(v, 10000000); *dec = 2; *prefix = 3;
        } else if (a < 100000000000LL) {
            v = q(v, 100000000); *dec = 1; *prefix = 3;
        } else if (a < 1000000000000LL) {
            v = q(v, 1000000000); *dec = 0; *prefix = 3;
        } else if (a < 10000000000000LL) {
            v = q(v, 10000000000LL); *dec = 2; *prefix = 4;
        } else if (a < 100000000000000LL) {
            v = q(v, 100000000000LL); *dec = 1; *prefix = 4;
        }
    } else if (kind >= 7 && kind <= 10) {
        if (a < 1000) {
            *dec = 0; *prefix = 0;
        } else if (a < 10000) {
            *dec = 3; *prefix = 1;
        } else if (a < 100000) {
            v = qr(v, 10, 6); *dec = 2; *prefix = 1;
        } else if (a < 1000000) {
            v = qr(v, 100, 51); *dec = 1; *prefix = 1;
        } else if (a < 10000000) {
            v = qr(v, 1000, 501); *dec = 3; *prefix = 2;
        } else if (a < 100000000) {
            v = qr(v, 10000, 5001); *dec = 2; *prefix = 2;
        } else if (a < 1000000000) {
            v = qr(v, 100000, 50001); *dec = 1; *prefix = 2;
        } else if (a < 10000000000LL) {
            v = qr(v, 1000000, 500001); *dec = 3; *prefix = 3;
        } else if (a < 100000000000LL) {
            v = qr(v, 10000000, 5000001); *dec = 2; *prefix = 3;
        } else if (a < 1000000000000LL) {
            v = qr(v, 100000000, 50000001); *dec = 1; *prefix = 3;
        } else if (a < 10000000000000LL) {
            v = qr(v, 1000000000, 500000001); *dec = 3; *prefix = 4;
        } else if (a < 100000000000000LL) {
            v = qr(v, 10000000000LL, 5000000001LL); *dec = 2; *prefix = 4;
        }
    } else {
        *dec = v != 0 ? 2 : 0;
    }
    d = *dec;
    for (i = 0; i < 4; i++) {
        if (v % 10 == 0 && d > 0) {
            d--;
            v /= 10;
        }
    }
    *dec = d;
    *out = v;
}
