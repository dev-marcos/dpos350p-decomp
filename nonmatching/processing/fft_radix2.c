/*
 * fft_radix2 -- stock 0x400665A8 (592 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * In-place radix-2 decimation-in-time FFT of n complex floats (n a power of two, at least 2; the stock loops never end for n < 2 or other sizes). Bit-reversal reordering, a D-cache clean, then log2(n) stages with the twiddle w = cos(pi / half) - i sin(pi / half) advanced by cmul. After each stage, when byte SYS+0x54E is set and byte 0x42E68668+5 is clear, it calls runtime_event_service and stops (clearing byte 0x42E68668+4) when byte 0x42E68BB0+0x15 became set.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "processing.h"
#include "stock_asm.h"
#include "sys_info.h"
#include "state/levels_block.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))

typedef struct { float re, im; } cfloat;
typedef double (*dfn)(double);
#define COS_T ((dfn)0x40066531u)   /* cos at 0x40066530 is Thumb */
#define SIN_T ((dfn)0x40066535u)   /* sin at 0x40066534 is Thumb */

cfloat cmul(cfloat a, cfloat b);

void fft_radix2(int n, volatile float *x)
{
    int i, j, k, t, stages, s, le, le2, jj, p;
    float r0, i0, r1, i1, tr, ti;
    cfloat u, w;
    double a;
    j = 0;
    for (i = 0; i < n - 1; i++) {
        if (i < j) {
            r0 = x[2 * i];
            i0 = x[2 * i + 1];
            r1 = x[2 * j];
            i1 = x[2 * j + 1];
            x[2 * j] = r0;
            x[2 * j + 1] = i0;
            x[2 * i] = r1;
            x[2 * i + 1] = i1;
        }
        k = n / 2;
        while (j >= k) {
            j -= k;
            k /= 2;
        }
        j += k;
    }
    dcache_clean_inv_all();
    stages = 1;
    t = n;
    for (;;) {
        t /= 2;
        if (t == 1)
            break;
        stages++;
    }
    for (s = 1; s <= stages; s++) {
        le = 2 << (s - 1);
        le2 = le / 2;
        u.re = 1.0f;
        u.im = 0.0f;
        a = 3.141592653589793 / (double)le2;
        w.re = (float)COS_T(a);
        w.im = (float)-SIN_T(a);
        for (jj = 0; jj <= le2 - 1; jj++) {
            for (i = jj; i <= n - 1; i += le) {
                p = i + le2;
                r1 = x[2 * p];
                i1 = x[2 * p + 1];
                tr = r1 * u.re - i1 * u.im;
                ti = r1 * u.im + i1 * u.re;
                x[2 * p] = x[2 * i] - tr;
                x[2 * p + 1] = x[2 * i + 1] - ti;
                x[2 * i] = x[2 * i] + tr;
                x[2 * i + 1] = x[2 * i + 1] + ti;
            }
            u = cmul(u, w);
        }
        if (SI(f_54e) && !LEVELS(f_005)) {
            runtime_event_service();
            if (B8(0x42E68BB0u + 0x15)) {
                LEVELS(f_004) = 0;
                return;
            }
        }
    }
}
