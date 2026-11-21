# Two-point gain/offset calibration of the 8 ADC lanes for every calibrated row (V/div range), in two
# passes (SYS+0x102 = 0 and 1). Called by system_calibration and by the generator self-test flow.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wc(addr, size, name, desc, body, src="calibration"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


wc(0x4004B0A0, 3864, "lane_gain_calibrate",
   "Two-point calibration of the 8 ADC lanes. Resets the acquisition defaults, sets the level SYS+0x60/+0x120 to 450 and "
   "then 50, and for each row r = SYS+0x6EC .. SYS+0x6ED (written to SYS+0x588) measures the mean of each lane "
   "(lane_level_mean over 2000 samples). With K = level x SYS+0x1C0 x SYS+0x26C and S = SYS+0x268, the gain of lane k is "
   "(K450 - K50) x S / (H - L) and the offset (K450 x S - gain x H) / SYS+0x26C (64-bit). Pass 1 (SYS+0x102 = 0) writes "
   "CAL_OBJ+0x190 (gains, 8 x 50 ints) and +0x7D0 (offsets); pass 2 (SYS+0x102 = 1, lanes 4..7 measured with ch = 1) "
   "writes CAL_OBJ+0xE10 and +0x1450.", r"""
void acq_defaults_reset(void);
u32 lane_level_mean(u32 start, u32 lane, u32 ch, u32 n, unsigned char *buf);

#define LCAL_BUF ((unsigned char *)0x429D3AB4u)
#define LCAL_ROWS 50

static void lcal_measure(int n, u32 ch_hi, int v[8][LCAL_ROWS])
{
    int i, k;
    for (i = 0; i < n; i++) {
        SYS8(0x588) = (unsigned char)(i + SYS8(0x6EC));
        outputs_apply();
        delay_ms(20);
        v[0][i] = (int)lane_level_mean(1, 0, 0, 2000, LCAL_BUF);
        for (k = 1; k < 8; k++)
            v[k][i] = (int)lane_level_mean(0, k, k >= 4 ? ch_hi : 0, 2000, LCAL_BUF);
    }
}

static u32 lcal_level(u32 pass, u32 level)
{
    SYS8(0x3E) = 1;
    SYS8(0x102) = pass;
    SYS32(0x60) = level;
    SYS32(0x120) = level;
    outputs_apply();
    delay_ms(100);
    return SYS32(0x60) * SYS32(0x1C0) * SYS32(0x26C);
}

static void lcal_fit(int n, u32 hi, u32 lo, int h[8][LCAL_ROWS], int l[8][LCAL_ROWS], volatile int *t)
{
    u32 s = SYS32(0x268);
    int j, k;
    for (j = 0; j < n; j++) {
        long long d = (long long)(int)(hi - lo) * (long long)s;
        long long e;
        u32 y;
        for (k = 0; k < 8; k++)
            t[k * LCAL_ROWS + j] = (int)__aeabi_ldivmod(d, (long long)(h[k][j] - l[k][j]));
        e = (long long)(int)hi * (long long)s;
        y = SYS32(0x26C);
        for (k = 0; k < 8; k++)
            t[8 * LCAL_ROWS + k * LCAL_ROWS + j] =
                (int)__aeabi_ldivmod(e - (long long)t[k * LCAL_ROWS + j] * h[k][j], (long long)y);
    }
}

void lane_gain_calibrate(void)
{
    int h[8][LCAL_ROWS], l[8][LCAL_ROWS];
    u32 hi, lo;
    int n;
    acq_defaults_reset();
    SYS8(0x3C) = 0;
    SYS8(0x41) = 6;
    SYS8(0x105) = 6;
    SYS8(0x40) = 0;
    SYS8(0x3F) = 0;
    SYS8(0x46) = 0;
    SYS8(0x104) = 0;
    SYS8(0x103) = 0;
    SYS8(0x10A) = 0;
    SYS8(0x275) = 0;
    SYS8(0x660) = 0;
    g_touch->enabled = 0;
    SYS8(0x102) = 0;
    SYS8(0x3E) = 1;
    SYS32(0x60) = 450;
    SYS32(0x120) = 450;
    outputs_apply();
    acquisition_dispatcher();
    delay_ms(100);
    hi = SYS32(0x60) * SYS32(0x1C0) * SYS32(0x26C);
    n = (int)SYS8(0x6ED) - (int)SYS8(0x6EC) + 1;
    lcal_measure(n, 0, h);
    lo = lcal_level(0, 50);
    if (n > 0) {
        lcal_measure(n, 0, l);
        lcal_fit(n, hi, lo, h, l, (volatile int *)((u32)CAL_OBJ + 0x190));
    }
    hi = lcal_level(1, 450);
    lcal_measure(n, 1, h);
    lo = lcal_level(1, 50);
    if (n > 0) {
        lcal_measure(n, 1, l);
        lcal_fit(n, hi, lo, h, l, (volatile int *)((u32)CAL_OBJ + 0xE10));
    }
}
""")


# ---- generator self-test: per-lane trim search (0x4006453C 8 lanes, 0x400647F4 two groups of 4)
LTRIM = r"""
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
"""

wc(0x4006453C, 692, "lane_trim_search8",
   "Generator self-test check (row, bank, step): sets up the acquisition (SYS+0x588 = row, level 250, SYS+0x102 = 0), then "
   "five rounds of a trim search over lanes 1..7. Each pass measures the lane errors with 0x40064E0C; the trim byte "
   "SYS+0x684 + bank x 8 + SYS8(0x6E3 - k) moves by +step (error >= 0) or -step (error < 0) until both signs were seen, "
   "then keeps the trim with the smaller error. A round ends when all 7 lanes are settled or after 61 passes (then the "
   "result becomes 0 and the next rounds do one pass each). Returns 1 / 0.", LTRIM + r"""
void func_40064e0c(int *m);

int lane_trim_search8(u32 row, u32 bank, u32 step)
{
    ltrim_t s;
    int m[8];
    volatile unsigned char *p = (volatile unsigned char *)(0x40544E68u + bank * 8 + 0x684);
    unsigned char ok = 1, st = (unsigned char)step;
    int round, it, k;
    ltrim_setup(row, 0);
    for (round = 5; round; round--) {
        ltrim_clear(&s, 8);
        it = 0;
        for (;;) {
            func_40063c34_all8();
            func_40064e0c(m);
            for (k = 1; k < 8; k++)
                ltrim_lane(&s, k, m[k], p, SYS8(0x6E3 - k), st);
            if (ltrim_mask(&s, 8) == 0xFE)
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
""")

wc(0x400647F4, 1076, "lane_trim_search4x2",
   "Second generator self-test check (row, bank, step): like lane_trim_search8 with SYS+0x102 = 1 and two groups of lanes "
   "1..3 measured together by 0x4006529C (first group trims SYS+0x684 + bank x 8 + SYS8(0x6DF - k), second group "
   "SYS8(0x6E3 - k)). A round ends when both groups are settled or after 61 passes. Returns 1 / 0.", LTRIM + r"""
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
""")
