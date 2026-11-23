/*
 * lane_trim_search8 -- stock 0x4006453C (692 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Generator self-test check (row, bank, step): sets up the acquisition (SYS+0x588 = row, level 250, SYS+0x102 = 0), then five rounds of a trim search over lanes 1..7. Each pass measures the lane errors with 0x40064E0C; the trim byte SYS+0x684 + bank x 8 + SYS8(0x6E3 - k) moves by +step (error >= 0) or -step (error < 0) until both signs were seen, then keeps the trim with the smaller error. A round ends when all 7 lanes are settled or after 61 passes (then the result becomes 0 and the next rounds do one pass each). Returns 1 / 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "drivers/timer.h"
#include "fpga.h"
#include "system.h"
#include "util.h"
#include "sys_info.h"

typedef struct { u32 neg[8], pos[8], dn[8], dp[8], tn[8], tp[8]; } ltrim_t;

static void ltrim_setup(u32 row, u32 pass)
{
    acq_defaults_reset();
    SI(timebase_index) = row;
    SI(ch1_enabled) = 1;
    SI(ch1_vdiv_index) = 7;
    SI(ch1_coupling) = 0;
    SI(ch1_probe) = 0;
    SI(f_046) = 1;
    SI(ch1_offset) = 250;
    SI(ch2_enabled) = pass;
    SI(ch2_vdiv_index) = 7;
    SI(ch2_probe) = 0;
    SI(trigger_level) = 250;
    SI(ch2_offset) = 250;
    SI(ch2_coupling) = 0;
    SI(f_10a) = 1;
    SI(trigger_mode) = 0;
    SI(f_275) = 0;
    SI(f_274) = 0;
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
            lane_phase_measure8(m);
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
