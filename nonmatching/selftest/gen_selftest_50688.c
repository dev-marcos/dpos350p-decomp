/*
 * gen_selftest_50688 -- stock 0x40050688 (520 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Self-test (DERIVED): both channels on, SYS+0x660 = 5, byte 0x42E68BC6 = 0; two generator settings (as gen_selftest_504b8, no delay): amplitude +2500 with offset 0 and -2500 with offset halfword SYS+0xA. Pass flags per setting: s64 SYS+0x8F8 / 1000 and SYS+0x958 / 1000 at least 1500 (positive) / below -1499 (negative); ANDed over both settings into bytes 0x42E68B0F / 0x42E68B10, and together into 0x42E68B0E.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "drivers/timer.h"
#include "generator.h"
#include "selftest.h"
#include "system.h"
#include "sys_info.h"
#include "state/factory_block.h"
#include "state/gen_block.h"

/* one generator setting: amplitude word a, range byte b, level c, offset d; measure twice */
static void st_step(u32 a, u32 b, u32 c, u32 d, int wait)
{
    GENS(waveform) = 0xD;
    GENS(frequency) = 0;
    GENS(amplitude) = 0;
    GENS(offset) = a;
    SI(timebase_index) = b;
    SI(ch1_vdiv_index) = c;
    SI(ch2_vdiv_index) = c;
    SI(ch1_offset) = d;
    SI(ch2_offset) = d;
    outputs_apply();
    gen_update();
    if (wait)
        delay_ms(500);
    acquisition_dispatcher();
    acquisition_dispatcher();
}

void gen_selftest_50688(void)
{
    u32 a = 1, b = 1, i, off;
    acq_defaults_reset();
    off = SI(plot_height);
    SI(ch1_enabled) = 1;
    SI(ch2_enabled) = 1;
    SI(f_660) = 5;
    g_touch->enabled = 0;
    for (i = 0; i < 2; i++) {
        int amp = i ? -2500 : 2500;
        int q1, q2;
        st_step((u32)amp, 0x18, 6, i ? off : 0, 0);
        q1 = (int)(SI(f_8f8) / 1000);
        q2 = (int)(SI(f_958) / 1000);
        if (amp > 0) {
            a &= q1 >= 1500;
            b &= q2 >= 1500;
        } else {
            a &= q1 < -1499;
            b &= q2 < -1499;
        }
    }
    FACT(f_02f) = a;
    FACT(f_030) = b;
    FACT(f_02e) = b & a;
}
