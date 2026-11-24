/*
 * bode_calibrate -- stock 0x40039EB8 (380 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Calibration sweep: trace_defaults_37b48 (which fills the frequencies of record D 0x42E554B0), byte 0x42E180C8+0xAD18 = 1 (no event service during the points), +0x47 = 1, +0x4C = 1000, +0x50 = 0 (also into the generator words 0x42E3F984+0x2F00/04), touch bytes +0x16 / +0x15 = 0; resets the calibration (ratio +0x4C0[i] = 1.0, phase +0xE20[i] = 0 for i = 0 .. word D+0xC), input ranges SYS+0x41 / +0x105 = 11, then measures every frequency D+0x10[i] (ratio into 0x42E180C8+0x68[i], phase into +0x7D68[i]) and copies them into the calibration (ratio, phase as a double). The record is saved (settings_d_save) only when both inputs reached 5000. Ends with touch byte +0x16 = 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "bode.h"
#include "libm.h"
#include "processing.h"
#include "storage/settings.h"
#include "sys_info.h"
#include "state/bode_block.h"
#include "state/gen_block.h"
#include "shared/access.h"

#define SYS 0x40544E68u
#define DREC 0x42E554B0u
#define W(a) (*(volatile u32 *)(a))
#define DD(a) (*(volatile double *)(a))
#define U16(v) ((unsigned short)(v))

static const union { unsigned long long u; double d; } LIM = { 0x40B3880000000001ULL };   /* 5000.000000000001 */

static double point_ratio(void)
{
    double a = BODES(amp_a), b = BODES(amp_b), r;
    r = b / a;
    if (a >= LIM.d)
        BODES(a_reached) = 1;
    if (b >= LIM.d)
        BODES(b_reached) = 1;
    return r;
}

void bode_calibrate(void)
{
    u32 i;
    trace_defaults_37b48();
    BODES(sweep_busy) = 1;
    BODES(sweep_running) = 1;
    BODES(amplitude) = 1000;
    BODES(offset) = 0;
    GENS(amplitude) = 1000;
    GENS(offset) = 0;
    g_touch->enabled = 0;
    g_touch->busy2 = 0;
    for (i = 0;; ) {
        DD(DREC + 0x4C0 + 8 * i) = 1.0;
        W(DREC + 0xE20 + 8 * i) = 0;
        W(DREC + 0xE24 + 8 * i) = 0;
        i++;
        if (W(DREC + 0xC) < i)
            break;
    }
    BODES(a_reached) = 0;
    BODES(b_reached) = 0;
    SI(ch1_vdiv_index) = 11;
    SI(ch2_vdiv_index) = 11;
    for (i = 0;; ) {
        bode_point_measure(W(DREC + 0x10 + 4 * i));
        BODES(gain_ratio[i]) = point_ratio();
        bode_phase_measure(i & 0xFFFF);
        i++;
        if (W(DREC + 0xC) < i)
            break;
    }
    for (i = 0;; ) {
        DD(DREC + 0x4C0 + 8 * i) = BODES(gain_ratio[i]);
        DD(DREC + 0xE20 + 8 * i) = (double)(short)BODES(phase_deg[i]);
        i++;
        if (W(DREC + 0xC) < i)
            break;
    }
    if (BODES(a_reached) && BODES(b_reached))
        settings_d_save();
    g_touch->enabled = 1;
}
