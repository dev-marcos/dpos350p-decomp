/*
 * calibration_result_flags -- stock 0x4004FAB0 (132 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clears the bytes 0x42E68B01..0x42E68B03, runs system_calibration and records its result: 1 sets all three to 1; 0 gives (+2, +3) = (0, 0), -1 gives (0, 1), -2 gives (1, 0); other results keep the values read back.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "system.h"
#include "state/factory_block.h"


void calibration_result_flags(void)
{
    int r;
    u32 a, b;
    FACT(f_021) = 0;
    *(volatile unsigned short *)(0x42E68AE0u + 0x22) = 0;
    r = system_calibration();
    if (r == 1) {
        FACT(f_021) = 1;
        FACT(f_023) = 1;
        FACT(f_022) = 1;
        return;
    }
    a = FACT(f_022);
    b = FACT(f_023);
    if (r == 0) {
        a = 0;
        b = 0;
    } else if (r == -1) {
        a = 0;
        b = 1;
    } else if (r == -2) {
        a = 1;
        b = 0;
    }
    FACT(f_023) = b;
    FACT(f_022) = a;
}
