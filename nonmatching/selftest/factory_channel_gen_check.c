/*
 * factory_channel_gen_check -- stock 0x4004FB44 (2396 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Generator-to-channel loop-back check of the tester step. For 9 settings (sine, 0x42E4287E = 0; frequency Hz 0x42E42880, amplitude mVpp +4, offset mV +8; time base SYS+0x588; volts/div SYS+0x41 = SYS+0x105) it runs outputs_apply, gen_update and acquisition_dispatcher twice and checks the measurements / 1000 (CH1: frequency s64 SYS+0x918, amplitude +0x910, mean +0x8F8; CH2: +0x978, +0x970, +0x958) against the set value +- t % (for a negative offset the window is mirrored). Settings 1..4 (10 kHz, 1250 / 500 mVpp, +-1000 / +-400 mV) check all three, 5..7 (50 MHz 2500 / 1000 mVpp at 70 %, 10 Hz 1000 mVpp) frequency and amplitude, 8..9 (+-2500 mV DC) the mean. Flags 0x42E68AE0+0x25 / +0x26 = all CH1 / CH2 checks passed, +0x24 = both. It first runs acq_defaults_reset and sets SYS+0x3E = SYS+0x102 = 1, SYS+0x660 = 5, byte 0x42E68BC6 = 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "generator.h"
#include "selftest.h"
#include "system.h"
#include "sys_info.h"
#include "state/factory_block.h"

#define GEN8(o)  (*(volatile unsigned char *)(0x42E42880u + (o)))
#define GEN32(o) (*(volatile int *)(0x42E42880u + (o)))
#define CG_MEAS(o) ((int)__aeabi_ldivmod((long long)SYS64(o), 1000))

enum { CG_FREQ = 1, CG_AMP = 2, CG_MEAN = 4 };

/* set value v +- t % (t mirrored for a negative v) contains m */
static int cg_in(int m, int v, int t)
{
    int lo, hi;
    if (v < 0)
        t = -t;
    lo = (int)__aeabi_ldivmod((long long)v * (100 - t), 100);
    hi = (int)__aeabi_ldivmod((long long)v * (100 + t), 100);
    return m >= lo && hi >= m;
}

/* one setting; returns bit 0 = CH1 passed, bit 1 = CH2 passed */
static u32 cg_point(int freq, int amp, int offs, u32 tb, u32 vdiv, int t, u32 what)
{
    int ok1 = 1, ok2 = 1;
    GEN8(-2) = 0;
    GEN32(0) = freq;
    GEN32(4) = amp;
    GEN32(8) = offs;
    SI(timebase_index) = tb;
    SI(ch1_vdiv_index) = vdiv;
    SI(ch2_vdiv_index) = vdiv;
    outputs_apply();
    gen_update();
    acquisition_dispatcher();
    acquisition_dispatcher();
    if (what & CG_FREQ) {
        ok1 &= cg_in(CG_MEAS(0x918), freq, t);
        ok2 &= cg_in(CG_MEAS(0x978), freq, t);
    }
    if (what & CG_AMP) {
        ok1 &= cg_in(CG_MEAS(0x910), amp, t);
        ok2 &= cg_in(CG_MEAS(0x970), amp, t);
    }
    if (what & CG_MEAN) {
        ok1 &= cg_in(CG_MEAS(0x8F8), offs, t);
        ok2 &= cg_in(CG_MEAS(0x958), offs, t);
    }
    return ok1 | ok2 << 1;
}

void factory_channel_gen_check(void)
{
    u32 r = 3;
    acq_defaults_reset();
    SI(ch1_enabled) = 1;
    SI(ch2_enabled) = 1;
    SI(f_660) = 5;
    g_touch->enabled = 0;
    r &= cg_point(10000, 1250, 1000, 24, 5, 15, CG_FREQ | CG_AMP | CG_MEAN);
    r &= cg_point(10000, 1250, -1000, 24, 5, 15, CG_FREQ | CG_AMP | CG_MEAN);
    r &= cg_point(10000, 500, 400, 24, 6, 15, CG_FREQ | CG_AMP | CG_MEAN);
    r &= cg_point(10000, 500, -400, 24, 6, 15, CG_FREQ | CG_AMP | CG_MEAN);
    r &= cg_point(50000000, 2500, 0, 13, 5, 70, CG_FREQ | CG_AMP);
    r &= cg_point(50000000, 1000, 0, 13, 6, 70, CG_FREQ | CG_AMP);
    r &= cg_point(10, 1000, 0, 32, 6, 15, CG_FREQ | CG_AMP);
    r &= cg_point(0, 0, 2500, 24, 4, 15, CG_MEAN);
    r &= cg_point(0, 0, -2500, 24, 4, 15, CG_MEAN);
    FACT(ch1_checks_ok) = r & 1;
    FACT(ch2_checks_ok) = r >> 1;
    FACT(f_024) = r == 3;
}
