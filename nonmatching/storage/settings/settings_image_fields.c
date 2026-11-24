/*
 * settings_image_fields -- stock 0x400448B4 (3276 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * The field list of the main settings image (settings_image_encode starts it at cursor 0x32): the fields of SYS from +4 up, then u16 SYS+0x5D0 triples of words +0x2C4 / +0x38C / +0x454, the block from +0x800, and B8 SYS+0x8E4 u16 values from +0x880. Both counts are re-read on every pass, so a decode that loads them takes effect at once.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"
#include "sys_info.h"
#include "shared/access.h"

#define SYS 0x40544E68u

#define FLD(p, s) serializer_process_field((volatile void *)(p), (s))

void settings_image_fields(void)
{
    int j;
    for (j = 0; j < 5; j++) {
        FLD(SYS + 0x4 + 0x2 * j, 2);
    }
    FLD(SYS + 0xC, 2);
    for (j = 0; j < 12; j++) {
        FLD(SYS + 0x10 + 0x2 * j, 2);
    }
    FLD(SYS + 0x28, 1);
    FLD(SYS + 0x2A, 1);
    FLD(SYS + 0x2C, 2);
    FLD(SYS + 0x30, 8);
    FLD(SYS + 0x39, 1);
    for (j = 0; j < 7; j++) {
        FLD(SYS + 0x3C + 0x1 * j, 1);
    }
    FLD(SYS + 0x44, 2);
    FLD(SYS + 0x46, 1);
    FLD(SYS + 0x50, 8);
    FLD(SYS + 0x58, 8);
    FLD(SYS + 0x60, 4);
    FLD(SYS + 0x64, 4);
    FLD(SYS + 0xD8, 8);
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0xE0 + 0x4 * j, 4);
    }
    for (j = 0; j < 6; j++) {
        FLD(SYS + 0xEC + 0x2 * j, 2);
    }
    FLD(SYS + 0x100, 1);
    for (j = 0; j < 5; j++) {
        FLD(SYS + 0x102 + 0x1 * j, 1);
    }
    FLD(SYS + 0x108, 2);
    FLD(SYS + 0x10A, 1);
    FLD(SYS + 0x110, 8);
    FLD(SYS + 0x118, 8);
    FLD(SYS + 0x120, 4);
    FLD(SYS + 0x124, 4);
    FLD(SYS + 0x198, 8);
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0x1A0 + 0x4 * j, 4);
    }
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0x1B0 + 0x1 * j, 1);
    }
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0x1B8 + 0x1 * j, 1);
    }
    FLD(SYS + 0x1BC, 4);
    FLD(SYS + 0x1C0, 4);
    for (j = 0; j < 4; j++) {
        FLD(SYS + 0x264 + 0x4 * j, 4);
    }
    FLD(SYS + 0x274, 1);
    FLD(SYS + 0x275, 1);
    FLD(SYS + 0x276, 2);
    for (j = 0; j < 5; j++) {
        FLD(SYS + 0x278 + 0x1 * j, 1);
    }
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0x27E + 0x2 * j, 2);
    }
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0x288 + 0x4 * j, 4);
    }
    FLD(SYS + 0x29C, 2);
    FLD(SYS + 0x2A0, 4);
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0x2A4 + 0x1 * j, 1);
    }
    FLD(SYS + 0x2A9, 1);
    FLD(SYS + 0x2AA, 1);
    FLD(SYS + 0x2AC, 2);
    for (j = 0; j < 5; j++) {
        FLD(SYS + 0x2B0 + 0x4 * j, 4);
    }
    FLD(SYS + 0x520, 4);
    FLD(SYS + 0x524, 4);
    FLD(SYS + 0x530, 8);
    FLD(SYS + 0x538, 8);
    FLD(SYS + 0x528, 8);
    FLD(SYS + 0x540, 4);
    for (j = 0; j < 5; j++) {
        FLD(SYS + 0x544 + 0x2 * j, 2);
    }
    for (j = 0; j < 7; j++) {
        FLD(SYS + 0x554 + 0x4 * j, 4);
    }
    FLD(SYS + 0x570, 8);
    FLD(SYS + 0x578, 8);
    FLD(SYS + 0x581, 1);
    FLD(SYS + 0x588, 1);
    FLD(SYS + 0x590, 8);
    FLD(SYS + 0x598, 1);
    FLD(SYS + 0x5A0, 8);
    FLD(SYS + 0x550, 2);
    FLD(SYS + 0x54F, 1);
    FLD(SYS + 0x5D0, 2);
    FLD(SYS + 0x5A8, 8);
    FLD(SYS + 0x5B0, 1);
    FLD(SYS + 0x5B8, 8);
    FLD(SYS + 0x5C0, 1);
    FLD(SYS + 0x5C8, 8);
    FLD(SYS + 0x5D4, 4);
    FLD(SYS + 0x5D8, 4);
    FLD(SYS + 0x5E0, 8);
    FLD(SYS + 0x5E8, 4);
    FLD(SYS + 0x5EC, 4);
    FLD(SYS + 0x5F4, 2);
    for (j = 0; j < 12; j++) {
        FLD(SYS + 0x5F8 + 0x4 * j, 4);
    }
    FLD(SYS + 0x628, 1);
    FLD(SYS + 0x634, 4);
    FLD(SYS + 0x63C, 4);
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0x640 + 0x2 * j, 2);
    }
    for (j = 0; j < 4; j++) {
        FLD(SYS + 0x652 + 0x1 * j, 1);
    }
    FLD(SYS + 0x660, 1);
    FLD(SYS + 0x284, 1);
    FLD(SYS + 0x664, 1);
    for (j = 0; j < 5; j++) {
        FLD(SYS + 0x666 + 0x1 * j, 1);
    }
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0x671 + 0x1 * j, 1);
    }
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0x710 + 0xC * j, 4);
        FLD(SYS + 0x714 + 0xC * j, 4);
    }
    FLD(SYS + 0x724, 4);
    FLD(SYS + 0x730, 4);
    for (j = 0; j < 6; j++) {
        FLD(SYS + 0x738 + 0x4 * j, 4);
    }
    for (j = 0; j < 5; j++) {
        FLD(SYS + 0x750 + 0x2 * j, 2);
    }
    for (j = 0; j < (int)SI(f_5d0); j++) {   /* count re-read: decoding may have just set it */
        FLD(SYS + 0x2C4 + 4 * j, 4);
        FLD(SYS + 0x38C + 4 * j, 4);
        FLD(SYS + 0x454 + 4 * j, 4);
    }
    for (j = 0; j < 5; j++) {
        FLD(SYS + 0x800 + 0x1 * j, 1);
    }
    for (j = 0; j < 4; j++) {
        FLD(SYS + 0x808 + 0x4 * j, 4);
    }
    for (j = 0; j < 24; j++) {
        FLD(SYS + 0x9A8 + 0x8 * j, 8);
    }
    for (j = 0; j < (int)SI(f_8e4); j++)
        FLD(SYS + 0x880 + 2 * j, 2);
}
