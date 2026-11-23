/*
 * gen_selftest_504b8 -- stock 0x400504B8 (464 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Self-test (DERIVED from the settings it writes): both channels on (SYS+0x3E, +0x102, +0x3F, +0x103 = 1), SYS+0x660 = 5, byte 0x42E68BC6 = 0; one generator setting (byte +0x2EFA = 0xD, words +0x2EFC, +0x2F00 = 0, +0x2F04 = 1000 of block 0x42E3F984; SYS+0x588 = 0x18, SYS+0x41 = SYS+0x105 = 6, SYS+0x60 = SYS+0x120 = 0), applied, 500 ms, two acquisitions. Pass flags: s64 SYS+0x8F8 / 1000 and SYS+0x958 / 1000 within -399..399, stored in bytes 0x42E68B08 / 0x42E68B09 and their AND in 0x42E68B07.
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

void gen_selftest_504b8(void)
{
    u32 a, b;
    acq_defaults_reset();
    SI(ch1_enabled) = 1;
    SI(ch2_enabled) = 1;
    SI(ch1_coupling) = 1;
    SI(ch2_coupling) = 1;
    SI(f_660) = 5;
    *(volatile unsigned char *)(0x42E68BB0u + 0x16) = 0;
    st_step(1000, 0x18, 6, 0, 1);
    {
        int q1 = (int)(SI(f_958) / 1000);
        int q2 = (int)(SI(f_8f8) / 1000);
        a = (u32)(q2 + 0x18F) < 0x31Fu;
        b = (u32)(q1 + 0x18F) < 0x31Fu;
    }
    FACT(f_028) = a;
    FACT(f_029) = b;
    FACT(f_027) = b & a;
}
