/*
 * calibration_process_serialized_fields -- stock 0x40044450 (560 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * The field list of the calibration record: cursor 0, then 50 rows of the 32 word columns of the table 0x42E53A0C (+0x190 + 0xC8 x column + 4 x row), then 20 rows of the four word columns +0x50 / +0xF0 / +0xA0 / +0x140. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"

#define SYS 0x40544E68u
#define BLK 0x42E68668u    /* level (spectrum) view block */
#define BODE 0x42E180C8u   /* frequency-response view block */
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))

#define FLD(p, s) serializer_process_field((volatile void *)(p), (s))

void calibration_process_serialized_fields(void)
{
    int j;
    serializer_set_cursor(0);
    for (j = 0; j < 50; j++) {
        FLD(0x42E53A0Cu + 0x190 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x258 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x320 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x3E8 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x4B0 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x578 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x640 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x708 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x7D0 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x898 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x960 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0xA28 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0xAF0 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0xBB8 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0xC80 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0xD48 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0xE10 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0xED8 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0xFA0 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x1068 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x1130 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x11F8 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x12C0 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x1388 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x1450 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x1518 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x15E0 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x16A8 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x1770 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x1838 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x1900 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x19C8 + 0x4 * j, 4);
    }
    for (j = 0; j < 20; j++) {
        FLD(0x42E53A0Cu + 0x50 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0xF0 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0xA0 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x140 + 0x4 * j, 4);
    }
}
