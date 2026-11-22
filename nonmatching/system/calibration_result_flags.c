/*
 * calibration_result_flags -- stock 0x4004FAB0 (132 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Clears the bytes 0x42E68B01..0x42E68B03, runs system_calibration and records its result: 1 sets all three to 1; 0 gives (+2, +3) = (0, 0), -1 gives (0, 1), -2 gives (1, 0); other results keep the values read back.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "system.h"

#define B8(o) (*(volatile unsigned char *)(0x42E68AE0u + (o)))

void calibration_result_flags(void)
{
    int r;
    u32 a, b;
    B8(0x21) = 0;
    *(volatile unsigned short *)(0x42E68AE0u + 0x22) = 0;
    r = system_calibration();
    if (r == 1) {
        B8(0x21) = 1;
        B8(0x23) = 1;
        B8(0x22) = 1;
        return;
    }
    a = B8(0x22);
    b = B8(0x23);
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
    B8(0x23) = b;
    B8(0x22) = a;
}
