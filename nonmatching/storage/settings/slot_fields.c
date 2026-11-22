/*
 * slot_fields -- stock 0x40043F54 (1032 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * The field list of a user slot (settings_slot_save / _load): cursor 0, then 121 fields: the acquisition and display state of SYS, the generator words 0x42E3F984+0x2EFA.., the level-view settings (BLK +1..+3, +0x58.., +0xF4, the 8 s64 at +0x120, +0x188 / +0x18C), the Bode sweep settings (BODE +0x44 / +0x46 / +0x4C.. / +0x60) and 0x42E22D28 .. +8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SYS 0x40544E68u
#define BLK 0x42E68668u    /* level (spectrum) view block */
#define BODE 0x42E180C8u   /* frequency-response view block */
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))

void serializer_set_cursor(u32 v);
void serializer_process_field(volatile void *p, u32 size);

#define FLD(p, s) serializer_process_field((volatile void *)(p), (s))

void slot_fields(void)
{
    int j;
    serializer_set_cursor(0);
    FLD(SYS + 0x664, 1);
    FLD(SYS + 0x671, 1);
    FLD(SYS + 0x588, 1);
    FLD(SYS + 0x598, 1);
    FLD(SYS + 0x2AA, 1);
    FLD(SYS + 0x570, 8);
    FLD(SYS + 0x578, 8);
    for (j = 0; j < 5; j++) {
        FLD(SYS + 0x3E + 0x1 * j, 1);
    }
    FLD(SYS + 0x60, 4);
    FLD(SYS + 0x46, 1);
    for (j = 0; j < 5; j++) {
        FLD(SYS + 0x102 + 0x1 * j, 1);
    }
    FLD(SYS + 0x120, 4);
    FLD(SYS + 0x10A, 1);
    FLD(SYS + 0x27B, 1);
    FLD(SYS + 0x27C, 1);
    FLD(SYS + 0x27E, 2);
    FLD(SYS + 0x284, 1);
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0x654 - 0x1 * j, 1);
    }
    FLD(SYS + 0x660, 1);
    FLD(SYS + 0x655, 1);
    FLD(SYS + 0x802, 1);
    FLD(SYS + 0x803, 1);
    for (j = 0; j < 4; j++) {
        FLD(SYS + 0x808 + 0x4 * j, 4);
    }
    FLD(SYS + 0x804, 1);
    for (j = 0; j < 50; j++) {
        FLD(SYS + 0x880 + 0x2 * j, 2);
    }
    FLD(0x42E3F984u + 0x2EFA, 1);
    for (j = 0; j < 4; j++) {
        FLD(0x42E3F984u + 0x2EFC + 0x4 * j, 4);
    }
    for (j = 0; j < 3; j++) {
        FLD(BLK + 0x1 + 0x1 * j, 1);
    }
    for (j = 0; j < 3; j++) {
        FLD(BLK + 0x58 + 0x4 * j, 4);
    }
    FLD(BLK + 0xF4, 4);
    for (j = 0; j < 8; j++) {
        FLD(BLK + 0x120 + 0x8 * j, 8);
    }
    FLD(BLK + 0x188, 4);
    FLD(BLK + 0x18C, 4);
    FLD(BODE + 0x44, 1);
    FLD(BODE + 0x46, 1);
    for (j = 0; j < 5; j++) {
        FLD(BODE + 0x4C + 0x4 * j, 4);
    }
    FLD(BODE + 0x60, 8);
    FLD(0x42E22D28u + 0x0, 1);
    FLD(0x42E22D28u + 0x4, 4);
    FLD(0x42E22D28u + 0x8, 4);
}
