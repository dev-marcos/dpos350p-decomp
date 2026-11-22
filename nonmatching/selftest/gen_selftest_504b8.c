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

#define GB8(o) (*(volatile unsigned char *)(0x42E3F984u + (o)))
#define GB32(o) (*(volatile u32 *)(0x42E3F984u + (o)))
#define RS8(o) (*(volatile unsigned char *)(0x42E68AE0u + (o)))
#define S64(o) (*(volatile long long *)(0x40544E68u + (o)))

/* one generator setting: amplitude word a, range byte b, level c, offset d; measure twice */
static void st_step(u32 a, u32 b, u32 c, u32 d, int wait)
{
    GB8(0x2EFA) = 0xD;
    GB32(0x2EFC) = 0;
    GB32(0x2F00) = 0;
    GB32(0x2F04) = a;
    SYS8(0x588) = b;
    SYS8(0x41) = c;
    SYS8(0x105) = c;
    SYS32(0x60) = d;
    SYS32(0x120) = d;
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
    SYS8(0x3E) = 1;
    SYS8(0x102) = 1;
    SYS8(0x3F) = 1;
    SYS8(0x103) = 1;
    SYS8(0x660) = 5;
    *(volatile unsigned char *)(0x42E68BB0u + 0x16) = 0;
    st_step(1000, 0x18, 6, 0, 1);
    {
        int q1 = (int)(S64(0x958) / 1000);
        int q2 = (int)(S64(0x8F8) / 1000);
        a = (u32)(q2 + 0x18F) < 0x31Fu;
        b = (u32)(q1 + 0x18F) < 0x31Fu;
    }
    RS8(0x28) = a;
    RS8(0x29) = b;
    RS8(0x27) = b & a;
}
