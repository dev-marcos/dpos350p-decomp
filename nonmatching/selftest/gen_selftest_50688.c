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

void gen_selftest_50688(void)
{
    u32 a = 1, b = 1, i, off;
    acq_defaults_reset();
    off = SYS16(0xA);
    SYS8(0x3E) = 1;
    SYS8(0x102) = 1;
    SYS8(0x660) = 5;
    *(volatile unsigned char *)(0x42E68BB0u + 0x16) = 0;
    for (i = 0; i < 2; i++) {
        int amp = i ? -2500 : 2500;
        int q1, q2;
        st_step((u32)amp, 0x18, 6, i ? off : 0, 0);
        q1 = (int)(S64(0x8F8) / 1000);
        q2 = (int)(S64(0x958) / 1000);
        if (amp > 0) {
            a &= q1 >= 1500;
            b &= q2 >= 1500;
        } else {
            a &= q1 < -1499;
            b &= q2 < -1499;
        }
    }
    RS8(0x2F) = a;
    RS8(0x30) = b;
    RS8(0x2E) = b & a;
}
