/*
 * lane_trim_search4x2 -- stock 0x400647F4 (1076 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Second generator self-test check (row, bank, step): like lane_trim_search8 with SYS+0x102 = 1 and two groups of lanes 1..3 measured together by 0x4006529C (first group trims SYS+0x684 + bank x 8 + SYS8(0x6DF - k), second group SYS8(0x6E3 - k)). A round ends when both groups are settled or after 61 passes. Returns 1 / 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void acq_defaults_reset(void);
void fill_u32(u32 *d, u32 v, u32 n);

typedef struct { u32 neg[8], pos[8], dn[8], dp[8], tn[8], tp[8]; } ltrim_t;

static void ltrim_setup(u32 row, u32 pass)
{
    acq_defaults_reset();
    SYS8(0x588) = row;
    SYS8(0x3E) = 1;
    SYS8(0x41) = 7;
    SYS8(0x3F) = 0;
    SYS8(0x40) = 0;
    SYS8(0x46) = 1;
    SYS32(0x60) = 250;
    SYS8(0x102) = pass;
    SYS8(0x105) = 7;
    SYS8(0x104) = 0;
    SYS16(0x27E) = 250;
    SYS32(0x120) = 250;
    SYS8(0x103) = 0;
    SYS8(0x10A) = 1;
    SYS8(0x27A) = 0;
    SYS8(0x275) = 0;
    SYS8(0x274) = 0;
    outputs_apply();
    delay_ms(5);
}

static void ltrim_clear(ltrim_t *s, u32 n)
{
    fill_u32(s->neg, 0, n);
    fill_u32(s->pos, 0, n);
    fill_u32(s->dn, 0, n);
    fill_u32(s->dp, 0, n);
    fill_u32(s->tn, 0, n);
    fill_u32(s->tp, 0, n);
}

/* one search step of lane k: trim byte p[t], measured error m */
static void ltrim_lane(ltrim_t *s, int k, int m, volatile unsigned char *p, u32 t, u32 step)
{
    u32 v;
    if (s->pos[k] && s->neg[k]) {
        v = (int)s->dn[k] < (int)s->dp[k] ? s->tn[k] : s->tp[k];
    } else if (m < 0) {
        s->neg[k] = 1;
        s->dn[k] = -m;
        s->tn[k] = p[t];
        v = p[t] - step;
    } else {
        s->pos[k] = 1;
        s->dp[k] = m;
        s->tp[k] = p[t];
        v = p[t] + step;
    }
    p[t] = (unsigned char)v;
}

static u32 ltrim_mask(ltrim_t *s, int n)
{
    u32 mask = 0;
    int k;
    for (k = 1; k < n; k++)
        if (s->pos[k] && s->neg[k])
            mask |= 1u << k;
    return mask;
}

void func_4006529c(int *m1, int *m2);

int lane_trim_search4x2(u32 row, u32 bank, u32 step)
{
    ltrim_t s1, s2;
    int m1[4], m2[4];
    volatile unsigned char *p = (volatile unsigned char *)(0x40544E68u + bank * 8 + 0x684);
    unsigned char ok = 1, st = (unsigned char)step;
    int round, it, k;
    ltrim_setup(row, 1);
    for (round = 5; round; round--) {
        ltrim_clear(&s1, 4);
        ltrim_clear(&s2, 4);
        it = 0;
        for (;;) {
            func_40063c34_all8();
            func_4006529c(m1, m2);
            for (k = 1; k < 4; k++)
                ltrim_lane(&s1, k, m1[k], p, SYS8(0x6DF - k), st);
            for (k = 1; k < 4; k++)
                ltrim_lane(&s2, k, m2[k], p, SYS8(0x6E3 - k), st);
            if (ltrim_mask(&s1, 4) == 0xE && ltrim_mask(&s2, 4) == 0xE)
                break;
            if (++it >= 61) {
                ok = 0;
                break;
            }
            if (!ok)
                break;
        }
    }
    func_40063c34_all8();
    return ok;
}
